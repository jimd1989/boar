#pragma once

#include "../audio/sample.h"
#include "../chunk/chunk.h"

#define VOICES_SIZE 8

typedef struct Voice {
  /* Every voice is doubly-linked to allow O(1) removal during note-off */
  int             note; /* Should eventually be complex pitch struct */
  /* Need some kind of "prev pitch" to interpolate voice stealing */
  struct Voice  * next;
  struct Voice  * prev;
  AudioFrame      audio[AUDIO_CHUNK_SIZE];
} Voice;

void voice(Voice *);
