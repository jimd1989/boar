#include <stdlib.h>

#include "buffer.h"
#include "internal_buffer.h"
#include "output_buffer.h"

void audioBuffer(AudioBuffer *b, int soundcardSizeFrames) {
  int n = 3;
  b->gens = 0;
  b->writes   = 0;
  internalBuffer(&b->i, soundcardSizeFrames * n);  
  outputBuffer(&b->o, b->i.noise, soundcardSizeFrames * n, soundcardSizeFrames);
  b->soundcardChunks = (b->o.writeChunks * b->o.chunkSize) / b->i.chunkSize;
}

void generateDsp(void *arg, int delta) {
  /* DSP updated in sio_write callback, hence the void *arg */
  int diff       = 0;
  int chunks     = 0;
  AudioBuffer *b = (AudioBuffer *)arg;
  b->writes     += delta;
  if (b->gens < b->writes) {
    /* Catching up to an underrun: */
    /* 1. Reset output position to current DSP position. */
    b->o.pos = (b->i.pos * b->i.chunkSize) / b->o.chunkSize;
    /* 2. Add an extra soundcard buffer of distance to get further ahead */
    diff    = b->writes - b->gens;
    chunks  = diff / b->i.chunkSize;
    chunks += (diff % b->i.chunkSize) == 0 ? 0 : 1;
    chunks += 1 + b->soundcardChunks;
    /* 3. Don't actually write. Accept the glitch and change DSP position. */
    b->i.pos  = (b->i.pos + chunks) % b->i.chunks;
    b->gens  += chunks * b->i.chunkSize;
  } else {
    /* Normal buffer write-ahead */
    chunks   = b->soundcardChunks;
    chunks  += (b->writes % b->i.chunkSize) == 0 ? 0 : 1;
    fillAudio(&b->i, chunks);
    b->gens += chunks * b->i.chunkSize;
  }
}

void freeAudioBuffer(AudioBuffer *b) {
  freeInternalBuffer(&b->i);
  freeOutputBuffer(&b->o);
}
