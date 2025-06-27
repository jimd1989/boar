#include <unistd.h>

#include "../keyboard/keyboard.h"
#include "voice.h"
#include "voice_stack.h"
#include "voice_zones.h"

static void resetAllVoices(VoiceZones *);

static void resetAllVoices(VoiceZones *vz) {
  int i = 0;
  for (; i < VOICES_SIZE ; i++) { drainVoices(&vz->zones[i]); }
}

void splitZonesEvenly(VoiceZones *vz, Keyboard *kb, int n) {
  /* Splits keyboard into independent polyphonic zones. Intended for traditional
   * "ensemble" playing, like chords on one hand, lead on the other, etc. */
  int i             = 0;
  int j             = 0;
  int z             = 0;
  int keysPerZone   = KEYBOARD_SIZE / n;
  int voicesPerZone = VOICES_SIZE / n;
  VoiceStack *vs    = NULL;
  resetAllVoices(vz);
  printKeyboard(kb);
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
   * dedicated timbre. Where n = 1, can do a classic bass/poly split in the
   * keyboard starting at min key. */
  int i = 0;
  int z = 0;
  resetAllVoices(vz);
  for (; i < min ; i ++) {
    /* Fill all keys up until min with the first monophonic zone. */
    kb->keys[i].zone = z;
  }
  for (; z < n ; i++, ++z) {
    /* Assign all monozones. */
    kb->keys[i].zone = z;
    pushVoiceStack(&vz->zones[z].free, &vz->voices[z]);
  }
  for (; i < KEYBOARD_SIZE ; i++) {
    /* Fill remaining keys with the last zone. */
    kb->keys[i].zone = z;
  }
  for (i = z ; i < VOICES_SIZE ; i++) {
    /* Assign all remaining voices to the last zone. */
    pushVoiceStack(&vz->zones[z].free, &vz->voices[i]);
  }
}

void voiceZones(VoiceZones *vz, Keyboard *kb) {
  int i = 0;
  for (; i < VOICES_SIZE ; i++) { voice(&vz->voices[i]); }
  vz->currentZone = 0;
  splitZonesEvenly(vz, kb, 1);
}
