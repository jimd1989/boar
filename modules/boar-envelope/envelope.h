#pragma once

#include <stdint.h>

typedef enum EnvStage {
  ENV_FREE    = 0,
  ENV_SUSTAIN = 1,
  ENV_ATTACK  = 65536,
  ENV_DECAY   = 131072,
  ENV_RELEASE = 262144
} EnvStage;

uint32_t update_env_stages(int, uint32_t, EnvStage *, float *);
