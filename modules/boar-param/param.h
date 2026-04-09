#pragma once

static int PARAM_SIZE = 64;
static int PARAM_BLOCK_SIZE = 32;

void params_set_linear(int, int, int, float, float, float *);
void params_after_fade_cleanup(int, int, float *, float *);
