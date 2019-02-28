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
static float modulate(Osc *, Osc *, const unsigned int);

/* functions */
float
hzToPitch(const float hz, const unsigned int rate) {

/* Converts a frequency in Hz to a wavetable increment value. */

    return hz * ((float)DEFAULT_WAVELEN / (float)rate);
}

float
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

float
interpolate(const float phase, const Wave *w) {

/* Performs a linear interpolation on the Wave's table in terms of phase. This
 * is the primary means of pitching tones in the program. */

    float r, s1, s2;
    int i = 0;
    
    if (w->Type == WAVE_TYPE_NOISE) {
        /* Generate white noise instead of wavetable lookup. */
        return (float)arc4random() / (float)UINT_MAX;
    }
    i = (int)phase;
    r = fabsf(phase) - abs(i);
    s1 = w->Table[(unsigned int)i % DEFAULT_WAVELEN];
    s2 = w->Table[((unsigned int)i+1) % DEFAULT_WAVELEN];
    return ((1.0f - r) * s1) + (r * s2);
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

    for (; i < o->Buffer->Size; i++) {
        o->Phase += o->Pitch;
        o->Phase = fmodf(o->Phase, (float)DEFAULT_WAVELEN);
        o->Buffer->Values[i] = interpolate(o->Phase, o->Wave) * o->Amplitude *
            applyEnv(&m->Env) * o->Velocity.Sensitivity;
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

    c->Phase += c->Pitch;
    c->Phase += m->Pitch * m->Buffer->Values[i];
    c->Phase = fmodf(c->Phase, (float)DEFAULT_WAVELEN);
    return interpolate(c->Phase, c->Wave);
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
            c->Osc.Amplitude * applyEnv(&c->Env) * c->Osc.Velocity.Sensitivity;
    }
}
