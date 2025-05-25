#pragma once

#define CURVES_LEN 4096

typedef struct Curves {
  float log10[CURVES_LEN];
  float cubic[CURVES_LEN];
} Curves;

void curves(Curves *);
