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
  b->delta           = 0;
  b->fractionalPhase = 0;
  internalBuffer(&b->i, internalFrames);  
  /* Prefill entire buffer - 1 soundcards. The actual number of soundcard frames
   * filled might differ slightly due to DSP block size differences, but there
   * should be enough headroom to prevent overwriting. */
  outputBuffer(&b->o, b->i.noise, internalFrames, soundcardSizeFrames);
  chunks       = (b->mult - 1) * (b->o.writeChunks * b->o.chunkSize);
  chunks       = snapToMultiple(chunks, b->i.chunkSize) / b->i.chunkSize;
  fillAudio(&b->i, chunks);
}

void generateDsp(void *arg, int delta) {
  /* DSP updated in sio_write callback, hence the void *arg */
  int chunks      = 0;
  int extraChunks = 0;
  AudioBuffer *b  = (AudioBuffer *)arg;
  b->delta        = delta; /* clamp this to maxFrames? */
  if (delta <= 0) {
    /* Underrun: don't do any additional DSP */
    return;
  }
  /* Normal buffer write-ahead: 
   * 1. Fill up as many internal DSP chunks as the delta requires.
   * 2. Round based upon output buffer chunk size.
   * 3. Subsequent underruns can queue up many chunks of backfill, leading
   *    to cascading write failures. chunks should never exceed the DSP buffer
   *    length - 1 soundcard buffer.
   * 4. No special handling of xruns needed beyond ensuring the buffer sizes
   *    are respected. Just accept any glitches and move on. */
  chunks              = delta / b->i.chunkSize; /* can be bitwize */
  b->fractionalPhase += delta % b->i.chunkSize; /* can be bitwize */
  extraChunks         = b->fractionalPhase / b->i.chunkSize;
  b->fractionalPhase -= extraChunks * b->i.chunkSize;
  chunks             += extraChunks;
  chunks              = chunks < b->maxChunks ? chunks : b->maxChunks;
  fillAudio(&b->i, chunks);
}

void freeAudioBuffer(AudioBuffer *b) {
  freeInternalBuffer(&b->i);
  freeOutputBuffer(&b->o);
}
