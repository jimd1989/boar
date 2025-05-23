#pragma once

#include <stdint.h>

typedef union AudioSample {
  float     f;
  uint32_t  n;
} AudioSample;

typedef struct AudioFrame {
  AudioSample l;
  AudioSample r;
} AudioFrame;
