#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../audio/sample.h"
#include "../chunk/chunk.h"
#include "noise.h"

static void pinkNoise(PinkNoise *);
static uint32_t pinkSample(PinkNoise *, int, uint32_t);
static void bandIdxs(uint8_t[NOISE_PINK_IDXS]);
static int bandIdx(uint8_t, uint8_t[NOISE_PINK_IDXS]);

static void pinkNoise(PinkNoise *p) {
  p->sample = 0;
  memset(p->bands, 0, NOISE_PINK_BANDS * sizeof(uint32_t));
}

static void bandIdxs(uint8_t idxs[NOISE_PINK_IDXS]) {
  /* Store pink noise bands to update for a 8-bit phase, as 4-bit values */
  uint8_t count1 = 0;
  uint8_t count2 = 0;
  int i = 0;
  uint8_t n = 0;
  for (; i < NOISE_PINK_IDXS * 2; i += 2) {
    count1 = 0;
    n = i;
    while ((n & 1) == 0 && n != 0) { count1++; n >>= 1; }
    count2 = 0;
    n = i + 1;
    while ((n & 1) == 0 && n != 0) { count2++; n >>= 1; }
    idxs[i / 2] = (count2 << 4) | count1;
  }
}

static int bandIdx(uint8_t phase, uint8_t idxs[NOISE_PINK_IDXS]) {
  int i     = phase >> 1;
  bool odd  = phase & 1;
  uint8_t n = idxs[i];
  return (odd * (n & 240)) + ((!odd) * (n & 15));
}

void noise(Noise *n, int size) {
  n->phase = 0;
  n->rand  = 1; /* No need for unique seed */
  pinkNoise(&n->lPink);
  pinkNoise(&n->rPink);
  n->white = calloc(size, sizeof(AudioFrame));
  n->pink  = calloc(size, sizeof(AudioFrame));
  bandIdxs(n->bandIdxs);
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
    band                     = bandIdx(n->phase, n->bandIdxs);
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
