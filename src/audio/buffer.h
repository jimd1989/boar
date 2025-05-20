#pragma once

#include "internal_buffer.h"
#include "output_buffer.h"

/* Overlapping ring buffers of internal DSP and soundcard output.
 * - Both buffers are split into perfect chunks for faster processing.
 * - InternalBuffer size is an multiple of OutputBuffer size + 1 chunk. */
typedef struct AudioBuffer {
  int             mult;   /* InternalBuffer is at least mult times Output */
  int             fractionalPhase; /* Track i/o mismatch */
  InternalBuffer  i;
  OutputBuffer    o;
} AudioBuffer;

void audioBuffer(AudioBuffer *, int);
void generateDsp(void *, int);
void freeAudioBuffer(AudioBuffer *);
