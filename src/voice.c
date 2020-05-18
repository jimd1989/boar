/* The procedures in this file handle the lifespans of notes. They tell
 * new notes to play and kill off old ones. The Voice struct tracks an
 * individual note in a polyphonic performance. You can read more about it
 * in "voice.h". */

#include <err.h>
#include <stdint.h>
#include <stdlib.h>

#include "voice.h"

#include "audio-settings.h"
#include "buffers.h"
#include "defaults.h"
#include "envelope.h"
#include "errors.h"
#include "key.h"
#include "synthesis.h"
#include "wave.h"

/* headers */
static Voice * findFreeVoice(Voices *);
static void resetVoice(const Voices *, Voice *, const uint16_t);
static void setVoicesSettings(Voices *, const AudioSettings *);
static void allocateVoices(Voices *);
static void makeOperator(Operators *, Operator *, Buffer *);
static void makeVoice(Voices *, Voice *, Buffer *, Buffer *);
static void makeOperators(Operators *, const unsigned int);

/* functions */
static Voice *
findFreeVoice(Voices *vs) {

/* Iterates through Voices.All, returning a pointer to the first Voice with
 * a Voice.Carrier.Env.Stage value of ENV_FINISHED. If no Voices are free, then
 * the active Voice at the initial value of Voices.Current + 1 is stolen. This
 * is theoretically simple but perhaps less than ideal during realtime
 * performance. The algorithm is subject to change. */

    unsigned int i = 0;
    Voice *v = NULL;

    for (; i < vs->N ; i++) {
        v = &vs->All[vs->Current];
        if (v->Carrier.Env.Stage == ENV_FINISHED) {
            return v;
        }
        vs->Current = (vs->Current + 1) % vs->N;
    }
    vs->Current = (vs->Current + 1) % vs->N;
    return &vs->All[vs->Current];
}

static void
resetVoice(const Voices *vs, Voice *v, const uint16_t note) {

/* Retriggers keyboard settings and envelopes in a Voice. Notes that are
 * turned off then on again without engaging another voice also trigger this
 * branch, which could be a cause of subtle errors in future features. Be
 * advised. */    
    
    applyKey(&vs->Keyboard, &v->Carrier, &v->Modulator, note);
    resetEnv(&v->Carrier.Env);
    resetEnv(&v->Modulator.Env);
}

void
voiceOn(Voices *vs, const uint16_t n) {

/* Assigns a pitch derived from pitch(note) to the first free Voice in
 * Voices.All. The sound output thread only plays Voices with an Env.Stage
 * value that != ENV_FINISHED, meaning that atomic switching of this
 * value allows for lock-free modification of a Voice. Even if a Voice is in
 * the middle of its buffer-filling operation, changing its values has a
 * negligible side-effect and is considered acceptable by this program. */

    const unsigned int note = getNote(n);
    Voice *v = NULL;

    if (note >= DEFAULT_KEYS_NUM) {
        warnx("Note must be between 0 and %u", DEFAULT_KEYS_NUM - 1);
        return;
    }
    if (vs->Active[note] != NULL) {
        resetVoice(vs, vs->Active[note], n);
        return;
    }
    v = findFreeVoice(vs);
    if (v->Note != DEFAULT_NO_KEY) {
        /* The previous note that held this voice remains as an artifact.
         * Its entry in Voices.Active must be removed first. This is a 
         * deliberate design choice that is explained further in voiceOff(). */
        vs->Active[v->Note] = NULL;
    }
    v->Note = note;
    vs->Active[note] = v;
    resetVoice(vs, v, n);
}

void
voiceOff(Voices *vs, const uint16_t n) {

/* Signals a Voice to stop by setting its Env.Stage values to ENV_RELEASE.
 * The pointer to the Voice remains in Voices.Active until it is overwritten
 * by the next instance of voiceOn() that chances upon the Voice. This
 * process is slightly harder to follow, but it ensures that released envelopes
 * have a better shot at decaying naturally before their Voices are stolen. */

    const unsigned int note = getNote(n);
    Voice *v = NULL;

    if (vs->Active[note] == NULL) {
        return;
    }
    v = vs->Active[note];
    v->Modulator.Env.Stage = ENV_RELEASE;
    v->Carrier.Env.Stage = ENV_RELEASE;
}

void
pollVoice(Voice *v) {

/* Generates a cycle of sample data for a voice, if it is active. */

    if (v->Carrier.Env.Stage != ENV_FINISHED) {
        fillCarrierBuffer(&v->Carrier, &v->Modulator);
    }
}

void
setPitchRatio(Voices *vs, const bool isCarrier, const float r) {

/* Sets the pitch ratio for a carrier or modulator. In additon to changing
 * the master ratio that all children Operators derive their values from, the
 * function must also loop through each voice and reset its pitch manually to
 * change any notes that are currently playing. */

    unsigned int i = 0;
    Voice *v = NULL;

    if (isCarrier) {
        vs->Carrier.Ratio = r;
    } else {
        vs->Modulator.Ratio = r;
    }
    for (; i < vs->N; i++) {
        v = &vs->All[i];
        applyKey(&vs->Keyboard, &v->Carrier, &v->Modulator, v->Note);
    }
}

void
setFixedRate(Voices *vs, const bool isCarrier, const float r) {

/* Changes the Operator.FixedRate setting in a manner similar to that of
 * setPitchRatio(), operating in O(n) time upon all children. */

    unsigned int i = 0;
    Voice *v = NULL;

    if (isCarrier) {
        vs->Carrier.FixedRate = r;
    } else {
        vs->Modulator.FixedRate = r;
    }
    for (; i < vs->N; i++) {
        v = &vs->All[i];
        applyKey(&vs->Keyboard, &v->Carrier, &v->Modulator, v->Note);
    }
}

void
setModulation(Voices *vs, const float m) {

/* Sets modulation index on all voices. Since this variable makes use of
 * Osc.Amplitude, which is a local variable for carriers, it must be updated in
 *  a O(n) loop rather than simple change to a pointer value. Audible
 *  distortion should not be too much of a problem. */

    unsigned int i = 0;

    for (; i < vs->N; i++) {
        vs->All[i].Modulator.Osc.Amplitude = m;
    }
}

static void
setVoicesSettings(Voices *vs, const AudioSettings *aos) {

/* Populates numerical fields of Voices struct with proper playback values. */

    vs->N = aos->Polyphony;
    vs->Rate = aos->Rate;
    vs->Carrier.Ratio = 1.0f;
    vs->Modulator.Ratio = 1.0f;
    vs->Phase = 1;
    vs->Amplitude = 1.0f / (float)vs->N;
}

static void
allocateVoices(Voices *vs) {

/* Allocates memory for all Voice structs in Voices. */

    vs->All = calloc(sizeof(*vs->All), vs->N);
    if (vs->All == NULL) {
        errx(ERROR_ALLOC, "Error initializing voices");
    }
}

static void
makeOperator(Operators *os, Operator *op, Buffer *b) {

/* Initializes an Operator type within a voice. */
    
    op->FixedRate = &os->FixedRate;
    op->Ratio = &os->Ratio;
    op->Osc.Buffer = b;
    op->Osc.Wave = &os->Wave;
    makeEnv(&os->Env, &op->Env);
}

static void
makeVoice(Voices *vs, Voice *v, Buffer *cB, Buffer *mB) {

/* Initializes a Voice type within Voices.All. */

    v->Note = DEFAULT_NO_KEY;
    v->Carrier.Osc.Amplitude = vs->Amplitude;
    makeOperator(&vs->Carrier, &v->Carrier, cB);
    makeOperator(&vs->Modulator, &v->Modulator, mB);
}

static void
makeOperators(Operators *os, const unsigned int rate) {

/* Initializes a Voices.Operators type. */    

    makeEnvs(&os->Env, rate);
    selectWave(&os->Wave, WAVE_TYPE_SINE);
}

void
makeVoices(Voices *vs, Buffer *cBuffer, const AudioSettings *aos) {

/* Initializes a Voices type. Errors are fatal. */
    
    unsigned int i = 0;
    Buffer *mBuffer = makeBuffer(cBuffer->Size);
    Voice *v = NULL;

    setVoicesSettings(vs, aos);
    allocateVoices(vs);
    makeOperators(&vs->Carrier, aos->Rate);
    makeOperators(&vs->Modulator, aos->Rate);
    for (; i < vs->N ; i++) {
        v = &vs->All[i];
        makeVoice(vs, v, cBuffer, mBuffer);
    }
    makeKeyboard(&vs->Keyboard, vs->Rate, &vs->Phase);
}

void
killVoices(Voices *vs) {

/* Frees memory allocated during initialization of Voices struct. Does not free
 * Voice.CarrierBuffer, since this was allocated during the initialization of
 * the Audio struct. */

    killBuffer(vs->All[0].Modulator.Osc.Buffer);
    free(vs->All);
}