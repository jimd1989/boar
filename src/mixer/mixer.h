#pragma once

#define MIXER_SIZE 16

typedef struct Mixer {
  int   chan;
  float volumes[MIXER_SIZE];
} Mixer;

void mixer(Mixer *, int);
