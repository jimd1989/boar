#include <math.h>
#include <unistd.h>

#include "keyboard.h"

void keyboard(Keyboard *kb) {
  int i      = 0;
  float freq = KEYBOARD_LOWEST_FREQ;
  while (i < KEYBOARD_SIZE) {
    kb->keys[i].freq  = freq;
    kb->keys[i].inc   = KEYBOARD_FREQ_INC * freq;
    kb->keys[i].voice = NULL;
    freq              = KEYBOARD_LOWEST_FREQ * powf(2.0f, (float)++i / 12);
  }
}
