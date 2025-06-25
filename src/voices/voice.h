#pragma once

#include "../audio/sample.h"
#include "../chunk/chunk.h"

#define VOICES_SIZE 8

typedef enum VoiceStatus {
  VOICE_FREE = 0,
  VOICE_PLAYING,
  VOICE_RELEASED,
  VOICE_STOLEN
} VoiceStatus;

typedef struct Voice {
  VoiceStatus       status;
  /* Need some kind of "prev inc" to interpolate voice stealing */
  int               note;
  float             inc;
  float             vel;
  /* Every voice is doubly-linked to allow O(1) removal during note-off */
  struct Voice  *   next;
  struct Voice  *   prev;
  struct Voice  **  key; /* Associated keyboard position (stealing) */
  AudioFrame        audio[AUDIO_CHUNK_SIZE];
} Voice;

void voice(Voice *);
