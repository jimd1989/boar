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
#include <unistd.h>

#define AUDIO_BLOCK_SIZE_FRAMES 64
#define AUDIO_CHANS 2
#define AUDIO_BITS 16
#define AUDIO_BLOCK_SIZE_BYTES \
  (AUDIO_BLOCK_SIZE_FRAMES * AUDIO_CHANS * (AUDIO_BITS / 8))
#define STDIN_BUFFER_SIZE 4096
#define STDIN_IDX 0
#define SNDIO_OUT_IDX 1
#define FD_LIMIT 2

static struct pollfd POLLFDS[FD_LIMIT]                     = {0};
static uint8_t STDIN_BUFFER[STDIN_BUFFER_SIZE]             = {0};
static uint8_t AUDIO_OUTPUT_BUFFER[AUDIO_BLOCK_SIZE_BYTES] = {0};

void init_stdin(void) {
  int i     = 0;
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
}

struct sio_hdl * init_audio_output(char *name) {
  struct sio_hdl *sio = NULL;
  struct sio_par par  = {0};
  sio = sio_open(name, SIO_PLAY, true);
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return NULL;
  }
  sio_initpar(&par);
  /* edit pars here */
  sio_setpar(sio, &par);
  /* getpar negotiation here */
  sio_start(sio);
  return sio;
}

void poll_io(void (*eval)(char *), struct sio_hdl *sio) {
  int i         = 0;
  int mask      = 0;
  int bytesRead = 0;
  /* Seemingly has to run each time */
  sio_pollfd(sio, &POLLFDS[SNDIO_OUT_IDX], POLLOUT);
  poll(POLLFDS, FD_LIMIT, -1);
  if (POLLFDS[STDIN_IDX].revents & POLLIN) {
    bytesRead = read(STDIN_FILENO, STDIN_BUFFER, STDIN_BUFFER_SIZE - 1);
    if (bytesRead > 1) { 
      eval(STDIN_BUFFER);
    }
  }
  /* MIO HDL loop eventually */
  mask = sio_revents(sio, &POLLFDS[SNDIO_OUT_IDX]);
  if (mask & POLLOUT) {
    sio_write(sio, AUDIO_OUTPUT_BUFFER, AUDIO_BLOCK_SIZE_BYTES);
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
