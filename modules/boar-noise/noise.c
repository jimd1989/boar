#include <stdint.h>

#include "noise.h"

static uint64_t NOISE_RAND = 1; /* No need for special seed */

void fill_white_noise(uint8_t *bytes, int bytesToWrite) {
  uint32_t *buf    = (uint32_t *)bytes;
  int bufLen       = bytesToWrite / 4;
  int i            = 0;
  uint32_t x       = NOISE_RAND;
  uint64_t product = 0;
  for (i = 0 ; i < bufLen ; i++) {
    product = x * 48271;
    x       = (product & 2147483647) + (product >> 31);
    buf[i]  = x;
  }
  NOISE_RAND = x;
}
