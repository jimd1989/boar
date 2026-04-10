#include <stdint.h>

#pragma once

static int PARAM_SIZE = 64;

void params_set_linear(int, int, int, float, float, float *);
void params_after_fade_cleanup(int, int, float *, float *);
uint32_t params_after_fade_cleanup_new(int, float *, float *, uint32_t *);
