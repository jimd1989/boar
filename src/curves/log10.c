#include <math.h>

#include "curve_settings.h"
#include "log10.h"

void fillLog10(float *c) {
  int i     = 0;
  for (; i < CURVE_LEN ; i++) {
    c[i] = log10f((float)(i + 1)) / log10f((float)CURVE_LEN);
  }
}
