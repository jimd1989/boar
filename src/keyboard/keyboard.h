#pragma once

#include "../audio/settings.h"
#include "../curves/curve_settings.h"
#include "../voices/voice.h"

#define KEYBOARD_SIZE 128
#define KEYBOARD_LOWEST_FREQ 8.1757989156f
#define KEYBOARD_FREQ_INC ((float)CURVE_LEN / (float)AUDIO_SAMPLE_RATE)

typedef struct Key {
  int     zone;
  float   freq;
  float   inc;
  Voice * voice;
} Key;

typedef struct Keyboard {
  Key   keys[KEYBOARD_SIZE];
} Keyboard;

void keyboard(Keyboard *);
void printKeyboard(Keyboard *);
