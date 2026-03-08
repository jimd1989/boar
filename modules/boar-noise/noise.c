#include <stdint.h>

#include "noise.h"

#define NOISE_FLOAT_SCALE (1.0f / 4294967296.0f)

static uint64_t NOISE_RAND = 1; /* No need for special seed */

/* Uses Xorshift for 32 bits */
void fill_white_noise(uint32_t *buf, float *fs, int bufLen) {
  int i      = 0;
  uint32_t x = NOISE_RAND;
  for (i = 0 ; i < bufLen ; i++) {
    x          = NOISE_RAND;
    x         ^= x << 13;
    x         ^= x >> 17;
    x         ^= x << 5;
    NOISE_RAND = x;
    /* additional scramble → not in state */
    x         *= 2654435771;
    buf[i]     = x;
    fs[i]      = 2.0f * ((float)x * NOISE_FLOAT_SCALE) - 1.0f;
  }
}
