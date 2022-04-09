/* Functions related to audio output, including the main playback function. */

#include <err.h>
#include <limits.h>
#include <math.h>
#include <sndio.h>
#include <stdint.h>
#include <string.h>

#include "audio-output.h"

#include "audio-init.h"
#include "buffers.h"
#include "constants/defaults.h"
#include "constants/errors.h"
#include "numerical.h"
#include "voice.h"

static void clearBuffer(BufferX *);
static void fillBuffer(Audio *);
static int16_t mixdownSample(const float, const float);
static void writeFrames(Audio *);

void
setVolume(Audio *a, const float f) {

/* Sets master amplitude, without getting too loud. */

  a->Amplitude = unipolar(expCurve(truncateFloat(f, 1.0f)));
}

static void
clearBuffer(BufferX *b) {

/* Explicitly zeroes Audio.Buffer.Mix, since sample data is summed up inside
 * of it. This differs from Audio.Buffer.Output, which can simply be overwritten
 * during each cycle. */

  memset(b->Mix, 0, sizeof(*b->Mix) * DEFAULT_BUFSIZE);
}

static void
fillBuffer(Audio *a) {

/* Calculates all sample data from Audio.Voices and sums it up in
 * Audio.Buffer.Mix. The master phase is incremented by DEFAULT_BUFSIZE as a
 * rough phase-tracking heuristic for new notes. */ 

  unsigned int n = 0;

  for (; n < a->Settings.Polyphony ; n++) {
    pollVoice(&a->Voices.All[n]);
  }
  a->Voices.Phase += DEFAULT_BUFSIZE; /* Maybe something else */
}

static int16_t
mixdownSample(const float s, const float amplitude) {

/* Takes a float from Audio.MixingBuffer and returns an int16_t with simple
 * dither noise added to it. This algorithm was adapted from Jonas Norberg's
 * post on KVR Audio. */

  float r = (rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;

  r /= (float)(USHRT_MAX + 1);
  return (int16_t)(roundf(clip(s + r) * (float)SHRT_MAX) * amplitude);
}

static void
writeFrames(Audio *a) {

/* Writes DEFAULT_BUFSIZE worth of frames from Audio.Buffer.Mix to
 * Audio.Buffer.Output. These floats representing an internal monophonic
 * signal are dithered and output as 16 bit signed integers representing
 * the stereo signal. Since DEFAULT_BUFSIZE may not be a perfect multiple of
 * Audio.Buffer.SizeFrames, a call to sio_write may take place within the middle 
 * of this loop. */

  int16_t sl = 0;
  int16_t sr = 0;
  size_t n = 0;
  size_t localFramesWritten = 0;
  size_t limit = 0;
  BufferX *b = &a->Buffer;
  size_t tillWrite = b->SizeFrames - b->FramesWritten;
  while (localFramesWritten < DEFAULT_BUFSIZE) {
    limit = LESSER(tillWrite, DEFAULT_BUFSIZE);
    for (n = 0 ; n < limit ; n++) {
      /* Will eventually have channel independent amplitudes here. */
      sl = mixdownSample(b->Mix[n], a->Amplitude);
      sr = mixdownSample(b->Mix[n], a->Amplitude);
      b->Output[b->BytesWritten++] = (uint8_t)(sl & 255);
      b->Output[b->BytesWritten++] = (uint8_t)(sl >> 8);
      b->Output[b->BytesWritten++] = (uint8_t)(sr & 255);
      b->Output[b->BytesWritten++] = (uint8_t)(sr >> 8);
    }
    localFramesWritten += limit;
    b->FramesWritten += limit;
    if (b->FramesWritten == b->SizeFrames) {
      sio_write(a->Output, b->Output, b->SizeBytes);
      b->FramesWritten = 0;
      b->BytesWritten = 0;
    }
    tillWrite = DEFAULT_BUFSIZE - localFramesWritten;
  }
}

void
play(Audio *a) {

/* Calculates DEFAULT_BUFSIZE frames worth of synthesis data and writes them to
 * Audio.Buffer.Output. This output buffer will eventually be dumped to sndio, 
 * but this may not occur during every invocation of this function. It depends 
 * on the Audio.Buffer.SizeFrames returned by the hardware settings. */

  clearBuffer(&a->Buffer);
  fillBuffer(a);
  writeFrames(a);
}
