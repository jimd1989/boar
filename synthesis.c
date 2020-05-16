/* These functions contain the actual mathematics that create sound. */

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "synthesis.h"

#include "defaults.h"
#include "envelope.h"
#include "wave.h"

/* headers */
static void fillModulatorBuffer(Operator *);
static float hzToPitch(const float, const unsigned int);
static float pitch(const unsigned int, const unsigned int);
static float modulate(Osc *, Osc *, const unsigned int);

/* functions */
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

static void
fillModulatorBuffer(Operator *m) {

/* The Buffer pointed to by Operator.Osc.Buffer is safe to overwrite on every
 * invocation of this function, as modulation is calculated in a single thread.
 * This procedure assigns an interpolated float sample to every index of the
 * Osc buffer, derived from Osc.Pitch. This buffer is later used to modulate
 * the carrier signal. */
    
    unsigned int i = 0;
    Osc *o = &m->Osc;
    int tableNo = 1 + (int)ilogb(DEFAULT_OCTAVE_SCALING * o->Pitch);

    tableNo = abs(tableNo);
    tableNo = (tableNo >= DEFAULT_OCTAVES) ? DEFAULT_OCTAVES - 1 : tableNo;
    for (; i < o->Buffer->Size; i++) {
        o->Phase += o->Pitch * o->Wave->Polarity;
        o->Phase = fmodf(o->Phase, (float)DEFAULT_WAVELEN);
        o->Buffer->Values[i] =
            newInterpolate(o->Wave, o->Wave->NewTable[tableNo], o->Phase) * 
            o->Amplitude * applyEnv(&m->Env) * o->KeyMod;
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
                        (m->Pitch * m->Buffer->Values[i]);
    int tableNo = 1 + (int)ilogb(DEFAULT_OCTAVE_SCALING * pitch);

    tableNo = abs(tableNo);
    tableNo = (tableNo >= DEFAULT_OCTAVES) ? DEFAULT_OCTAVES - 1 : tableNo;
    c->Phase = fmodf(c->Phase + pitch, (float)DEFAULT_WAVELEN);
    return newInterpolate(c->Wave, c->Wave->NewTable[tableNo], c->Phase);
}

void
fillCarrierBuffer(Operator *c, Operator *m) {

/* Calculates the cycle of the modulating wave, then modulates the cycle of
 * the carrier wave against it. Sums its final values up in the carrier wave's
 * buffer. */

    unsigned int i = 0;
    
    fillModulatorBuffer(m);
    for (; i < c->Osc.Buffer->Size ; i++) {
        c->Osc.Buffer->Values[i] += modulate(&c->Osc, &m->Osc, i) *
            c->Osc.Amplitude * applyEnv(&c->Env) * c->Osc.KeyMod;
    }
}
