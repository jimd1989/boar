#include <err.h>
#include <stdlib.h>

#include "../control/control.h"
#include "../control/volume.h"
#include "../noise/noise.h"
#include "chunk.h"
#include "internal_buffer.h"
#include "sample.h"

void internalBuffer(InternalBuffer *i, int frames, Control *c) {
  i->pos       = 0;
  i->chunkSize = AUDIO_CHUNK_SIZE;
  i->frames    = frames;
  i->chunks    = AUDIO_CHUNK_DIV(i->frames);
  i->audio     = calloc(i->frames, sizeof(AudioFrame));
  i->control   = c;
  noise(&i->noise, i->frames);
  warnx("dsp →\tframes: %d\tchunk size: %d", i->frames, i->chunkSize);
}

void fillAudio(InternalBuffer *in, int chunks) {
  int i          = 0;
  unsigned int j = 0;
  int k          = 0;
  float f        = 0.0f;
  float v[64]    = {0}; /* HACK HACK HACK: should be matrix row */
  for( ; i < chunks ; i++) {
    fillNoise(&in->noise, in->pos * in->chunkSize, in->chunkSize);
    /* Need actual DSP matrix here */
    for (j = 0; j < in->chunkSize ; j++) {
      fillVol(&in->control->vol, v);
      k                = j + (in->pos * in->chunkSize);
      f                = NOISE_FLOAT(in->noise.pink[k].l.n);
      f               *= in->control->vol.val;
      in->audio[k].l.f = f;
      f                = NOISE_FLOAT(in->noise.pink[k].r.n);
      f               *= in->control->vol.val;
      in->audio[k].r.f = f;
    }
    in->pos = (in->pos + 1) % in->chunks;
  }
}

void freeInternalBuffer(InternalBuffer *i) {
  free(i->audio);
  freeNoise(&i->noise);
}
