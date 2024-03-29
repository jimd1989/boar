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

#include "amplitude.h"
#include "audio-settings.h"
#include "buffers.h"
#include "constants/defaults.h"
#include "constants/errors.h"
#include "constants/maximums.h"
#include "voice.h"

static void populateSettings(const AudioSettings *, struct sio_par *); 
static void openOutput(struct sio_hdl **);
static void suggestSettings(struct sio_hdl *, struct sio_par *);
static int roundBuffer(AudioSettings *, const struct sio_par *);
static void setSetting(const unsigned int, const unsigned int,
    unsigned int *, const char *); 
static void setSettings(AudioSettings *, const struct sio_par *);
static void checkSettings(struct sio_par *);
static void startAudio(struct sio_hdl *);

static void
populateSettings(const AudioSettings *aos, struct sio_par *sp) {

/* Initializes a sio_par struct and provides it with parameters from
 * AudioSettings. These settings will be suggested to the sound hardware, but 
 * won't necessarily be the final playback parameters. */

  sio_initpar(sp);
  sp->bits = aos->Bits;
  sp->appbufsz = aos->BufSizeFrames * aos->BufBlocks;
  sp->rate = aos->Rate;
  sp->pchan = DEFAULT_CHAN;
}

static void
openOutput(struct sio_hdl **o) {

/* Opens a handle to the soundcard. */

  *o = sio_open(SIO_DEVANY, SIO_PLAY, true);
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

static int
roundBuffer(AudioSettings *aos, const struct sio_par *sp) {

/* Round the buffer size according to hardware suggestions. */

  int frames = aos->BufSizeFrames;
  frames = frames + sp->round - 1;
  frames -= frames % sp->round;
  return frames;
}

static void
setSettings(AudioSettings *aos, const struct sio_par *sp) {

/* Runs setSetting() on essential playback parameters. */

  setSetting(aos->Bits, sp->bits, &aos->Bits, "bits");
  setSetting(aos->BufSizeFrames, roundBuffer(aos, sp), &aos->BufSizeFrames, 
      "block size");
  setSetting(aos->Rate, sp->rate, &aos->Rate, "rate");
}

static void
startAudio(struct sio_hdl *o) {

/* Primes the sndio device to start accepting audio output. */

  if (sio_start(o) == 0) {
    errx(ERROR_SIO, "Error initializing audio output.");
  }
}

static void
checkSettings(struct sio_par *sp) {

/* Kill program if certain sndio settings aren't satisfied. */

  if (sp->bits != DEFAULT_BITS) {
    errx(ERROR_SIO, "Expected %d bit output.", DEFAULT_BITS);
  }
  if (sp->pchan != DEFAULT_CHAN) {
    errx(ERROR_SIO, "Expected %d channels.", DEFAULT_CHAN);
  }
}

void
makeAudio(Audio *a, const int argc, char **argv) {

/* Initializes an Audio struct. Opens sndio, applies parameters, allocates
 * buffers, starts sndio. */

  struct sio_par sp = {0};

  makeAudioSettings(&a->Settings, argc, argv);
  /* First suggestion */
  populateSettings(&a->Settings, &sp);
  openOutput(&a->Output);
  suggestSettings(a->Output, &sp);
  setSettings(&a->Settings, &sp);
  checkSettings(&sp);
  /* Second suggestion: for accurate `appbufsz` with `round` */
  populateSettings(&a->Settings, &sp);
  openOutput(&a->Output);
  suggestSettings(a->Output, &sp);
  setSettings(&a->Settings, &sp);
  checkSettings(&sp);
  /* Should this be BufSizeFrames * BufBlocks too? */
  a->Buffer = makeBuffer(a->Settings.BufSizeFrames);
  makeVoices(&a->Voices, a->Buffer.Mix, &a->Settings);
  a->Amplitude = makeAmplitude();
  startAudio(a->Output);
}

void
killAudio(Audio *a) {

/* Stops sndio. Frees all memory allocated by Audio type. */

  sio_close(a->Output); 
  killBuffer(&a->Buffer);
  killVoices(&a->Voices);
}
