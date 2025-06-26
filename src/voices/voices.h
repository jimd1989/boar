#pragma once

#include "voice.h"
#include "voice_list.h"
#include "voice_stack.h"

/* Represents a zone-local cluster of polyphonic notes, governed by common
 * parameters. Most data structures here are built around rationing voices when 
 * there aren't enough to play every intended note. 
 * Some expectations for voice assignment:
 * - If there are free voices, use one before stealing anything.
 * - If there are released voices, steal the oldest one of these first.
 * - If there are only playing voices, steal the oldest one of these next.
 * - Stolen voices should be segregated for one audio block to crossfade between
 *   the old voice and the new one (should prevent click).
 * - Retriggering a released (but not yet free) note should reuse its voice in
 *   the released queue. This resets its age. */
typedef struct Voices {
  VoiceList   playing;
  VoiceList   released;
  VoiceStack  free;
  VoiceStack  stolen;
} Voices;

Voice * getVoice(Voices *);
void playVoice(Voices *, Voice *);
void releaseVoice(Voices *, Voice *);
void retriggerVoice(Voices *, Voice *);
void drainVoices(Voices *);
void voices(Voices *);
void printVoices(Voices *);
