#include <string.h>
#include <unistd.h>

#include "../audio/sample.h"
#include "../chunk/chunk.h"
#include "voice.h"

void freeVoice(Voice *v) {
  if (v->key != NULL) { *v->key = NULL; }
  v->key    = NULL;
  v->prev   = NULL;
  v->next   = NULL;
  v->status = VOICE_FREE;
}

void voice(Voice *v) {
  v->status = VOICE_FREE;
  v->key    = NULL;
  v->prev   = NULL;
  v->next   = NULL;
  memset(v->audio, 0, AUDIO_CHUNK_SIZE * sizeof(AudioSample));
}
