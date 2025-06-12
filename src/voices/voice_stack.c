#include <unistd.h>

#include "voice.h"
#include "voice_stack.h"

Voice * popVoiceStack(VoiceStack *vs) {
  return vs->voices[--vs->count];
}

void pushVoiceStack(VoiceStack *vs, Voice *v) {
  vs->voices[vs->count++] = v;
}

void voiceStack(VoiceStack *vs) {
  int i = 0;
  vs->count = 0;
  for (; i < VOICES_SIZE; i++) { vs->voices[i] = NULL; }
}
