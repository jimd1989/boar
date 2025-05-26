#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../audio/sample.h"
#include "../chunk/chunk.h"
#include "noise.h"

static void pinkNoise(PinkNoise *);
static uint32_t pinkSample(PinkNoise *, int, uint32_t);

static void pinkNoise(PinkNoise *p) {
  p->sample = 0;
  memset(p->bands, 0, NOISE_PINK_BANDS * sizeof(uint32_t));
}

void noise(Noise *n, int size) {
  n->phase = 0;
  n->rand  = 1; /* No need for unique seed */
  pinkNoise(&n->lPink);
  pinkNoise(&n->rPink);
  n->white = calloc(size, sizeof(AudioFrame));
  n->pink  = calloc(size, sizeof(AudioFrame));
}

static uint32_t pinkSample(PinkNoise *p, int i, uint32_t s) {
  p->sample  -= p->bands[i];
  p->bands[i] = s >> NOISE_PINK_BITS; /* Too quiet? */
  p->sample  += p->bands[i];
  return p->sample;
}

void fillNoiseChunk(Noise *n, int offset) {
  int i = 0;
  int band = 0;
  uint32_t x = 0;
  uint64_t product = 0;
  for (; i < AUDIO_CHUNK_SIZE ; i++, n->phase++) {
    /* Pink noise uses recycled white noise in Voss-McCartney. */
    band                     = __builtin_ctz(n->phase);
    x                        = n->white[offset + i].l.n;
    n->pink[offset + i].l.n  = pinkSample(&n->lPink, band, x);
    x                        = n->white[offset + i].r.n;
    n->pink[offset + i].r.n  = pinkSample(&n->lPink, band, x);
    /* White noise is Park-Miller directly ripped from Wikipedia. */
    product                  = n->rand * 48271;
    x                        = (product & 0x7fffffff) + (product >> 31);
    n->rand                  = x;
    n->white[offset + i].l.n = x;
    product                  = n->rand * 48271;
    x                        = (product & 0x7fffffff) + (product >> 31);
    n->rand                  = x;
    n->white[offset + i].r.n = x;
  }
}

void freeNoise(Noise *n) {
  free(n->white);
  free(n->pink);
}
