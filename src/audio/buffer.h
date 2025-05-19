#pragma once

#include <stdint.h>

#include "sample.h"
#include "settings.h"

/* Ring buffer that juggles dynamically-sized soundcard output against 0.05 
 * seconds of internal DSP data. Manages when to write audio and when to perform
 * DSP based upon the discrepancy between `framesGenerated` and `framesWritten`.
 */
typedef struct AudioBuffer {
  int           currentChunk;
  int           chunkSize;
  int           sizeInChunks;
  int           sizeInFrames;
  int           soundcardBytesToWrite;
  int           soundcardFramesToWrite;
  int           soundcardChunksToWrite;
  int           soundcardPosFrames;
  uint64_t      framesGenerated;
  uint64_t      framesWritten;
  AudioFrame  * frames;
  AudioFrame  * noise;
  uint8_t     * output;
} AudioBuffer;

void generateDsp(void *, int);
void fillBuffer(AudioBuffer *);
void audioBuffer(AudioBuffer *, int);
void freeAudioBuffer(AudioBuffer *);
