#include <err.h>
#include <stdlib.h>

#include "../chunk/chunk.h"
#include "../control/control.h"
#include "../control/volume.h"
#include "../noise/noise.h"
#include "internal_buffer.h"
#include "sample.h"

void internalBuffer(InternalBuffer *i, int frames, Control *c) {
  i->pos       = 0;
  i->frames    = frames;
  i->chunks    = AUDIO_CHUNK_DIV(i->frames);
  i->audio     = calloc(i->frames, sizeof(AudioSample));
  i->control   = c;
  noise(&i->noise, i->frames);
  warnx("dsp →\tframes: %d\tchunk size: %d", i->frames, AUDIO_CHUNK_SIZE);
}

void fillAudio(InternalBuffer *in, int chunks) {
  int i                     = 0;
  unsigned int j            = 0;
  int k                     = 0;
  float f                   = 0.0f;
  float v[AUDIO_CHUNK_SIZE] = {0}; /* HACK HACK HACK: should be matrix row */
  for( ; i < chunks ; i++) {
    fillNoiseChunk(&in->noise, in->pos * AUDIO_CHUNK_SIZE);
    /* Need actual DSP matrix here */
    for (j = 0; j < AUDIO_CHUNK_SIZE ; j++) {
      fillVolChunk(&in->control->vol, v);
      k              = j + (in->pos * AUDIO_CHUNK_SIZE);
      f              = NOISE_FLOAT(in->noise.pink[k].n);
      f             *= in->control->vol.val;
      in->audio[k].f = f;
    }
    in->pos = (in->pos + 1) % in->chunks;
  }
}

void freeInternalBuffer(InternalBuffer *i) {
  free(i->audio);
  freeNoise(&i->noise);
}
