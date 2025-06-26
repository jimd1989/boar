#pragma once

#include "voice.h"

typedef struct VoiceStack {
  int     count;
  Voice * voices[VOICES_SIZE];
} VoiceStack;

Voice * popVoiceStack(VoiceStack *);
void pushVoiceStack(VoiceStack *, Voice *);
void drainVoiceStack(VoiceStack *);
void voiceStack(VoiceStack *);
void printVoiceStack(VoiceStack *);
