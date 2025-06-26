#include <unistd.h>

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
}
