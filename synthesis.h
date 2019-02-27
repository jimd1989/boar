#pragma once

#include "buffers.h"
#include "envelope.h"
#include "wave.h"

/* types */
typedef struct Osc {

/* The primitive sound generating type. For every sound sample value generated,
 * Osc.Phase is incremented by Osc.Pitch, which serves as the index of
 * WAVE_SINE where the sample resides. Osc.Amplitude governs the depth of
 * modulation or the volume of the note, depending upon whether Osc is a
 * modulator or carrier. At step i of every buffer-filling cycle,
 * Osc.Phase * Osc.Amplitude is written to Osc.Buffer.Values[i]. */
    
    float          Amplitude;
    float          Phase;
    float          Pitch;
    Buffer       * Buffer;
    Wave         * Wave;
} Osc;

typedef struct Operator {

/* Couples an Osc with an Env for organization's sake. */

    Osc Osc;
    Env Env;
} Operator;

typedef struct Operators {

/* A master Operator that contains the wave and envelope settings that
 * individual child Operators point to. No oscillator information is contained
 * here, as that is decided on a Voice by Voice basis. */

    Wave        Wave;
    Envs        Env;

} Operators;

/* headers */
float pitch(const unsigned int, const unsigned int);
float interpolate(const float, const Wave *);
void fillCarrierBuffer(Operator *, Operator *m);
