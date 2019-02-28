#pragma once

#include "buffers.h"
#include "envelope.h"
#include "velocity.h"
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
    Velocity       Velocity;
} Osc;

typedef struct Operator {

/* Couples an Osc with an Env for organization's sake. Contains pointers to
 * values in a parent Operators struct, which govern the fixed rate and pitch
 * ratio values of the Operator. */

    float       * FixedRate;
    float       * Ratio;
    Osc           Osc;
    Env           Env;
} Operator;

typedef struct Operators {

/* A master Operator that contains the wave and envelope settings that
 * individual child Operators point to. No oscillator information is contained
 * here, as that is decided on a Voice by Voice basis. */
    
    float       FixedRate;    
    float       Ratio;
    Wave        Wave;
    Wave        VelocityCurve;
    Envs        Env;

} Operators;

/* headers */
float hzToPitch(const float, const unsigned int);
float pitch(const unsigned int, const unsigned int);
void setPitch(Operator *, const unsigned int, const unsigned int);
float interpolate(const float, const Wave *);
void fillCarrierBuffer(Operator *, Operator *m);
