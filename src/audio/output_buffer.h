#pragma once

#include <stdint.h>

#include "sample.h"

typedef struct OutputBuffer {
  int           pos;
  int           chunkSize;
  int           readChunks;
  int           writeChunks;
  AudioFrame  * noise;
  uint8_t     * output;
} OutputBuffer;

void outputBuffer(OutputBuffer *, AudioFrame *, int, int);
void fillOutputBuffer(OutputBuffer *);
void freeOutputBuffer(OutputBuffer *);
