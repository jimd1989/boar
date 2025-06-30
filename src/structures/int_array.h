#pragma once

#include "../keyboard/keyboard.h"
#include "../voices/voice.h"

/* Generic (to an extent) arrays of integers. Sized for compile-time exactness.
 * Make a new, similar struct for other sizes. */

typedef struct IntArrayVoices {
  int len;
  int vals[VOICES_SIZE];
} IntArrayVoices;

typedef struct IntArrayNotes {
  int len;
  int vals[KEYBOARD_SIZE];
} IntArrayNotes;

void intArrayNotes(IntArrayNotes *);
void intArrayVoices(IntArrayVoices *);
