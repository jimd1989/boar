#pragma once

#include "../control/control.h"
#include "../noise/noise.h"
#include "sample.h"

typedef struct InternalBuffer {
  int           pos;
  unsigned int  chunkSize;
  int           chunks;
  int           frames;
  Noise         noise;
  Control     * control;
  AudioFrame  * audio;
} InternalBuffer;

void internalBuffer(InternalBuffer *, int, Control *);
void fillAudio(InternalBuffer *, int);
void freeInternalBuffer(InternalBuffer *);
