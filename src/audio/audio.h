#pragma once

#include "buffer.h"

typedef struct Audio {
  AudioBuffer buffer;
  /* Sndio device here */
} Audio;

void audio(Audio *);
