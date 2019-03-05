# pragma once

#include "defaults.h"
#include "synthesis.h"
#include "velocity.h"
#include "wave.h"

typedef struct KeyboardLayer {

/* A KeyboardLayer is holds all tuning, velocity, and key follow information
 * for a carrier or modulator. */

    float       Tunings[DEFAULT_KEYS_NUM];
    Wave        VelocityCurve;
    Wave        KeyFollowCurve;
} KeyboardLayer;

typedef struct KeyboardSettings {

/* KeyboardSettings holds essential information passed from higher level
 * types like Voices and Audio, without requiring direct pointers to them. */

    unsigned int          Rate;
    uint64_t            * Phase;
} KeyboardSettings;

typedef enum TuningLayer {

/* Determines which layer of the keyboard should be modified by tuning
 * commands. */

    TUNING_CARRIER = 0,
    TUNING_MODULATOR = 1
} TuningLayer;

typedef struct Keyboard {

/* The Keyboard is a singleton type that contains all tuning, velocity, and
 * key follow information for the entire program. The velocity and key follow
 * settings of carriers and modulators are exposed to the user through the
 * t/T and k/K commands of the REPL, but the tuning command u/U only points to
 * one KeyboardLayer at any given time. Keyboard.TuningTarget is set through
 * the extra parameters (E) interface in order to change which layer
 * Keyboard.TuningNote is selected from with U. This value is ultimately
 * altered by the u command. */

    float               * TuningNote;
    float               * TuningLayer;
    KeyboardSettings      Settings;
    KeyboardLayer         Carrier;
    KeyboardLayer         Modulator;

} Keyboard;

/* headers */
unsigned int getNote(const uint16_t);
void selectTuningLayer(Keyboard *, const TuningLayer);
void selectTuningKey(Keyboard *, const unsigned int);
void tuneKey(Keyboard *, const float);
void applyKey(const Keyboard *, Operator *, Operator *, const uint16_t); 
void makeKeyboard(Keyboard *, const unsigned int, uint64_t *);
