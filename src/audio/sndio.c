#include <err.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "settings.h"
#include "sndio.h"

typedef struct sio_par SioPar;

void sio(Sio *s) {
  /* On buffers (all sizes are in frames, not bytes):
   * sndio holds a buffer governed by soundcard settings that is only known at 
   * runtime. It may not perfectly align with the internal DSP buffer described 
   * in buffer.h. The sndio initialization must negotiate this discrepancy.
   * bufSize    = intended audio output bufsize (overridden by sndio settings)
   * p.bufsz    = maximum sndio bufsize
   * p.appbufsz = minimum sndio bufsize needed be filled to avoid glitches
   * Final returned and allocated audio output buffer should be a perfect
   * multiple of p.appbufsz, which is what p.round is used for. This size must
   * not be larger than p.bufsz. p.bufsz itself seems to be a multiple of 
   * p.appbufsz, making it an acceptable fallback value. */
  unsigned int bufSize = AUDIO_SAMPLE_RATE / 200;
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
  p.xrun     = SIO_IGNORE; /* Keep set for development; remove in prod */
  if (!(sio_setpar(s->port, &p))) { errx(1, "Error setting sndio parameters"); }
  if (!(sio_getpar(s->port, &p))) { errx(1, "Error getting sndio parameters"); }
  bufSize = (bufSize + p.round) - 1;
  bufSize -= bufSize % p.round;
  if (bufSize > p.bufsz) { bufSize = p.bufsz; }
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
