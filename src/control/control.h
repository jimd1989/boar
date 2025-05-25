#pragma once

#include "../curves/curves.h"
#include "volume.h"

typedef struct Control {
  Volume  vol;
  Curves  curves;
} Control;

void control(Control *);
