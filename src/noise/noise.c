#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../audio/sample.h"
#include "noise.h"

static void pinkNoise(PinkNoise *);

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

void fillNoise(Noise *n, int offset, int len) {
  /* Blatant Park-Miller ripoff taken directly from Wikipedia. */
  int i = 0;
  uint32_t x = 0;
  uint64_t product = 0;
  for (; i < len ; i++, n->phase++) {
    /* Use old white buffer for pink */
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
