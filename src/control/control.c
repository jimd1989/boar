#include <err.h>

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
  Voice *v = NULL;
  /* Retrigger active voice */
  if (k->voice != NULL) {
    warnx("RETRIGGER %d %p", note, (void *)k->voice->prev);
    v      = k->voice;
    v->vel = (float)vel * CONTROL_127_DIV;
    retriggerVoice(&c->voices, v);
    printVoices(&c->voices);
    printKeyboard(&c->keyboard);
    return;
  }
  warnx("NOTE ON %d", note);
  v        = getVoice(&c->voices);
  v->note  = note;
  v->inc   = k->inc;
  /* There will eventually be a velocity sensitivity curve */
  v->vel   = (float)vel * CONTROL_127_DIV;
  k->voice = v;
  v->key   = &k->voice;
  playVoice(&c->voices, v);
  printVoices(&c->voices);
  printKeyboard(&c->keyboard);
}

void noteOff(Control *c, uint8_t note, uint8_t vel) {
  Key *k   = &c->keyboard.keys[note];
  Voice *v = k->voice;
  warnx("NOTE OFF %d", note);
  if (v == NULL) { return; }
  /* k->voice = NULL; Not yet! */
  v->vel   = (float)vel * CONTROL_127_DIV;
  releaseVoice(&c->voices, v);
  printVoices(&c->voices);
  printKeyboard(&c->keyboard);
}
