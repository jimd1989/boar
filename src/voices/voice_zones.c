#include <unistd.h>

#include "../keyboard/keyboard.h"
#include "voice.h"
#include "voice_stack.h"
#include "voice_zones.h"

void splitZonesEvenly(VoiceZones *vz, Keyboard *kb, int n) {
  /* Splits keyboard into independent polyphonic zones. Intended for traditional
   * "ensemble" playing, like chords on one hand, lead on the other, etc. */
  int i             = 0;
  int j             = 0;
  int z             = 0;
  int keysPerZone   = KEYBOARD_SIZE / n;
  int voicesPerZone = VOICES_SIZE / n;
  VoiceStack *vs    = NULL;
  for (; i < KEYBOARD_SIZE ; z++) {
    for (j = 0 ; j < keysPerZone ; i++, j++) {
      kb->keys[i].zone = z;
    }
  }
  for (i = 0, z = 0; i < VOICES_SIZE ; z++) {
    vs = &vz->zones[z].free;
    for (j = 0 ; j < voicesPerZone ; i++, j++) {
      pushVoiceStack(vs, &vz->voices[i]);
    }
  }
}

void splitZonesWithLeftovers(VoiceZones *vz, Keyboard *kb, int min, int n) {
  /* Creates n monophonic zones starting at the min key. Any leftover
   * voices are allocated to the remainder of the keyboard. Intended for
   * "sampler" style playing, where each of these lower keys can trigger a
   * dedicated timbre. */
  int i = 0;
  int z = 0;
  for (; i < min ; i ++) {
    kb->keys[i].zone = z;
  }
  for (; z < n ; i++, z++) {
    kb->keys[i].zone = z;
    pushVoiceStack(&vz->zones[z].free, &vz->voices[z]);
  }
  for (; i < KEYBOARD_SIZE ; i++) {
    kb->keys[i].zone = z;
  }
  for (i = z ; i < VOICES_SIZE ; i++) {
    pushVoiceStack(&vz->zones[z].free, &vz->voices[i]);
  }
}

void voiceZones(VoiceZones *vz, Keyboard *kb) {
  int i = 0;
  for (; i < VOICES_SIZE ; i++) { voice(&vz->voices[i]); }
  vz->currentZone = 0;
  splitZonesEvenly(vz, kb, 1);
}
