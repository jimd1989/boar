#include "sample.h"
#include "noise.h"

#include <limits.h>
#include <stdint.h>

#include "noise.h"

#define RECIPROCAL (1.0f / (float)UINT_MAX)

static uint32_t RAND = 1; /* No need for unique seed */

void noise(AudioFrame *buf, int n) {
  /* Blatant Park-Miller ripoff taken directly from Wikipedia. */
  int i = 0;
  uint32_t x = 0;
  uint64_t product = 0;
  for (; i < n ; i++) {
    product  = (uint64_t)RAND * 48271;
    x        = (product & 0x7fffffff) + (product >> 31);
    RAND     = x;
    buf[i].l = (float)x * RECIPROCAL;
    product  = (uint64_t)RAND * 48271;
    x        = (product & 0x7fffffff) + (product >> 31);
    RAND     = x;
    buf[i].r = (float)x * RECIPROCAL;
  }
}
