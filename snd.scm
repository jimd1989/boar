(import (chicken file posix) (chicken foreign) (chicken io) 
        (chicken port) srfi-4 srfi-18 typed-records)

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
#define AUDIO_CHANS 2
#define AUDIO_BITS 16
#define STDIN_BUFFER_SIZE 4096
#define STDIN_IDX 0
#define SNDIO_OUT_IDX 1
#define FD_LIMIT 2

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

static struct pollfd POLLFDS[FD_LIMIT]         = {0};
static uint8_t STDIN_BUFFER[STDIN_BUFFER_SIZE] = {0};
static OutputBuffer OUTPUT_BUFFER              = {0};
static void (*SCHEME_AUDIO_OUT_CALLBACK)(int)  = NULL;

static void init_stdin(void);
static void audio_out_callback(void *, int);
static void fill_silence(OutputBuffer *);
static void init_audio_out(char *);
static void write_audio(OutputBuffer *);

static void init_stdin(void) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
}

static void audio_out_callback(void *arg, int delta) {
  SCHEME_AUDIO_OUT_CALLBACK(delta);
}

static void fill_silence(OutputBuffer *ob) {
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

static void init_audio_out(char *name) {
  int bytes           = 0;
  struct sio_hdl *sio = NULL;
  struct sio_par par  = {0};
  sio = sio_open(name, SIO_PLAY, true);
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return;
  }
  sio_initpar(&par);
  par.bits     = AUDIO_BITS;
  par.appbufsz = 1; /* soundcard will overwrite with min size */
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
  OUTPUT_BUFFER.writeSizeBytes = par.pchan * par.appbufsz * bytes;
  OUTPUT_BUFFER.dspData        = malloc(OUTPUT_BUFFER.dspSizeBytes);
  OUTPUT_BUFFER.writeData      = malloc(OUTPUT_BUFFER.writeSizeBytes);
  sio_onmove(sio, &audio_out_callback, (void *)&OUTPUT_BUFFER);
  sio_start(sio);
  fill_silence(&OUTPUT_BUFFER);
}

void fill_dsp(uint8_t *data, int sizeBytes) {
  OutputBuffer *ob = &OUTPUT_BUFFER;
  memcpy(&ob->dspData[ob->dspPos], data, sizeBytes);
  ob->dspPos = (ob->dspPos + sizeBytes) % ob->dspSizeBytes;
  //warnx("Δ %d → %d", sizeBytes, ob->dspPos);
}

static void write_audio(OutputBuffer *ob) {
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
}

void poll_io(void (*eval)(char *)) {
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
  }
}

void init(void (*schemeAudioOutCallback)(int)) {
  struct sio_hdl *sio = NULL;
  init_stdin();
  SCHEME_AUDIO_OUT_CALLBACK = schemeAudioOutCallback;
  init_audio_out("default");
}
<#

(define-external (stdin_eval (c-string x)) void
  (print (eval (with-input-from-string x read))))
(define init (foreign-safe-lambda void "init" (function void (int))))
(define fill-dsp (foreign-safe-lambda void "fill_dsp" u8vector int))
; (make-u8vector) can be anything! Scheme has full DSP control!
;(define-external (scheme_audio_out_callback (int x)) void
;  (fill-dsp (make-u8vector 2084 0) x))
(define poll-io (foreign-safe-lambda void "poll_io" (function void (c-string))))
;(init (location scheme_audio_out_callback))

(: io-loop (-> noreturn))
(define (io-loop)
  ; needs error handling
  (poll-io (location stdin_eval))
  (io-loop))

(define-record dsp-buffer
  (bytes : u8vector)
  (data : any)
  (f : (u8vector any fixnum -> noreturn)))

(: adjust-buffer (u8vector fixnum --> u8vector))
(define (adjust-buffer buffer size)
  (if (< (u8vector-length buffer) size)
    (begin (release-number-vector buffer)
           (make-u8vector size 0 #t #f))
    buffer))

(: ignore-buffer (u8vector any fixnum -> noreturn))
(define (ignore-buffer bytes data bytes-to-fill) bytes)

(: make-audio-out-condition-variable (--> (struct condition-variable)))
(define (make-audio-out-condition-variable)
  (let* ((cvar (make-condition-variable))
         (u8 (make-u8vector 128 0 #t #f))
         (buffer (make-dsp-buffer u8 '() ignore-buffer)))
    (condition-variable-specific-set! cvar buffer)
    cvar))

(define-syntax make-audio-out-hdl
  (syntax-rules ()
    ((_ c-func-name cvar)
     (define-external (c-func-name (int bytes-to-fill)) void
       (let* ((buf (condition-variable-specific cvar))
              (nu8 (adjust-buffer (dsp-buffer-bytes buf) bytes-to-fill)))
         (dsp-buffer-bytes-set! buf nu8)
         (fill-dsp nu8 bytes-to-fill)
         (condition-variable-broadcast! cvar))))))

(define AUDIO-OUT-COND (make-audio-out-condition-variable))
(make-audio-out-hdl sndio_0 AUDIO-OUT-COND)
(init (location sndio_0))
(io-loop)
