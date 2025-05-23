#include <err.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "chunk.h"
#include "output_buffer.h"
#include "sample.h"

void outputBuffer(OutputBuffer *o, AudioFrame *whiteNoise,
                  int rFrames, int wFrames) {
  int chunkSize = AUDIO_CHUNK_SIZE;
  while ((rFrames % chunkSize) != 0 || (wFrames % chunkSize) != 0) {
    chunkSize--;
    if (chunkSize < 1) { errx(1, "Error setting output chunk size."); }
  }
  warnx("Using %d frame output chunks", chunkSize);
  o->chunkSize   = chunkSize;
  o->readChunks  = rFrames / chunkSize;
  o->writeChunks = wFrames / chunkSize;
  o->writeFrames = wFrames;
  o->pos         = 0;
  o->whiteNoise  = whiteNoise;
  o->output      = calloc(wFrames * 4, 1);
  warnx("OUTPUT %d frames %d chunks", wFrames, o->writeChunks);
}

void fillOutputBuffer(OutputBuffer *o) {
  int i         = 0;
  int j         = 0;
  int16_t s     = 0;
  AudioFrame *a = NULL;
  uint8_t *out  = o->output;
  for (; i < o->writeChunks ; i++) {
    a = &o->whiteNoise[o->pos * o->chunkSize];
    for (j = 0 ; j < o->chunkSize ; j++) {
      s      = 0.005f * a[j].l * SHRT_MAX; /* Need to dither */
      *out++ = s & 255;
      *out++ = s >> 8;
      s      = 0.005f * a[j].r * SHRT_MAX; /* Need to dither */
      *out++ = s & 255;
      *out++ = s >> 8;
    }
    o->pos = (o->pos + 1) % o->readChunks;
  }
}

void freeOutputBuffer(OutputBuffer *o) {
  free(o->output);
}
