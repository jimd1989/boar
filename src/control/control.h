#pragma once

#include "../curves/curves.h"

typedef struct Control {
  float   vol;
  Curves  curves;
} Control;

void control(Control *);
void setVol(Control *, float);
