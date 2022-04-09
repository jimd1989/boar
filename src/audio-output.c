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
#include "constants/errors.h"
#include "numerical.h"
#include "voice.h"

static void clearBuffer(FloatBuffer *);
static void fillBuffer(Audio *);
static size_t writeSample(ByteBuffer *, const int16_t, const unsigned int);
static int16_t mixdownSample(const float, const float);
static void writeBuffer(Audio *);
static void writeFrames(Audio *);

void
setVolume(Audio *a, const float f) {

/* Sets master amplitude, without getting too loud. */

  a->Amplitude = unipolar(expCurve(truncateFloat(f, 1.0f)));
}

static void
clearBuffer(FloatBuffer *b) {

/* Explicitly zeroes Audio.MixingBuffer, since sample data is summed up inside
 * of it. This differs from Audio.MainBuffer, which can simply be overwritten
 * during each cycle. */

  memset(b->Values, 0, sizeof(*b->Values) * b->SizeFrames);
}

static void
fillBuffer(Audio *a) {

/* Calculates all sample data from Audio.Voices and sums it up in
 * Audio.MixingBuffer. The master phase is incremented by Audio.Bufsize as a
 * rough phase-tracking heuristic for new notes. */ 

  unsigned int i = 0;

  for (; i < a->Settings.Polyphony ; i++) {
    pollVoice(&a->Voices.All[i]);
  }
  a->Voices.Phase += a->Settings.BufSizeFrames;
}

static size_t
writeSample(ByteBuffer *bb, const int16_t s, const unsigned int i) {

/* int16_t s is a sample from Audio.Mixing buffer. All dithering, clipping,
 * etc of the sample is assumed to have taken place already. This function
 * writes the sample to every channel of the final output buffer. Returns the
 * number of bytes written. */

  unsigned int j;

  for (j = 0 ; j < bb->ChanBytes; j+=2) {
    bb->Values[i+j]   = (uint8_t)(s & 255);
    bb->Values[i+j+1] = (uint8_t)(s >> 8); 
  }
  return bb->ChanBytes;
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

/* Writes the contents of Audio.MixingBuffer to Audio.MainBuffer. Errors are
 * fatal. */

  unsigned int i, j;
  int16_t s;

  for (i = 0, j = 0 ; i < a->MixingBuffer->SizeFrames ; i++) {
    s = mixdownSample(a->MixingBuffer->Values[i], a->Amplitude);
    j += writeSample(a->MainBuffer, s, j);
  }
  /* WORKS: do actual buffer size now */
  sio_write(a->Output, a->MainBuffer->Values, a->Settings.WindowSizeFrames * 4);
}

static void
writeFrames(Audio *a) {
  int16_t sl = 0;
  int16_t sr = 0;
  size_t n = 0;
  size_t localFramesWritten = 0;
  size_t limit = 0;
  FloatBuffer *fb = a->MixingBuffer;
  ByteBuffer *bb = a->MainBuffer;
  size_t remaining = bb->SizeFrames - bb->FramesWritten;
  while (localFramesWritten < fb->SizeFrames) {
    limit = LESSER(remaining, fb->SizeFrames);
    for (n = 0 ; n < limit ; n++) {
      /* Amplitudes will eventually be channel independent. */
      sl = mixdownSample(fb->Values[n], a->Amplitude);
      sr = mixdownSample(fb->Values[n], a->Amplitude);
      bb->Values[bb->BytesWritten++] = (uint8_t)(sl & 255);
      bb->Values[bb->BytesWritten++] = (uint8_t)(sl >> 8);
      bb->Values[bb->BytesWritten++] = (uint8_t)(sr & 255);
      bb->Values[bb->BytesWritten++] = (uint8_t)(sr >> 8);
    }
    localFramesWritten += limit;
    bb->FramesWritten += limit;
    if (bb->FramesWritten == bb->SizeFrames) {
      sio_write(a->Output, bb->Values, bb->SizeBytes);
      bb->BytesWritten = 0;
      bb->FramesWritten = 0;
    }
    limit = fb->SizeFrames - localFramesWritten;
  }
}

void
play(Audio *a) {

/* Calculate and output a buffer's worth of synthesis information. */

  clearBuffer(a->MixingBuffer);
  fillBuffer(a);
  writeFrames(a);
}
