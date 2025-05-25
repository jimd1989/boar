#include "../curves/curves.h"
#include "control.h"

void control(Control *c) {
  setVol(c, 0.0f);
  curves(&c->curves);
}

void setVol(Control *c, float v) {
  int i  = v * (float)(CURVES_LEN - 1);
  c->vol = c->curves.cubic[i];
}
