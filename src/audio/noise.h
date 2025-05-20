#pragma once

#include <stdint.h>

#include "sample.h"

#define NOISE_PINK_BANDS 8

typedef struct PinkNoise {
  uint8_t   phase;
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

void initNoise(Noise *, int);
void freeNoise(Noise *);
void noise(AudioFrame *, int);
