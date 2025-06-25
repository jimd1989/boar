#include "curves.h"
#include "cubic.h"
#include "log10.h"
#include "sine.h"

void curves(Curves *c) {
  fillCubic(c->cubic);
  fillLog10(c->log10);
  fillSine(c->sine);
}
