/* The procedures in this file handle the lifespans of notes. They tell
 * new notes to play and kill off old ones. The Voice struct tracks an
 * individual note in a polyphonic performance. You can read more about it
 * in "voice.h". */

#include <err.h>
#include <stdlib.h>

#include "voice.h"

#include "audio-settings.h"
#include "buffers.h"
#include "defaults.h"
#include "envelope.h"
#include "errors.h"
#include "synthesis.h"
#include "wave.h"

/* headers */
static Voice * findFreeVoice(Voices *);
static void setVoicesSettings(Voices *, const AudioSettings *);
static void allocateVoices(Voices *);
static void makeOperator(Operator *, Buffer *, Envs *, Wave *);
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

void
voiceOn(Voices *vs, const unsigned int note) {

/* Assigns a pitch derived from pitch(note) to the first free Voice in
 * Voices.All. The sound output thread only plays Voices with an Env.Stage
 * value that != ENV_FINISHED, meaning that atomic switching of this
 * value allows for lock-free modification of a Voice. Even if a Voice is in
 * the middle of its buffer-filling operation, changing its values has a
 * negligible side-effect and is considered acceptable by this program. */

    Voice *v = NULL;
    float p = 0.0f;

    if (note >= DEFAULT_KEYS_NUM) {
        warnx("Note must be between 0 and %d", DEFAULT_KEYS_NUM - 1);
        return;
    }
    if (vs->Keys[note] != NULL) {
        /* The note is already playing, so reset the envelope and return. */
        v = vs->Keys[note];
        resetEnv(&v->Carrier.Env);
        resetEnv(&v->Modulator.Env);
        return;
    }
    v = findFreeVoice(vs);
    if (v->Key != DEFAULT_NO_KEY) {
        /* The previous note that held this voice remains as an artifact.
         * Its entry in Voices.Keys must be removed first. This is a deliberate
         * design choice that is explained further in voiceOff(). */
        vs->Keys[v->Key] = NULL;
    }
    v->Key = note;
    p = pitch(note, vs->Rate);
    v->Carrier.Osc.Pitch = p;
    v->Modulator.Osc.Pitch = vs->Ratio * v->Carrier.Osc.Pitch;
    v->Carrier.Osc.Phase = vs->Phase * v->Carrier.Osc.Pitch;
    v->Modulator.Osc.Phase = vs->Phase * v->Modulator.Osc.Pitch;
    vs->Keys[note] = v;
    resetEnv(&v->Modulator.Env);
    resetEnv(&v->Carrier.Env);
}

void
voiceOff(Voices *vs, const unsigned int note) {

/* Signals a Voice to stop by setting its Env.Stage values to ENV_RELEASE.
 * The pointer to the Voice remains in Voices.Keys until it is overwritten
 * by the next instance of voiceOn() that chances upon the Voice. This
 * process is slightly harder to follow, but it ensures that released envelopes
 * have a better shot at decaying naturally before their Voices are stolen. */

    Voice *v = NULL;

    if (vs->Keys[note] == NULL) {
        return;
    }
    v = vs->Keys[note];
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
setPitchRatio(Voices *vs, const float r) {

/* Sets the pitch relationship between carrier:modulator. Fc is 440hz, then
 * a ratio value of 2.0 would make Fm 880hz, etc. Sets the value of
 * Voices.Ratio, which applies to any new note made after the change, but
 * still has to run through a O(n) loop to update the ratio of any note already
 * playing. */

    unsigned int i = 0;

    vs->Ratio = r;

    for (; i < vs->N; i++) {
        vs->All[i].Modulator.Osc.Pitch = vs->All[i].Carrier.Osc.Pitch * r;
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
    vs->Ratio = 1.0f;
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
makeOperator(Operator *op, Buffer *b, Envs *es, Wave *w) {

/* Initializes an Operator type within a voice. */

    op->Osc.Buffer = b;
    op->Osc.Wave = w;
    makeEnv(es, &op->Env);
}

static void
makeVoice(Voices *vs, Voice *v, Buffer *cB, Buffer *mB) {

/* Initializes a Voice type within Voices.All. */

    Operators *os = NULL;

    v->Key = DEFAULT_NO_KEY;
    v->Ratio = &vs->Ratio;
    v->Carrier.Osc.Amplitude = vs->Amplitude;
    os = &vs->Carrier;
    makeOperator(&v->Carrier, cB, &os->Env, &os->Wave);
    os = &vs->Modulator;
    makeOperator(&v->Modulator, mB, &os->Env, &os->Wave);
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
}

void
killVoices(Voices *vs) {

/* Frees memory allocated during initialization of Voices struct. Does not free
 * Voice.CarrierBuffer, since this was allocated during the initialization of
 * the Audio struct. */

    killBuffer(vs->All[0].Modulator.Osc.Buffer);
    free(vs->All);
}
