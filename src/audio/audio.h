#pragma once

#include "buffer.h"
#include "sndio.h"

typedef struct Audio {
  AudioBuffer buffer;
  Sio         sio;
} Audio;

void audio(Audio *);
void writeAudio(Audio *);
void stopAudio(Audio *);
