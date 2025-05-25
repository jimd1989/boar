#pragma once

#include "../control/control.h"
#include "buffer.h"
#include "sndio.h"

typedef struct Audio {
  AudioBuffer buffer;
  Sio         sio;
} Audio;

void audio(Audio *, Control *);
void writeAudio(Audio *);
void stopAudio(Audio *);
