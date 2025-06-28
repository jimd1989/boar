#pragma once

#include "../keyboard/keyboard.h"
#include "../mixer/mixer.h"

#include "voice.h"
#include "voices.h"

/* A zone is a subsection of the keyboard with its own independent voice stack
 * and parameters. This should permit multitimbrality. Parameter changes are
 * addressed statefully according to the "currentZone" field, but some syntax
 * hacks can make this more transparent. */
typedef struct VoiceZones {
  int     currentZone;
  Mixer   mixer;
  Voices  zones[VOICES_SIZE];
  Voice   voices[VOICES_SIZE];
} VoiceZones;

void splitZonesEvenly(VoiceZones *, Keyboard *, int);
void splitZonesWithLeftovers(VoiceZones *, Keyboard *, int, int);
void voiceZones(VoiceZones *, Keyboard *, int);
