#include "curves.h"
#include "cubic.h"
#include "log10.h"

void curves(Curves *c) {
  fillCubic(c->cubic, CURVES_LEN);
  fillLog10(c->log10, CURVES_LEN);
}
