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
static void writeFrame(BufferX *, const int16_t);
static int16_t mixdownSample(const float, const float);
static void writeBuffer(Audio *);

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

  unsigned int i = 0;

  for (; i < a->Settings.Polyphony ; i++) {
    pollVoice(&a->Voices.All[i]);
  }
  a->Voices.Phase += DEFAULT_BUFSIZE; /* Maybe something else */
  a->Buffer.OutputFramesToWrite = DEFAULT_BUFSIZE; /* Maybe not needed */
}

static void
writeFrames(Audio *a) {
  BufferX *b = &a->Buffer;
  size_t remaining = b->OutputSizeFrames - b->OutputFramesWritten;
  size_t limit = LESSER(remaining, DEFAULT_BUFSIZE);
}

static void
writeFrame(BufferX *b, const int16_t s) {

/* int16_t s is a sample from Audio.Buffer.Mix. All dithering, clipping,
 * etc of the sample is assumed to have taken place already. This function
 * writes the same sample to each stereo channel of Audio.Buffer.Output. */

  b->Output[b->OutputBytesWritten++] = (uint8_t)(s & 255);
  b->Output[b->OutputBytesWritten++] = (uint8_t)(s >> 8);
  b->Output[b->OutputBytesWritten++] = (uint8_t)(s & 255);
  b->Output[b->OutputBytesWritten++] = (uint8_t)(s >> 8);
  b->OutputFramesWritten++;
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
writeBuffer(Audio *a) {

/* Writes the contents of Audio.Buffer.Mix to Audio.Buffer.Output, then onwards
 * to sndio. */

  int16_t s = 0;
  BufferX *b = &a->Buffer;

  while(b->OutputFramesWritten < DEFAULT_BUFSIZE) {
    s = mixdownSample(b->Mix[b->OutputFramesWritten], a->Amplitude);
    writeFrame(b, s);
  }
  /* START BY ONLY WRITING DEFAULT_BUFSIZE AT A TIME, GET SMART LATER */
  (void)sio_write(a->Output, b->Output, DEFAULT_BUFSIZE);
  b->OutputFramesWritten = 0;
  b->OutputBytesWritten = 0;

}

void
play(Audio *a) {

/* Calculate and output a buffer's worth of synthesis information. */

  clearBuffer(&a->Buffer);
  fillBuffer(a);
  writeBuffer(a);
}
