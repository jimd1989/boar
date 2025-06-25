#include <math.h>

#include "curve_settings.h"
#include "sine.h"

void fillSine(float *c) {
  float f   = 0.0f;
  float inc = (M_PI * 2.0f) / (float)CURVE_LEN;
  int i     = 0;
  for (; i < CURVE_LEN ; i++, f += inc) { c[i] = sinf(f); }
}
