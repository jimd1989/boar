#pragma once

#include "voice.h"
#include "voice_list.h"
#include "voice_stack.h"

typedef struct Voices {
  VoiceList   playing;
  VoiceList   released;
  VoiceStack  free;
  VoiceStack  stolen;
  Voice       voices[VOICES_SIZE];
} Voices;

Voice * getVoice(Voices *);
void playVoice(Voices *, Voice *);
void releaseVoice(Voices *, Voice *);
void voices(Voices *);
void printVoices(Voices *);
