#include <stdio.h>
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
    /* Deactivate the keyboard key associated with the stolen voice.
     * It will be reassigned outside of this function. */
    *v->key = NULL;
    return v;
  }
}

void playVoice(Voices *vs, Voice *v) {
  appendVoiceList(&vs->playing, v);
  v->status = VOICE_PLAYING;
}

void releaseVoice(Voices *vs, Voice *v) {
  removeVoiceList(&vs->playing, v);
  appendVoiceList(&vs->released, v);
  v->status = VOICE_RELEASED;
}

void retriggerVoice(Voices *vs, Voice *v) {
  if (v->status == VOICE_RELEASED) {
    removeVoiceList(&vs->released, v);
    appendVoiceList(&vs->playing, v);
    v->status = VOICE_PLAYING;
  }
  /* Restart envelope when it exists */
}

void voices(Voices *vs) {
  /* Once initiated, no actual Voice structs are assigned to these data
   * structures. These will be allocated by the VoiceZones object instead. */
  voiceList(&vs->playing);
  voiceList(&vs->released);
  voiceStack(&vs->free);
  voiceStack(&vs->stolen);
}

void printVoices(Voices *vs) {
  printf("P | ");
  printVoiceList(&vs->playing);
  printf("R | ");
  printVoiceList(&vs->released);
  printf("F | ");
  printVoiceStack(&vs->free);
  printf("S | ");
  printVoiceStack(&vs->stolen);
  printf("\n");
}
