#include "cubic.h"
#include "curve_settings.h"

void fillCubic(float *c) {
  int i     = 0;
  float f   = 0.0f;
  for (; i < CURVE_LEN ; i++) {
    f    = (float)i / (float)CURVE_DIV;
    c[i] = f * f * f;
  }
}
