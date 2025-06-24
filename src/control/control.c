#include <stdint.h>
#include <unistd.h>

#include "../keyboard/keyboard.h" 
#include "../voices/voices.h"
#include "../voices/voice.h"
#include "control.h"
#include "volume.h"

void control(Control *c) {
  volume(&c->vol, c->curves.cubic); 
  curves(&c->curves);
  voices(&c->voices);
  keyboard(&c->keyboard);
}

void noteOn(Control *c, uint8_t note, uint8_t vel) {
  Key *k   = &c->keyboard.keys[note];
  Voice *v = getVoice(&c->voices);
  if (k->voice != NULL) {
    /* Some kind of retrigger mechanism needed here */
    return;
  }
  v->inc   = k->inc;
  /* There will eventually be a velocity sensitivity curve */
  v->vel   = (float)vel * CONTROL_127_DIV;
  k->voice = v;
  v->key   = &k->voice;
  playVoice(&c->voices, v);
  printVoices(&c->voices);
}

void noteOff(Control *c, uint8_t note, uint8_t vel) {
  Key *k   = &c->keyboard.keys[note];
  Voice *v = k->voice;
  if (v == NULL) { return; }
  k->voice = NULL;
  v->vel   = (float)vel * CONTROL_127_DIV;
  releaseVoice(&c->voices, v);
  printVoices(&c->voices);
}
