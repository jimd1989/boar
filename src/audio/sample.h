#pragma once

#include <stdint.h>

typedef union AudioSample {
  float     f;
  uint32_t  n;
} AudioSample;
