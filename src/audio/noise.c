#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sample.h"
#include "noise.h"

static void pinkNoise(PinkNoise *);

static void pinkNoise(PinkNoise *p) {
  p->phase  = 0;
  p->sample = 0;
  memset(p->bands, 0, NOISE_PINK_BANDS * sizeof(uint32_t));
}

void initNoise(Noise *n, int size) {
  n->rand  = 1; /* No need for unique seed */
  pinkNoise(&n->lPink);
  pinkNoise(&n->rPink);
  n->white = calloc(size, sizeof(AudioFrame));
  n->pink  = calloc(size, sizeof(AudioFrame));
}

void freeNoise(Noise *n) {
  free(n->white);
  free(n->pink);
}



static uint32_t RAND = 1; /* No need for unique seed */
//static uint32_t PINK_NOISE_PHASE = 0;

/* Make basic rand() function used by higher order noise fills */

void noise(AudioFrame *buf, int n) {
  /* Blatant Park-Miller ripoff taken directly from Wikipedia. */
  int i = 0;
  uint32_t x = 0;
  uint64_t product = 0;
  for (; i < n ; i++) {
    product  = (uint64_t)RAND * 48271;
    x        = (product & 0x7fffffff) + (product >> 31);
    RAND     = x;
    buf[i].l = (2.0f * (float)x / (float)RAND_MAX) - 1.0f;
    product  = (uint64_t)RAND * 48271;
    x        = (product & 0x7fffffff) + (product >> 31);
    RAND     = x;
    buf[i].r = (2.0f * (float)x / (float)RAND_MAX) - 1.0f;
  }
}
