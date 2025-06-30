#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Checks the membership of n ∈ [0, 127] in a set. Intended as a write-once
 * read-many structure for simple lookups. */
typedef uint32_t BitSet128[4];

void setBit128(BitSet128, uint8_t);
void unsetBit128(BitSet128, uint8_t);
bool isBitSet128(BitSet128, uint8_t);
void bitSet128(BitSet128);
