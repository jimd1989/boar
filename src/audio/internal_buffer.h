#pragma once

#include "sample.h"

typedef struct InternalBuffer {
  int           pos;
  unsigned int  chunkSize;
  int           chunks;
  int           frames;
  AudioFrame  * audio;
  AudioFrame  * noise;
} InternalBuffer;

void internalBuffer(InternalBuffer *, int);
void fillAudio(InternalBuffer *, int);
void freeInternalBuffer(InternalBuffer *);
