#pragma once

#include <stdint.h>

#include "../audio/sample.h"

#define NOISE_PINK_BANDS 8

typedef struct PinkNoise {
  uint8_t   phase; /* Can be factored out */
  uint32_t  bands[NOISE_PINK_BANDS];
  uint64_t  sample;
} PinkNoise;

typedef struct Noise {
  uint64_t      rand;
  PinkNoise     lPink;
  PinkNoise     rPink;
  AudioFrame  * white;
  AudioFrame  * pink;
} Noise;

void noise(Noise *, int);
void fillNoise(Noise *, int, int);
void freeNoise(Noise *);
