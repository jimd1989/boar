#pragma once

#include <stdint.h>

#include "../audio/sample.h"

#define NOISE_PINK_BANDS 8
#define NOISE_PINK_BITS 3
#define NOISE_PINK_IDXS 128

typedef struct PinkNoise {
  uint32_t  bands[NOISE_PINK_BANDS];
  uint32_t  sample;
} PinkNoise;

typedef struct Noise {
  uint8_t       phase;
  uint64_t      rand;
  PinkNoise     pinkGenerator;
  uint8_t       bandIdxs[NOISE_PINK_IDXS];
  AudioSample * white;
  AudioSample * pink;
} Noise;

void noise(Noise *, int);
void fillNoiseChunk(Noise *, int);
void freeNoise(Noise *);

#define NOISE_FLOAT(X) ((2.0f * (float)(X) / (float)RAND_MAX) - 1.0f)
