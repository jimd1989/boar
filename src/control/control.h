#pragma once

#include <stdint.h>

#include "../curves/curves.h"
#include "../keyboard/keyboard.h"
#include "../parse/args.h"
#include "../voices/voice_zones.h"
#include "../voices/voices.h"
#include "volume.h"

#define CONTROL_127_DIV 0.0078740157480315f

/* Governs "low-res" parameter changes: those outside of the core DSP loop. */
typedef struct Control {
  Args        args;
  Curves      curves;
  Keyboard    keyboard;
  VoiceZones  voiceZones;
  Volume      vol;
} Control;

void noteOn(Control *, uint8_t, uint8_t);
void noteOff(Control *, uint8_t, uint8_t);
void setMixerVol(Control *, int, float);
void control(Control *, Args);
