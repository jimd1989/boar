#pragma once

#include "voice.h"

typedef struct VoiceList {
  Voice * head;
  Voice * last;
} VoiceList;

void appendVoiceList(VoiceList *, Voice *);
void removeVoiceList(VoiceList *, Voice *);
Voice * carVoiceList(VoiceList *);
void drainVoiceList(VoiceList *);
void voiceList(VoiceList *);
void printVoiceList(VoiceList *);
