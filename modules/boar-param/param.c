#include "param.h"

#define PARAM_PHASE_INC (1.0f / (float)PARAM_SIZE)

void param_set_linear(float old, float new, float *buf) {
  int i       = 0;
  float phase = 0.0f;
  for (i = 0 ; i < PARAM_SIZE ; i++, phase += PARAM_PHASE_INC) {
    buf[i] = (phase * new) + ((1.0f - phase) * old);
  }
}
