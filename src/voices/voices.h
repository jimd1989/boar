#pragma once

#include "voice.h"

#define VOICES_SIZE 8

typedef struct VoiceStack {
  int     count;
  Voice * voices[VOICES_SIZE];
} VoiceStack;

typedef struct VoiceQueue {
  int     count;
  int     readPos;
  int     writePos;
  Voice * voices[VOICES_SIZE];
} VoiceQueue;

typedef struct Voices {
  VoiceQueue  playing;
  VoiceQueue  released;
  VoiceStack  free;
  VoiceStack  stolen;
  Voice       voices[VOICES_SIZE];
} Voices;

void voices(Voices *);
