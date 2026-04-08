#include <stdint.h>

#include "envelope.h"

uint32_t update_env_stages(int len, uint32_t currently_running, 
                           EnvStage *stages, float *phases) {
  int i = 0;
  for (i = 0 ; i < len ; i++) {
    if (phases[i] >= 1.0f) {
      currently_running -= stages[i];
      stages[i]          = stages[i] == ENV_SUSTAIN ? ENV_RELEASE   :
                           stages[i] == ENV_RELEASE ? ENV_FREE      :
                           /* otherwise */            stages[i] << 1;
      currently_running += stages[i];
      phases[i]          = 0.0f;
    }
  } 
  return currently_running;
}
