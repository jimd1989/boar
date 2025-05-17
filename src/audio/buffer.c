#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"
#include "noise.h"
#include "sample.h"
#include "settings.h"

void generateDsp(void *arg, int delta) {
  /* DSP updated in sio_write callback, hence the void *arg */
  int i = 0;
  AudioBuffer *b = (AudioBuffer *)arg;
  b->framesWritten += delta;
  if ((b->framesGenerated - b->framesWritten) <= b->bufferFillThreshold) {
    for( ; i < b->chunksToFill ; i++) {
      noise(&b->noise[b->currentChunk * b->chunkSize], b->chunkSize);
      b->currentChunk = (b->currentChunk + 1) % b->sizeInChunks;
      b->framesGenerated += b->chunkSize;
    }
  }
}

void fillBuffer(AudioBuffer *b) {
  int i = 0;
  int16_t s = 0;
  AudioFrame a = {0};
  for (; i < b->soundcardBytesToWrite ; i += 4) {
    a = b->frames[b->soundcardPosFrames];
    s = a.l * SHRT_MAX; /* Need to dither */
    b->output[i    ] = s & 255;
    b->output[i + 1] = s >> 8;
    s = a.r * SHRT_MAX; /* Need to dither */
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
  b->soundcardFramesToWrite = soundcardSizeFrames;
  b->soundcardBytesToWrite  = soundcardSizeFrames * 4;
  b->sizeInFrames           = soundcardSizeFrames;
  b->sizeInFrames          *= 4; /* Excessive? */
  b->sizeInFrames          += (b->sizeInFrames % AUDIO_CHUNK_SIZE);
  b->sizeInChunks           = (b->sizeInFrames / AUDIO_CHUNK_SIZE);
  b->bufferFillThreshold    = b->sizeInFrames / 2;
  b->chunksToFill           = b->sizeInChunks / 2;
  b->frames                 = calloc(b->sizeInFrames, sizeof(AudioFrame));
  b->noise                  = calloc(b->sizeInFrames, sizeof(AudioFrame));
  b->output                 = calloc(b->soundcardBytesToWrite, 1);
}

void freeAudioBuffer(AudioBuffer *b) {
  free(b->frames);
  free(b->noise);
  free(b->output);
}
