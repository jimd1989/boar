/* Functions that calculate the pitch, phase, tuning offset, velocity, and 
 * key follow values of individual notes on the keyboard. These values are 
 * assigned to Osc.KeyMod, which is applied during the synthesis process. */

#include <err.h>
#include <stdint.h>

#include "key.h"

#include "maximums.h"
#include "numerical.h"
#include "synthesis.h"
#include "velocity.h"

static float applyKeyFollowCurve(const Wave *, const unsigned int);
static void applyKeyboardLayer(const KeyboardLayer *, const KeyboardSettings *,
    Operator *, const uint16_t);
static void makeKeyboardLayer(KeyboardLayer *);

unsigned int
getNote(const uint16_t note) {

/* Returns the lower 7 bits of note, indicating which MIDI note to play. */

  return note & MAX_MIDI_VALUE;
}

static float
applyKeyFollowCurve(const Wave *kw, const unsigned int note) {

/* Returns the key follow curve value for a specific note. It is assumed that
 * only the bottom 7 bits of a note are passed to this function via
 * getNote(). */

  return interpolateCycle(kw, getNote(note) / (float)MAX_MIDI_VALUE); 
}

void
selectTuningLayer(Keyboard *k, const TuningLayer tl) {

/* Selects which set of tunings should be modified by the u/U commands. */

  switch ((unsigned int)tl) {
    case TUNING_CARRIER:
      k->TuningLayer = k->Carrier.Tunings;
      break;
    case TUNING_MODULATOR:
      k->TuningLayer = k->Modulator.Tunings;
      break;
    default:
      warnx("Invalid tuning layer. "\
          "Select %u (carrier) or %u (modulator).",
          TUNING_CARRIER, TUNING_MODULATOR);
  }
}

void
selectTuningKey(Keyboard *k, const unsigned int note) {

/* Sets the specific key to be tuned by the u command. */    

  if (note > MAX_MIDI_VALUE) {
    warnx("Note must be between 0 and %u", MAX_MIDI_VALUE);
    return;
  }
  k->TuningNote = &k->TuningLayer[note];
}

void
tuneKey(Keyboard *k, const float tuning) {

/* Sets the tuning offset for the key selected by the U command. */

  *k->TuningNote = tuning;
}

static void
applyKeyboardLayer(const KeyboardLayer *kl, const KeyboardSettings *ks, 
    Operator *o, const uint16_t n) {

/* Modifies Osc.Pitch and Osc.KeyMod based upon the tuning, velocity, and key
 * follow settings of the KeyboardLayer. */

  const unsigned int note = getNote(n);

  setPitch(o, note, ks->Rate);
  o->Osc.Pitch *= kl->Tunings[note];
  o->Osc.Phase = (float)*ks->Phase * o->Osc.Pitch;
  o->Osc.KeyMod = applyVelocityCurve(&kl->VelocityCurve, n) *
    applyKeyFollowCurve(&kl->KeyFollowCurve, note);
}

void
applyKey(const Keyboard *k, Operator *c, Operator *m, const uint16_t note) {

/* Modifies an Operator's carrier and modulator in terms of their respective
 * KeyboardLayer settings. */

  applyKeyboardLayer(&k->Carrier, &k->Settings, c, note);
  applyKeyboardLayer(&k->Modulator, &k->Settings, m, note);
}

static void
makeKeyboardLayer(KeyboardLayer *kl) {

/* Initializes all the elements of a KeyboardLayer type. */

  int i = 0;

  for (; i < DEFAULT_KEYS_NUM ; i++) {
    kl->Tunings[i] = 1.0f;
  }
  selectWave(&kl->VelocityCurve, WAVE_TYPE_FLAT);
  selectWave(&kl->KeyFollowCurve, WAVE_TYPE_FLAT);
}

void
makeKeyboard(Keyboard *k, const unsigned int rate, uint64_t *phase) {

/* Initializes all the elements of a Keyboard type. */

  makeKeyboardLayer(&k->Carrier);
  makeKeyboardLayer(&k->Modulator);
  selectTuningLayer(k, TUNING_CARRIER);
  selectTuningKey(k, 60);
  k->Settings.Rate = rate;
  k->Settings.Phase = phase;
}
