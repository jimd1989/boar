#include <err.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "../chunk/chunk.h"
#include "output_buffer.h"
#include "sample.h"

static int16_t dither(float, int32_t);

void outputBuffer(OutputBuffer *o, AudioFrame *audio, AudioFrame *whiteNoise,
                  int rFrames, int wFrames) {
  int chunkSize = AUDIO_CHUNK_SIZE;
  while ((rFrames % chunkSize) != 0 || (wFrames % chunkSize) != 0) {
    chunkSize--;
    if (chunkSize < 1) { errx(1, "Error setting output chunk size."); }
  }
  o->chunkSize   = chunkSize;
  o->readChunks  = rFrames / chunkSize;
  o->writeChunks = wFrames / chunkSize;
  o->writeFrames = wFrames;
  o->pos         = 0;
  o->audio       = audio;
  o->whiteNoise  = whiteNoise;
  o->output      = calloc(wFrames * 4, 1);
  warnx("out →\tframes: %d\tchunk size: %d", wFrames, o->chunkSize);
}

static int16_t dither(float s, int32_t noise) {
  int32_t n = (int32_t)(s * (1 << 15) * (1 << 16)); /* Scale up to Q15.16 int */
  n        += noise & USHRT_MAX; /* Add int noise to Q15.16 frac space */
  return (int16_t)(n >> 16); /* Shift down */
}

void fillOutputBuffer(OutputBuffer *o) {
  int i         = 0;
  int j         = 0;
  int16_t s     = 0;
  AudioFrame *a = NULL;
  AudioFrame *n = NULL;
  uint8_t *out  = o->output;
  for (; i < o->writeChunks ; i++) {
    a = &o->audio[o->pos * o->chunkSize];
    n = &o->whiteNoise[o->pos * o->chunkSize];
    for (j = 0 ; j < o->chunkSize ; j++) {
      /* Use opposite channel noise for dither for less self-reference. */
      s      = dither(a[j].l.f, n[j].r.n);
      *out++ = s & 255;
      *out++ = s >> 8;
      s      = dither(a[j].r.f, n[j].l.n);
      *out++ = s & 255;
      *out++ = s >> 8;
    }
    o->pos = (o->pos + 1) % o->readChunks;
  }
}

void freeOutputBuffer(OutputBuffer *o) {
  free(o->output);
}
