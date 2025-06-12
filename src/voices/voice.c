#include <unistd.h>

#include "voice.h"

void voice(Voice *v) {
  v->prev   = NULL;
  v->next   = NULL;
}
