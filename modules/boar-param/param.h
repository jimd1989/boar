#pragma once

static int PARAM_SIZE = 128;

void params_set_linear(int, int, int, float, float, float *);
void params_after_fade_cleanup(int, int, float *, float *);
