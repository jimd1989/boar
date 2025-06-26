#include <stdint.h>
#include <unistd.h>

#include "../keyboard/keyboard.h" 
#include "../voices/voice_zones.h"
#include "../voices/voices.h"
#include "../voices/voice.h"
#include "control.h"
#include "volume.h"

void control(Control *c) {
  volume(&c->vol, c->curves.cubic); 
  curves(&c->curves);
  voiceZones(&c->voiceZones, &c->keyboard);
  keyboard(&c->keyboard);
}

void noteOn(Control *c, uint8_t note, uint8_t vel) {
  Key *k     = &c->keyboard.keys[note];
  Voices *vs = &c->voiceZones.zones[k->zone]; 
  Voice *v   = NULL;
  if (k->voice != NULL) {
    /* Retrigger an active voice */
    v      = k->voice;
    v->vel = (float)vel * CONTROL_127_DIV;
    retriggerVoice(vs, v);
    printVoices(vs);
    printKeyboard(&c->keyboard);
    return;
  }
  v        = getVoice(vs);
  v->note  = note;
  v->inc   = k->inc;
  /* There will eventually be a velocity sensitivity curve */
  v->vel   = (float)vel * CONTROL_127_DIV;
  k->voice = v;
  v->key   = &k->voice;
  playVoice(vs, v);
  printVoices(vs);
  printKeyboard(&c->keyboard);
}

void noteOff(Control *c, uint8_t note, uint8_t vel) {
  Key *k     = &c->keyboard.keys[note];
  Voices *vs = &c->voiceZones.zones[k->zone]; 
  Voice *v   = k->voice;
  if (v == NULL) { return; /* Voice was already stolen. */ }
  v->vel   = (float)vel * CONTROL_127_DIV;
  releaseVoice(vs, v);
  printVoices(vs);
  printKeyboard(&c->keyboard);
}
