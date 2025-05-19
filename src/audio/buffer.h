#pragma once

#include <stdint.h>

#include "internal_buffer.h"
#include "output_buffer.h"

/* Overlapping ring buffers of internal DSP and soundcard output.
 * - InternalBuffer size is an multiple of OutputBuffer size.
 * - Both buffers are split into perfect chunks for faster processing.
 * - InternalBuffer's chunk size is an multiple of Outputbuffer's chunk size.
 * - The program can safely convert between chunk sizes. */
typedef struct AudioBuffer {
  int             soundcardChunks;
  uint64_t        gens;   /* DSP frames generated */
  uint64_t        writes; /* Audio frames written to soundcard */
  InternalBuffer  i;
  OutputBuffer    o;
} AudioBuffer;

void audioBuffer(AudioBuffer *, int);
void generateDsp(void *, int);
void freeAudioBuffer(AudioBuffer *);
