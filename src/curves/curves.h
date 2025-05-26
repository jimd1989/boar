#pragma once

#include "curve_settings.h"

typedef struct Curves {
  float log10[CURVE_LEN];
  float cubic[CURVE_LEN];
} Curves;

void curves(Curves *);
