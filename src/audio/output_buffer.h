#pragma once

#include <stdint.h>

#include "sample.h"

typedef struct OutputBuffer {
  int           pos;
  int           chunkSize;
  int           readChunks;
  int           writeChunks;
  int           writeFrames;
  AudioSample * audio;
  AudioSample * whiteNoise;
  uint8_t     * output; 
} OutputBuffer;

void outputBuffer(OutputBuffer *, AudioSample *, AudioSample *, int, int);
void fillOutputBuffer(OutputBuffer *);
void freeOutputBuffer(OutputBuffer *);
