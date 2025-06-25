#include <unistd.h>

#include "voice.h"

void voice(Voice *v) {
  v->status = VOICE_FREE;
  v->prev   = NULL;
  v->next   = NULL;
}
