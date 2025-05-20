#include <stdlib.h>

#include "buffer.h"
#include "internal_buffer.h"
#include "output_buffer.h"
#include "settings.h"

static int snapToMultiple(int, int);

static int snapToMultiple(int n, int m) {
  return m * ((n + (m - 1)) / m);
}

void audioBuffer(AudioBuffer *b, int soundcardSizeFrames) {
  /* Round total buffer to chunk size */
  int internalFrames = soundcardSizeFrames;
  b->mult            = 3;
  internalFrames    *= b->mult;
  internalFrames     = snapToMultiple(internalFrames, AUDIO_CHUNK_SIZE);
  internalFrames    += AUDIO_CHUNK_SIZE; /* Extra chunk of headroom */
  b->fractionalPhase = 0;
  internalBuffer(&b->i, internalFrames);  
  outputBuffer(&b->o, b->i.noise, internalFrames, soundcardSizeFrames);
}

void generateDsp(void *arg, int delta) {
  /* Only keep track of bytesWritten for rounding purposes */
  /* DSP updated in sio_write callback, hence the void *arg */
  int chunks     = 0;
  AudioBuffer *b = (AudioBuffer *)arg;
  if (delta == 0) {
    /* Catching up to an underrun:
     * 1. Reset audio output to current internal DSP position
     * 2. Fill entire internal buffer - 1 soundcard buffer */
    /* Hard reset everything to zero. Accept the glitch. */
    b->i.pos           = 0;
    b->o.pos           = 0;
    b->fractionalPhase = 0;
    /* The writing of (mult - 1) soundcard buffers might be slightly more than
     * the actual number of soundcard frames due to DSP block size differences,
     * but there should be enough internal buffer headroom to prevent
     * overwriting. */
    chunks = (b->mult - 1) * (b->o.writeChunks * b->o.chunkSize);
    chunks = snapToMultiple(chunks, b->i.chunkSize) / b->i.chunkSize;
    fillAudio(&b->i, chunks);
  } else {
    /* Normal buffer write-ahead: 
     * 1. Fill up as many internal DSP chunks as the delta requires.
     * 2. Round based upon output buffer chunk size. */
    chunks              = delta / b->i.chunkSize;
    b->fractionalPhase += delta % b->i.chunkSize;
    if (b->fractionalPhase >= b->i.chunkSize) {
      chunks             += 1;
      b->fractionalPhase -= b->i.chunkSize;
    }
    fillAudio(&b->i, chunks);
  }
}

void freeAudioBuffer(AudioBuffer *b) {
  freeInternalBuffer(&b->i);
  freeOutputBuffer(&b->o);
}
