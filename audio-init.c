/* Initialization functions for the Audio type, which serves as the link
 * between the program and sndio, using the audio settings provided by the user
 * in command-line parameters. Most functions are of the void type, since
 * initialization errors are fatal anyway. */

#include <err.h>
#include <stdbool.h>
#include <sndio.h>
#include <stdlib.h>
#include <string.h>

#include "audio-init.h"

#include "audio-settings.h"
#include "buffers.h"
#include "defaults.h"
#include "errors.h"
#include "maximums.h"
#include "voice.h"

/* headers */
static void populateSettings(const AudioSettings *, struct sio_par *); 
static void openOutput(struct sio_hdl **);
static void suggestSettings(struct sio_hdl *, struct sio_par *);
static void setSetting(const unsigned int, const unsigned int,
        unsigned int *, const char *); 
static void setSettings(AudioSettings *, const struct sio_par *);
static void allocateBuffers(Buffer **, ByteBuffer **, const AudioSettings *);
static void startAudio(struct sio_hdl *);
static void KLUDGE_bitCheck(const unsigned int);

/* functions */
static void
populateSettings(const AudioSettings *aos, struct sio_par *sp) {

/* Initializes a sio_par struct and provides it with parameters from
 * AudioSettings. These settings will be suggested to the sound hardware, but 
 * won't necessarily be the final playback parameters. */

    sio_initpar(sp);
    sp->bits = aos->Bits;
    sp->appbufsz = aos->Bufsize;
    sp->pchan = aos->Chan;
    sp->rate = aos->Rate;
}

static void
openOutput(struct sio_hdl **o) {

/* Opens a handle to the soundcard. */

    *o = sio_open(SIO_DEVANY, SIO_PLAY, false);
    if (*o == NULL) {
        errx(ERROR_SIO, "Error opening handle to sndio");
    }
}

static void
suggestSettings(struct sio_hdl *o, struct sio_par *sp) {

/* Sends user-suggested settings to soundcard, gets soundcard's response. */

    if (sio_setpar(o, sp) == 0) {
        errx(ERROR_SIO, "Error setting sndio settings");
    }
    if (sio_getpar(o, sp) == 0) {
        errx(ERROR_SIO, "Error retrieving sndio settings");
    }
}

static void
setSetting(const unsigned int expected, const unsigned int reality, 
        unsigned int *field, const char *name) {

/* Assigns the actual hardware-limited parameter to "field", rather than the
 * one suggested by the user in the command-line flags. Often the parameters
 * are the same, but hardware limitations could make it different. */

    if (expected != reality){
        warnx("Hardware limitations changed %s from %u to %u", name, expected, 
                reality);
    }
    *field = reality;
}

static void
setSettings(AudioSettings *aos, const struct sio_par *sp) {

/* Runs setSetting() on essential playback parameters. */

    setSetting(aos->Bits, sp->bits, &aos->Bits, "bits");
    setSetting(aos->Bufsize, sp->appbufsz, &aos->Bufsize, "bufsize");
    setSetting(aos->Chan, sp->pchan, &aos->Chan, "chan");
    setSetting(aos->Rate, sp->rate, &aos->Rate, "rate");
    aos->BufsizeMain = aos->Bufsize * aos->Chan * (aos->Bits / 8);
}

static void
allocateBuffers(Buffer **b, ByteBuffer **bb, const AudioSettings *aos) {

/* Allocates space for both the mixing and main audio buffers. */

    *b = makeBuffer((size_t)aos->Bufsize);
    *bb = makeByteBuffer(aos);
}

static void
startAudio(struct sio_hdl *o) {

/* Primes the sndio device to start accepting audio output. */

    if (sio_start(o) == 0) {
        errx(ERROR_SIO, "Error initializing audio output.");
    }
}

void
makeAudio(Audio *a, const int argc, char **argv) {

/* Initializes an Audio struct. Opens sndio, applies parameters, allocates
 * buffers, starts sndio. */

    struct sio_par sp = {0};
    
    makeAudioSettings(&a->Settings, argc, argv);
    populateSettings(&a->Settings, &sp);
    openOutput(&a->Output);
    suggestSettings(a->Output, &sp);
    setSettings(&a->Settings, &sp);
    KLUDGE_bitCheck(a->Settings.Bits);
    allocateBuffers(&a->MixingBuffer, &a->MainBuffer, &a->Settings);
    makeVoices(&a->Voices, a->MixingBuffer, &a->Settings);
    a->Amplitude = 0.3f;
    startAudio(a->Output);
}

void
killAudio(Audio *a) {

/* Stops sndio. Frees all memory allocated by Audio type. */

   sio_close(a->Output); 
   killBuffer(a->MixingBuffer);
   killByteBuffer(a->MainBuffer);
   killVoices(&a->Voices);
}

static void
KLUDGE_bitCheck(const unsigned int bits) {

/* This function should not exist. It is a temporary guard against non-16bit
 * ints, which are not supported yet. */    

    if (bits != 16) {
        errx(ERROR_ARG, "Sorry, only 16 bit audio output is actually"
                " supported for now. Other resolutions will be added later.");
    }
}
