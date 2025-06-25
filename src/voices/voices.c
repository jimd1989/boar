#include <err.h>

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
    warnx("STEALING VOICE");
    /* Stealing either oldest released voice or oldest playing voice */
    v = carVoiceList(&vs->released);
    if (v == NULL) { 
      v = carVoiceList(&vs->playing);
    }
    /* How to communicate back to keyboard? */
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
