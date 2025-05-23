#include <stdlib.h>

#include "buffer.h"
#include "chunk.h"
#include "internal_buffer.h"
#include "output_buffer.h"

static int snapToMultiple(int, int);

static int snapToMultiple(int n, int m) {
  return m * ((n + (m - 1)) / m);
}

void audioBuffer(AudioBuffer *b, int soundcardSizeFrames) {
  /* Round total buffer to chunk size */
  int chunks         = 0;
  int internalFrames = soundcardSizeFrames;
  b->mult            = 3;
  internalFrames    *= b->mult;
  internalFrames     = snapToMultiple(internalFrames, AUDIO_CHUNK_SIZE);
  internalFrames    += AUDIO_CHUNK_SIZE; /* Extra chunk of headroom */
  b->fractionalPhase = 0;
  internalBuffer(&b->i, internalFrames);  
  /* Prefill the entire buffer. 
   * Once this is something other than white noise, b->mult could be a cause
   * of any distortion. Double-check. */
  outputBuffer(&b->o, b->i.noise.white, internalFrames, soundcardSizeFrames);
  chunks       = (b->mult) * (b->o.writeChunks * b->o.chunkSize);
  chunks       = snapToMultiple(chunks, b->i.chunkSize) / b->i.chunkSize;
  b->maxChunks = chunks;
  fillAudio(&b->i, chunks);
}

void generateDsp(void *arg, int delta) {
  /* DSP updated in sio_write callback, hence the void *arg */
  int chunks      = 0;
  int extraChunks = 0;
  AudioBuffer *b  = (AudioBuffer *)arg;
  if (delta <= 0) {
    /* Underrun: don't do any additional DSP */
    return;
  }
  /* Normal buffer write-ahead: 
   * 1. Fill up as many internal DSP chunks as the delta requires.
   * 2. Round based upon output buffer chunk size.
   * 3. Subsequent underruns can queue up many chunks of backfill, leading
   *    to cascading write failures. chunks should never exceed the DSP buffer
   *    length.
   * 4. No special handling of xruns needed beyond ensuring the buffer sizes
   *    are respected. Just accept any glitches and move on.
   * 5. As long as b->chunkSize is a power of two, div and modulo can be
   *    bitwise. */
  chunks              = AUDIO_CHUNK_DIV(delta);
  b->fractionalPhase += AUDIO_CHUNK_MOD(delta);
  extraChunks         = AUDIO_CHUNK_DIV(b->fractionalPhase);
  b->fractionalPhase -= AUDIO_CHUNK_MULT(extraChunks);
  chunks             += extraChunks;
  chunks              = chunks < b->maxChunks ? chunks : b->maxChunks;
  fillAudio(&b->i, chunks);
}

void freeAudioBuffer(AudioBuffer *b) {
  freeInternalBuffer(&b->i);
  freeOutputBuffer(&b->o);
}
