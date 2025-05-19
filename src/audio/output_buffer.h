#pragma once

#include <stdint.h>

#include "sample.h"

typedef struct OutputBuffer {
  int           chunkSize;
  int           currentChunk;
  int           readChunks;
  int           writeChunks;
  uint64_t      framesWritten;
  AudioFrame  * noise;
  uint8_t     * output;
} OutputBuffer;

void outputBuffer(OutputBuffer *, AudioFrame *, int, int);
void fillOutputBuffer(OutputBuffer *);
void freeOutputBuffer(OutputBuffer *);
