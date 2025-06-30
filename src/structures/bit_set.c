#include <stdbool.h>
#include <stdint.h>

#include "bit_set.h"

void bitSet128(BitSet128 bs) {
  bs[0] = 0;
  bs[1] = 0;
  bs[2] = 0;
  bs[3] = 0;
}

void setBit128(BitSet128 bs, uint8_t n) {
  uint8_t i = n >> 5;
  bs[i]    |= 1 << (n & 31);
}

void unsetBit128(BitSet128 bs, uint8_t n) {
  uint8_t i = n >> 5;
  bs[i]    &= ~(1 << (n & 31));
}

bool isBitSet128(BitSet128 bs, uint8_t n) {
  uint8_t i = n >> 5;
  return (bool)(bs[i] & (1 << (n & 31)));
}
