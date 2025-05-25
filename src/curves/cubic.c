#include "cubic.h"

void fillCubic(float *c, int len) {
  int i     = 0;
  float f   = 0.0f;
  for (; i < len ; i++) {
    f    = (float)i / (float)(len - 1);
    c[i] = f * f * f;
  }
}
