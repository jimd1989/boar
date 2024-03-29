#pragma once

#include <stdbool.h>

#include "envelope.h"
#include "wave.h"

typedef struct Osc {

/* The primitive sound generating type. For every sound sample value generated,
 * Osc.Phase is incremented by Osc.Pitch, which serves as the index of Osc.Wave,
 * where the sample resides. Osc.Amplitude governs the modulation depth or 
 * volume of the note, depending upon whether Osc is a modulator or carrier. 
 * Osc.KeyMod is the aggregate values of the velocity and key follow settings of 
 * the struck key that engaged the Osc. Osc.Complexity adjusts the harmonic
 * richness of the signal offsetting +/- which band-limited wavetable to read.
 * At step i of every buffer-filling cycle, 
 * Osc.Phase * Osc.Amplitude * Osc.KeyMod is written to Osc.Buffer[i]. */

  float   KeyMod;
  float   Amplitude;
  float   Phase;
  float   Pitch;
  int   * Complexity;
  float * Buffer;
  Wave  * Wave;
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

  int   Complexity;
  float FixedRate;    
  float Ratio;
  Wave  Wave;
  Envs  Env;

} Operators;

void setPitch(Operator *, const unsigned int, const unsigned int);
void fillCarrierBuffer(Operator *, Operator *m);
