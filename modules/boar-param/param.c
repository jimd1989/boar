#include <stdint.h>
#include <string.h>

#include "param.h"

#define PARAM_PHASE_INC (1.0f / (float)PARAM_SIZE)

/* Parameter fades should be written interleaved within a channel since they 
 * will be accessed far more often than they are modified. */
void params_set_linear(int count, int ch, int n, 
                       float old, float new, float *buf) {
  float delta = new - old;
  int idx     = n + (PARAM_SIZE * count * ch);
  int i       = 0;
  float phase = 0.0f;
  for (i = 0 ; i < PARAM_SIZE ; i++, idx += count, phase += PARAM_PHASE_INC) {
    buf[idx] = old + (phase * delta);
  }
}

uint32_t params_after_fade_cleanup_new(int len, float *phases, 
                                       float *increments, uint32_t *fadeLens) {
  int i                     = 0;
  uint32_t finishedFadeLens = 0;
  for (i = 0 ; i < len ; i++) {
    if (phases[i] >= 1.0f) {
      phases[i]         = 1.0f;
      increments[i]     = 0.0f;
      finishedFadeLens += fadeLens[i];
      fadeLens[i]       = 0;
    }
  }
  return finishedFadeLens;
}

/* Uses a new (to me) memory-doubling trick for faster resets. Be careful. */
void params_after_fade_cleanup(int ch, int count, float *new, float *buf) {
  int remaining = 0;
  int chIdx     = 0;
  int fillSize  = count;
  int chBufSize = PARAM_SIZE * count;
  float *chBuf  = buf;
  for (chIdx = 0 ; chIdx < ch ; chIdx++) {
    fillSize = count;
    chBuf    = &buf[chIdx * chBufSize];
    memcpy(chBuf, &new[chIdx * count], fillSize * sizeof(float));
    while (fillSize * 2 <= chBufSize) {
      memcpy(&chBuf[fillSize], chBuf, fillSize * sizeof(float));
      fillSize *= 2;
    }
    remaining = chBufSize - fillSize;
    memcpy(&chBuf[fillSize], chBuf, remaining * sizeof(float));
  }
}
