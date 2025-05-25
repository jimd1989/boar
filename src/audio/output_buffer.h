#pragma once

#include <stdint.h>

#include "sample.h"

typedef struct OutputBuffer {
  int           pos;
  int           chunkSize;
  int           readChunks;
  int           writeChunks;
  int           writeFrames;
  float       * vol;
  AudioFrame  * whiteNoise;
  uint8_t     * output; /* Eventually do write in place */
} OutputBuffer;

void outputBuffer(OutputBuffer *, AudioFrame *, int, int, float *);
void fillOutputBuffer(OutputBuffer *);
void freeOutputBuffer(OutputBuffer *);
