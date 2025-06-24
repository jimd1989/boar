#pragma once

#include <stdint.h>

#include "../curves/curves.h"
#include "../keyboard/keyboard.h"
#include "../voices/voices.h"
#include "volume.h"

#define CONTROL_127_DIV 0.0078740157480315f

typedef struct Control {
  Curves    curves;
  Keyboard  keyboard;
  Voices    voices;
  Volume    vol;
} Control;

void noteOn(Control *, uint8_t, uint8_t);
void noteOff(Control *, uint8_t, uint8_t);
void control(Control *);
