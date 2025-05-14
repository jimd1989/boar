#include <err.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "settings.h"
#include "sndio.h"

typedef struct sio_par SioPar;

void sio(Sio *s) {
  int bufSize = AUDIO_SAMPLE_RATE / 200;
  SioPar p = {0};
  s->port = sio_open(SIO_DEVANY, SIO_PLAY, true);
  if (s == NULL) { errx(1, "Error opening sndio device %s", SIO_DEVANY); }
  sio_initpar(&p);
  p.pchan    = AUDIO_CHANNELS;
  p.rate     = AUDIO_SAMPLE_RATE;
  p.bits     = AUDIO_SAMPLE_SIZE_BYTES * 8;
  p.sig      = 1;
  p.le       = SIO_LE_NATIVE;
  p.appbufsz = bufSize;
  if (!(sio_setpar(s->port, &p))) { errx(1, "Error setting sndio parameters"); }
  if (!(sio_getpar(s->port, &p))) { errx(1, "Error getting sndio parameters"); }
  bufSize = (bufSize + p.round) - 1;
  bufSize -= bufSize % p.round;
  s->bufSizeFrames = bufSize;
  s->bufSizeBytes  = s->bufSizeFrames * 4;
  s->nfds = sio_nfds(s->port);
}

int writeSio(Sio *s, uint8_t *data) {
  int bytesWritten  = sio_write(s->port, data, s->bufSizeBytes);
  int framesWritten = bytesWritten / 4;
  return framesWritten;
}

void startSio(Sio *s) {
  if (!sio_start(s->port)) { errx(1, "Error starting sndio"); }
}

void stopSio(Sio *s) {
  if (!sio_stop(s->port)) { errx(1, "Error stoppingf sndio"); }
}
