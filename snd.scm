(import (chicken file posix) (chicken foreign) (chicken io) 
        (chicken port) srfi-4 srfi-18)

#>
#include <err.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define AUDIO_RATE 48000
#define AUDIO_RESOLUTION 375
#define AUDIO_BLOCK_SIZE_FRAMES (AUDIO_RATE / AUDIO_RESOLUTION)
#define AUDIO_CHANS 2
#define AUDIO_BITS 16
#define AUDIO_BLOCK_SIZE_BYTES \
  (AUDIO_BLOCK_SIZE_FRAMES * AUDIO_CHANS * (AUDIO_BITS / 8))
#define STDIN_BUFFER_SIZE 4096
#define STDIN_IDX 0
#define SNDIO_OUT_IDX 1
#define FD_LIMIT 2

/* Current understanding:
  1. Write rendered DSP at static rate: AUDIO_BLOCK_SIZE_FRAMES.
     This will likely be a partial, imperfect factor of the sndio
     `round` setting, but chunking writes into tiny blocks allows
     more polling of user input.
  2. Always handle stdin/MIDI events before audio. If audio output
     receives POLLOUT, it will remain open until multiple partial
     writes have taken place. Can also check user input on each of
     those cycles.
  3. Render DSP inside sio_onmove callback. Take delta argument and
     write that many bytes of new audio output to the buffer, which
     will eventually be processed by step 1. The delta will either be
     zero or `round`, which means a contiguous slice of the buffer can
     be passed to a Scheme callback for DSP without requiring modulo.
This OutputBuffer includes everything needed to maintain these separate
audio writing and DSP concerns. The C code should always think in terms
of bytes. Let Scheme handle higher level encodings. */
typedef struct OutputBuffer {
  int               dspSizeBytes;
  int               writeSizeBytes;
  int               dspPos;
  int               writePos;
  struct sio_hdl  * sio;
  struct sio_par    parameters;
  uint8_t         * dspData;
  uint8_t         * writeData;
} OutputBuffer;

static struct pollfd POLLFDS[FD_LIMIT]                     = {0};
static uint8_t STDIN_BUFFER[STDIN_BUFFER_SIZE]             = {0};
static uint8_t AUDIO_OUTPUT_BUFFER[AUDIO_BLOCK_SIZE_BYTES] = {0};
static OutputBuffer OUTPUT_BUFFER = {0};

void init_stdin(void) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
}

void sio_callback(void *arg, int delta) {
  OutputBuffer *ob = (OutputBuffer *)arg;
  ob->dspPos = (ob->dspPos + delta) % ob->dspSizeBytes;
  warnx("dsp Δ %d → %d", delta, ob->dspPos);
}

void fillSilence(OutputBuffer *ob) {
  /* Meant for pre-filling the buffer. For whatever reason it does not
     seem to trigger callbacks, so init ob->writePos at 0 for maximum
     distance from ob->dspPos. */
  int i            = 0;
  int n            = ob->dspSizeBytes / ob->writeSizeBytes;
  int bytesWritten = 0;
  struct pollfd pfd[1] = {0};
  for (i = 0 ; i <= n ; i++) {
    sio_pollfd(ob->sio, pfd, POLLOUT);
    poll(pfd, 1, -1);
    if (sio_revents(ob->sio, pfd) & POLLOUT) {
      memset(ob->writeData, 0, ob->writeSizeBytes);
      bytesWritten += sio_write(ob->sio, ob->writeData, ob->writeSizeBytes);
    }
  }
  ob->writePos += (ob->writePos + bytesWritten) % ob->dspSizeBytes;
}

struct sio_hdl * init_audio_output(char *name) {
  int bytes           = 0;
  struct sio_hdl *sio = NULL;
  struct sio_par par  = {0};
  sio = sio_open(name, SIO_PLAY, true);
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return NULL;
  }
  sio_initpar(&par);
  par.bits     = AUDIO_BITS;
  par.appbufsz = AUDIO_BLOCK_SIZE_FRAMES;
  par.rate     = AUDIO_RATE;
  par.pchan    = AUDIO_CHANS;
  par.le       = 1;
  par.sig      = 1;
  sio_setpar(sio, &par);
  sio_getpar(sio, &par);
  bytes = par.bits >> 3;
  OUTPUT_BUFFER.sio            = sio;
  OUTPUT_BUFFER.parameters     = par;
  OUTPUT_BUFFER.dspSizeBytes   = par.pchan * par.bufsz * bytes;
  OUTPUT_BUFFER.writeSizeBytes = par.pchan * AUDIO_BLOCK_SIZE_FRAMES * bytes;
  OUTPUT_BUFFER.dspData        = malloc(OUTPUT_BUFFER.dspSizeBytes);
  OUTPUT_BUFFER.writeData      = malloc(OUTPUT_BUFFER.writeSizeBytes);
  sio_onmove(sio, &sio_callback, (void *)&OUTPUT_BUFFER);
  sio_start(sio);
  fillSilence(&OUTPUT_BUFFER);
  return sio;
}

void write_audio(OutputBuffer *ob) {
  int i            = 0;
  int write_ix     = ob->writePos;
  int bytesWritten = 0;
  for (i = 0 ; i < ob->writeSizeBytes ; i++) {
    ob->writeData[i] = ob->dspData[write_ix];
    write_ix = (write_ix + 1) % ob->dspSizeBytes;
  }
  bytesWritten = sio_write(ob->sio, ob->writeData, ob->writeSizeBytes);
  /* It's possible not to write a full buffer's worth, especially if the
     writeSize is not a factor of dspSize. So the true writePos depends
     on bytesWritten. */
  ob->writePos = (ob->writePos + bytesWritten) % ob->dspSizeBytes;
  warnx("out Δ %d → %d", bytesWritten, ob->writePos);
  if (ob->writePos == ob->dspPos) { errx(1, "BUFFER PHASE PROBLEM"); }
}

void poll_io(void (*eval)(char *), struct sio_hdl *sio) {
  int i            = 0;
  int mask         = 0;
  int bytesRead    = 0;
  OutputBuffer *ob = &OUTPUT_BUFFER;
  /* Seemingly has to run each time */
  sio_pollfd(ob->sio, &POLLFDS[SNDIO_OUT_IDX], POLLOUT);
  poll(POLLFDS, FD_LIMIT, -1);
  if (POLLFDS[STDIN_IDX].revents & POLLIN) {
    bytesRead = read(STDIN_FILENO, STDIN_BUFFER, STDIN_BUFFER_SIZE - 1);
    if (bytesRead > 1) { 
      eval(STDIN_BUFFER);
    }
  }
  /* MIO HDL loop eventually */
  mask = sio_revents(ob->sio, &POLLFDS[SNDIO_OUT_IDX]);
  if (mask & POLLOUT) {
    write_audio(ob);
    //sio_write(ob->sio, AUDIO_OUTPUT_BUFFER, AUDIO_BLOCK_SIZE_BYTES);
  }
}

struct sio_hdl *init(void) {
  struct sio_hdl *sio = NULL;
  init_stdin();
  sio = init_audio_output("default");
  return sio;
}
<#

(define-external (stdin_eval (c-string x)) void
  (print (eval (with-input-from-string x read))))
(define init (foreign-safe-lambda c-pointer "init"))
(define poll-io
  (foreign-safe-lambda void "poll_io" (function void (c-string)) c-pointer))
(define sio-onmove
  (foreign-safe-lambda void "sio_onmove"
    c-pointer (function void (c-pointer int)) c-pointer))
(define-external (sndio_callback (c-pointer arg) (int delta)) void
  (print delta))
(print 'starting)
(define SNDIO-HDL (init))
(define (io-loop hdl)
  ; needs error handling
  (poll-io (location stdin_eval) hdl)
  (io-loop hdl))
;(sio-onmove sndio-hdl sndio_callback #f)
(io-loop SNDIO-HDL)
(print 'ending)
