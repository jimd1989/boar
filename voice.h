#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "audio-settings.h"
#include "defaults.h"
#include "key.h"
#include "synthesis.h"
#include "wave.h"

/* types */
typedef struct Voice {

/* A voice plays back an individual note in a polyphonic performance. To do
 * this, it manages a carrier:modulator pair of Operators whose respective 
 * Pitch value are governed by Voice.Ratio. During every cycle of audio output,
 * the values in Voice.Carrier's buffer are modulated against the values in
 * Voice.Modulator's buffer. */

    unsigned int  Note;
    Operator      Carrier;
    Operator      Modulator;
} Voice;

typedef struct Voices {

/* Voices is a master struct with an array of all available Voices, plus
 * additional playback information, most of which is self-explanatory.
 * Voices.Amplitude is 1.0 / Voices.N, so that simultaenous playback of all
 * Voices does not clip. Voices.Phase is incremented by the size of
 * Voice.Carrier.Osc.Buffer.Size every time data is written to the soundcard.
 * It serves as a rough measure of global phase, so that new notes do not start 
 * with a phase of zero. Voices.Keys contains pointers to active Voices in
 * terms of MIDI notes, allowing for easy access when turning a note on/off.
 * Voices.Current cycles through Voices.All looking for free voices to assign
 * new notes to. */

    unsigned int  Current;
    unsigned int  Rate;
    float         Amplitude;
    size_t        N;
    uint64_t      Phase;
    Operators     Carrier;
    Operators     Modulator;
    Voice       * All;
    Voice       * Active[DEFAULT_KEYS_NUM];
    Keyboard      Keyboard;
} Voices;

/* headers */
void voiceOn(Voices *, const uint16_t);
void voiceOff(Voices *, const uint16_t);
void pollVoice(Voice *);
void setPitchRatio(Voices *, const bool, const float);
void setFixedRate(Voices *, const bool, const float);
void setModulation(Voices *, const float);
void makeVoices(Voices *, Buffer *, const AudioSettings *);
void killVoices(Voices *);
