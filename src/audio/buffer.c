#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"
#include "sample.h"
#include "settings.h"

/* Will eventually have synth object injected */
void generateDsp(AudioBuffer *b) {
  int i = 0;
  if ((b->framesGenerated - b->framesWritten) < b->bufferFillThreshold) {
    for( ; i < b->chunksToFill ; i++) {
      /* dsp(b->frames[b->currentChunk * b->chunkSize]) */
      b->currentChunk = (b->currentChunk + 1) % b->sizeInChunks;
      b->framesGenerated += b->chunkSize;
    }
  }
}

void fillBuffer(AudioBuffer *b) {
  int i = 0;
  int16_t s = 0;
  AudioFrame a = {0};
  for (; i < b->soundcardBytesToWrite ; i += 4, b->soundcardPosFrames++) {
    a = b->frames[b->soundcardPosFrames % b->sizeInFrames];
    s = a.l * SHRT_MAX; /* Need to dither */
    b->output[i    ] = s & 255;
    b->output[i + 1] = s >> 8;
    s = a.r * SHRT_MAX; /* Need to dither */
    b->output[i + 2] = s & 255;
    b->output[i + 3] = s >> 8;
  }
}

void audioBuffer(AudioBuffer *b, int soundcardSizeFrames) {
  b->chunkSize              = AUDIO_CHUNK_SIZE;
  b->sizeInChunks           = AUDIO_CHUNKS;
  b->sizeInFrames           = AUDIO_BUFFER_SIZE;
  b->bufferFillThreshold    = AUDIO_BUFFER_SIZE / 2;
  b->chunksToFill           = AUDIO_CHUNKS / 3;
  b->currentChunk           = 0;
  b->soundcardFramesToWrite = soundcardSizeFrames;
  b->soundcardBytesToWrite  = soundcardSizeFrames * 4;
  b->soundcardPosFrames     = 0;
  b->framesGenerated        = 0;
  b->framesWritten          = 0;
  b->output                 = malloc(b->soundcardBytesToWrite);
}

void freeAudioBuffer(AudioBuffer *b) {
  free(b->output);
}
