#include <unistd.h>

#include "voice.h"
#include "voice_list.h"
#include "voice_stack.h"
#include "voices.h"

Voice * getVoice(Voices *vs) {
  Voice *v = NULL;
  if (vs->free.count > 0) {
    v = popVoiceStack(&vs->free);
    return v;
  } else {
    /* Stealing either oldest released voice or oldest playing voice */
    v = carVoiceList(&vs->released);
    if (v == NULL) { v = carVoiceList(&vs->playing); }
    return v;
  }
}

void voices(Voices *vs) {
  int i = 0;
  voiceList(&vs->playing);
  voiceList(&vs->released);
  voiceStack(&vs->free);
  voiceStack(&vs->stolen);
  for (; i < VOICES_SIZE ; i++) {
    voice(&vs->voices[i]);
    pushVoiceStack(&vs->free, &vs->voices[i]);
  }
}
