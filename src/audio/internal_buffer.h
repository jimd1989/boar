#pragma once

#include "../noise/noise.h"
#include "sample.h"

typedef struct InternalBuffer {
  int           pos;
  unsigned int  chunkSize;
  int           chunks;
  int           frames;
  Noise         noise;
  AudioFrame  * audio;
} InternalBuffer;

void internalBuffer(InternalBuffer *, int);
void fillAudio(InternalBuffer *, int);
void freeInternalBuffer(InternalBuffer *);
