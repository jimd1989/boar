#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"
#include "noise.h"
#include "output_buffer.h"
#include "sample.h"
#include "settings.h"

void generateDsp(void *arg, int delta) {
  /* DSP updated in sio_write callback, hence the void *arg */
  int i = 0;
  int diff = 0;
  int chunksToFill = 0;
  AudioBuffer *b = (AudioBuffer *)arg;
  b->outputBuffer.framesWritten += delta;
  if (b->framesGenerated < b->outputBuffer.framesWritten) {
    /* Catching up to an underrun */
    b->outputBuffer.currentChunk = b->currentChunk; /* Need more math */
    //b->soundcardPosFrames = b->currentChunk * b->chunkSize;
    /* Add an extra soundcard buffer of distance to get further ahead */
    diff                  = b->outputBuffer.framesWritten - b->framesGenerated;
    chunksToFill          = diff / b->chunkSize;
    chunksToFill         += (diff % b->chunkSize) == 0 ? 0 : 1;
    chunksToFill         += 1 + b->soundcardChunksToWrite;
    /* Don't actually write up. Accept the glitch. */
    b->currentChunk       = (b->currentChunk + chunksToFill) % b->sizeInChunks;
    b->framesGenerated   += chunksToFill * b->chunkSize;
  } else {
    /* Normal buffer write-ahead */
    chunksToFill  = b->soundcardChunksToWrite;
    chunksToFill += (b->outputBuffer.framesWritten % b->chunkSize) == 0 ? 0 : 1;
    for( ; i < chunksToFill ; i++) {
      noise(&b->noise[b->currentChunk * b->chunkSize], b->chunkSize);
      b->currentChunk = (b->currentChunk + 1) % b->sizeInChunks;
    }
    b->framesGenerated += chunksToFill * b->chunkSize;
  }
}

void fillBuffer(AudioBuffer *b) {
  int i = 0;
  int16_t s = 0;
  AudioFrame a = {0};
  for (; i < b->soundcardBytesToWrite ; i += 4) {
    a = b->noise[b->soundcardPosFrames];
    s = 0.01f * a.l * SHRT_MAX; /* Need to dither */
    b->output[i    ] = s & 255;
    b->output[i + 1] = s >> 8;
    s = 0.01f * a.r * SHRT_MAX; /* Need to dither */
    b->output[i + 2] = s & 255;
    b->output[i + 3] = s >> 8;
    b->soundcardPosFrames = (b->soundcardPosFrames + 1) % b->sizeInFrames;
  }
}

void audioBuffer(AudioBuffer *b, int soundcardSizeFrames) {
  /* framesGenerated = Number of DSP frames created, stored in buffer.
   * framesWritten   = Number of frames output to soundcard.
   * When the difference between these values exceeds bufferThreshold, then
   * more DSP is needed. chunksToFill chunks of buffer frames of additional DSP
   * is written to keep ahead of the audio input. There's an interest in keeping
   * chunksToFill on the smaller side to make DSP more responsive to user input.
   */
  b->currentChunk           = 0;
  b->soundcardPosFrames     = 0;
  b->framesGenerated        = 0;
  b->framesWritten          = 0;
  b->chunkSize              = AUDIO_CHUNK_SIZE;
  b->soundcardFramesToWrite = soundcardSizeFrames;
  b->soundcardBytesToWrite  = soundcardSizeFrames * 4;
  b->soundcardChunksToWrite = soundcardSizeFrames / b->chunkSize;
  b->sizeInFrames           = soundcardSizeFrames;
  b->sizeInFrames          *= 4; /* Excessive? */
  b->sizeInFrames          += b->chunkSize - (b->sizeInFrames % b->chunkSize);
  b->sizeInChunks           = (b->sizeInFrames / b->chunkSize);
  b->frames                 = calloc(b->sizeInFrames, sizeof(AudioFrame));
  b->noise                  = calloc(b->sizeInFrames, sizeof(AudioFrame));
  b->output                 = calloc(b->soundcardBytesToWrite, 1);
  outputBuffer(&b->outputBuffer, b->noise, b->sizeInFrames, soundcardSizeFrames);
}

void freeAudioBuffer(AudioBuffer *b) {
  free(b->frames);
  free(b->noise);
  free(b->output);
  freeOutputBuffer(&b->outputBuffer);
}
