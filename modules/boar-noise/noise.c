#include <stdint.h>

#include "noise.h"

#define NOISE_FLOAT_SCALE (1.0f / 2147483647.0f)

static uint64_t NOISE_RAND = 1; /* No need for special seed */

/* Uses Parker-Miller to generate 31 bits of randomness, ie no negatives */
void fill_white_noise(uint32_t *buf, float *fs, int bufLen) {
  int i            = 0;
  uint32_t x       = NOISE_RAND;
  uint64_t product = 0;
  for (i = 0 ; i < bufLen ; i++) {
    product = x * 48271;
    x       = (product & 2147483647) + (product >> 31);
    buf[i]  = x;
    fs[i]   = 2.0f * ((float)x * NOISE_FLOAT_SCALE) - 1.0f;
  }
  NOISE_RAND = x;
}
