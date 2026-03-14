#include <string.h>

#include "param.h"

#define PARAM_PHASE_INC (1.0f / (float)PARAM_SIZE)

/* Parameter fades should be written interleaved since they will be accessed
 * far more often than they are modified */
void params_set_linear(int population, int offset, float old, float new, 
                       float *buf) {
  float delta = new - old;
  int i       = offset;
  float phase = 0.0f;
  for (i = offset ; i < PARAM_SIZE * population ; i     += population, 
                                                  phase += PARAM_PHASE_INC) {
    buf[i] = old + (phase * delta);
  }
}

/* Uses a new (to me) memory-doubling trick for faster resets. Be careful */
void params_after_fade_cleanup(int population, float *new, float *buf) {
  int bufSize   = PARAM_SIZE * population;
  int fillPos   = population;
  int remaining = 0;
  memcpy(buf, new, population * sizeof(float));
  while (fillPos * 2 <= bufSize) {
    memcpy(&buf[fillPos], buf, fillPos * sizeof(float));
    fillPos *= 2;
  }
  remaining = bufSize - fillPos;
  if (remaining > 0) {
    memcpy(&buf[fillPos], buf, remaining * sizeof(float));
  }
}
