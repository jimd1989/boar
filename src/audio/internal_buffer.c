#include <err.h>
#include <stdlib.h>

#include "chunk.h"
#include "internal_buffer.h"
#include "noise.h"
#include "sample.h"

void internalBuffer(InternalBuffer *i, int frames) {
  i->pos       = 0;
  i->chunkSize = AUDIO_CHUNK_SIZE;
  i->frames    = frames;
  i->chunks    = AUDIO_CHUNK_DIV(i->frames);
  i->audio     = calloc(i->frames, sizeof(AudioFrame));
  i->noise     = calloc(i->frames, sizeof(AudioFrame));
  warnx("INTERNAL %d frames %d chunks", i->frames, i->chunks);
}

void fillAudio(InternalBuffer *in, int chunks) {
  int i = 0;
  for( ; i < chunks ; i++) {
    noise(&in->noise[in->pos * in->chunkSize], in->chunkSize);
    in->pos = (in->pos + 1) % in->chunks;
  }
}

void freeInternalBuffer(InternalBuffer *i) {
  free(i->audio);
  free(i->noise);
}
