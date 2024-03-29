/* These functions contain the actual mathematics that create sound. */

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "synthesis.h"

#include "constants/defaults.h"
#include "envelope.h"
#include "noise.h"
#include "numerical.h"
#include "wave.h"

static float hzToPitch(const float, const unsigned int);
static float pitch(const unsigned int, const unsigned int);
static int wavetableIndex(const int, const float);
static void fillModulatorBuffer(Operator *);
static float modulate(Osc *, Osc *, const unsigned int);

static float
hzToPitch(const float hz, const unsigned int rate) {

/* Converts a frequency in Hz to a wavetable increment value. */

  return hz * ((float)DEFAULT_WAVELEN / (float)rate);
}

static float
pitch(const unsigned int note, const unsigned int rate) {

/* Calculates the phase increment value for a pitch at the MIDI key number
 * "note" with regards to the sample rate given in "rate". */

  const float frequency = DEFAULT_LOWEST_FREQUENCY * 
    powf(2.0f, (float)note / 12.0f);

  return hzToPitch(frequency, rate);
}

void
setPitch(Operator *o, const unsigned int note, const unsigned int rate) {

/* Sets the pitch of an Osc derived from a note, or its fixed frequency. */

  if (*o->FixedRate) {
    o->Osc.Pitch = hzToPitch(*o->FixedRate, rate);
    return;
  }
  o->Osc.Pitch = *o->Ratio * pitch(note, rate);
}

static int
wavetableIndex(const int complexity, const float pitch) {

/* Return the index of proper wavetable to sample, based upon pitch and offset
 * from Osc.Complexity. This avoids reflected harmonics, unless the user is 
 * specifically trying to create them. */

  int tn = complexity + 1 + (int)ilogb(DEFAULT_OCTAVE_SCALING * pitch);
  if (tn < 0) {
    return 0;
  } else if (tn >= DEFAULT_OCTAVES) {
    return DEFAULT_OCTAVES - 1;
  }
  return tn;
}

static void
fillModulatorBuffer(Operator *m) {

/* Assigns an interpolated float sample to every index of the Osc buffer, 
 * derived from Osc.Pitch. This buffer is later used to modulate the carrier 
 * signal. */

  unsigned int i = 0;
  Osc *o = &m->Osc;
  int tableNo = wavetableIndex(*o->Complexity, o->Pitch);

  if (o->Wave->Type == WAVE_TYPE_NOISE) {
    for (; i < DEFAULT_BUFSIZE ; i++) {
      o->Buffer[i] = readNoise(&o->Wave->Noise, o->Pitch) *
        o->Amplitude * applyEnv(&m->Env) * o->KeyMod;
    }
  } else {
    for (; i < DEFAULT_BUFSIZE ; i++) {
      o->Phase = fmodf(o->Phase + o->Pitch, (float)DEFAULT_WAVELEN);
      o->Buffer[i] = interpolate(o->Wave->Table[tableNo],
          DEFAULT_WAVELEN, o->Phase) * o->Amplitude *
        applyEnv(&m->Env) * o->KeyMod;
    }
  }
}

static float
modulate(Osc *c, Osc *m, unsigned int i) {

/* A similar algorithm to the cannonical frequency modulation (FM) formula,
 * but operates on the phase of waves. The carrier wave has its phase increased
 * by the carrier pitch, then it has this phase further altered by adding
 * (or subtracting) the modulation pitch multiplied by its amplitude at the
 * discrete point in time i. This distorted carrier phase is then interpolated
 * against the carrier Osc's wavetable. */

  const float pitch = (c->Pitch * c->Wave->Polarity) + 
    (m->Pitch * m->Buffer[i]);
  int tableNo = wavetableIndex(*c->Complexity, pitch);

  if (c->Wave->Type == WAVE_TYPE_NOISE) {
    return readNoise(&c->Wave->Noise, pitch);
  } else {
    c->Phase = fmodf(c->Phase + pitch, (float)DEFAULT_WAVELEN);
    return interpolate(c->Wave->Table[tableNo], DEFAULT_WAVELEN, c->Phase);
  }
}

void
fillCarrierBuffer(Operator *c, Operator *m) {

/* Calculates the cycle of the modulating wave, then modulates the cycle of
 * the carrier wave against it. Sums its final values up in the carrier wave's
 * buffer. */

  unsigned int i = 0;

  fillModulatorBuffer(m);
  for (; i < DEFAULT_BUFSIZE ; i++) {
    c->Osc.Buffer[i] += modulate(&c->Osc, &m->Osc, i) *
      c->Osc.Amplitude * applyEnv(&c->Env) * c->Osc.KeyMod;
  }
}

