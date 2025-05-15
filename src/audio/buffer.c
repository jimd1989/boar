#include <err.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"
#include "noise.h"
#include "sample.h"
#include "settings.h"

/* Will eventually have synth object injected */
void generateDsp(AudioBuffer *b) {
  int i = 0;
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
  /* framesGenerated = Number of DSP frames created, stored in buffer.
   * framesWritten   = Number of frames output to soundcard.
   * When the difference between these values exceeds bufferThreshold, then
   * more DSP is needed. chunksToFill chunks of buffer frames of additional DSP
   * is written to keep ahead of the audio input. There's an interest in keeping
   * chunksToFill on the smaller side to make DSP more responsive to user input.
   */
  b->chunkSize              = AUDIO_CHUNK_SIZE;
  b->sizeInChunks           = AUDIO_CHUNKS;
  b->sizeInFrames           = AUDIO_BUFFER_SIZE;
  b->bufferFillThreshold    = AUDIO_BUFFER_SIZE / 2;
  b->chunksToFill           = AUDIO_CHUNKS / 2;
  b->currentChunk           = 0;
  b->soundcardFramesToWrite = soundcardSizeFrames;
  b->soundcardBytesToWrite  = soundcardSizeFrames * 4;
  b->soundcardPosFrames     = 0;
  b->framesGenerated        = 0;
  b->framesWritten          = 0;
  if (b->soundcardFramesToWrite > (b->chunksToFill * b->chunkSize)) {
    /* Indicates that the internal buffer is not large enough to write
     * ahead of soundcard's buffer. Figure out how to handle this. */
    warnx("Soundcard buffer size exceeds internal buffer.");
  }
  b->output                 = calloc(1, b->soundcardBytesToWrite);
}

void freeAudioBuffer(AudioBuffer *b) {
  free(b->output);
}
