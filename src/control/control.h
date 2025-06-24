#pragma once

#include "../curves/curves.h"
#include "../keyboard/keyboard.h"
#include "volume.h"

typedef struct Control {
  Curves    curves;
  Keyboard  keyboard;
  Volume    vol;
} Control;

void control(Control *);
