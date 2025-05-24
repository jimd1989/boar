#include <math.h>

#include "log10.h"

void fillLog10(float *c, int len) {
  int i     = 0;
  for (; i < len ; i++) {
    c[i] = log10f((float)(i + 1)) / log10f((float)len);
  }
}
