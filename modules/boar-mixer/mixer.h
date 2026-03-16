#pragma once

#include <stdint.h>

void mixer_zero(int, int, float *);
int mix_f32_fade(int, int, float *, int, int, int, float *);
void mix_f32_new(int, float *, int, int, int, float *);
int mix_s16_fade(int, int, float *, int, int, float *, uint8_t *);
void mix_s16_new(int, float *, int, int, float *, uint8_t *);
void mix_s16(int, int, int, float *, float *, float *, uint8_t *);
