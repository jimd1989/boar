(import (chicken condition) (chicken file posix) (chicken foreign) (chicken io) 
        (chicken port) (chicken random) srfi-4 srfi-18 typed-records)

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

static void init_stdin(void) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
}

static void audio_out_callback(void *arg, int deltaFrames) {
  OutputBuffer *ob = &OUTPUT_BUFFER;
  int chans        = ob->parameters.pchan;
  int byteDepth    = ob->parameters.bits >> 3;
  int deltaBytes   = deltaFrames * chans * byteDepth;
  SCHEME_AUDIO_OUT_CALLBACK(deltaBytes);
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

// introduce an index here
static void init_audio_out(void (*schemeAudioOutCallback)(int),
                           char *name, int rate, int outCh, int inCh, 
                           int bits, bool readWrite) {
  int bytes           = 0;
  struct sio_hdl *sio = NULL;
  struct sio_par par  = {0};
  if (readWrite) {
    sio = sio_open(name, SIO_REC | SIO_PLAY, true);
  } else {
    sio = sio_open(name, SIO_PLAY, true);
  }
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return;
  }
  sio_initpar(&par);
  par.bits     = bits;
  par.appbufsz = 1; /* soundcard will overwrite with min size */
  par.rate     = rate;
  par.pchan    = outCh;
  par.rchan    = inCh;
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
  SCHEME_AUDIO_OUT_CALLBACK    = schemeAudioOutCallback;
  sio_onmove(sio, &audio_out_callback, (void *)&OUTPUT_BUFFER);
  sio_start(sio);
  warnx("%dch %dHz %d frame buffer", par.pchan, par.rate, par.round);
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
  ob->writePos = (ob->writePos + bytesWritten) % ob->dspSizeBytes;
}

void poll_io(void (*eval)(char *)) {
  int i            = 0;
  int mask         = 0;
  int bytesRead    = 0;
  OutputBuffer *ob = &OUTPUT_BUFFER;
  /* Seemingly has to run each time */
  sio_pollfd(ob->sio, &POLLFDS[SNDIO_OUT_IDX], POLLIN | POLLOUT);
  poll(POLLFDS, FD_LIMIT, -1);
  if (POLLFDS[STDIN_IDX].revents & POLLIN) {
    bytesRead = read(STDIN_FILENO, STDIN_BUFFER, STDIN_BUFFER_SIZE - 1);
    if (bytesRead > 1) { 
      eval(STDIN_BUFFER);
    }
  }
  /* MIO HDL loop eventually */
  mask = sio_revents(ob->sio, &POLLFDS[SNDIO_OUT_IDX]);
  if (mask & POLLIN) {
    sio_read(ob->sio, ob->writeData, ob->writeSizeBytes);
  }
  if (mask & POLLOUT) {
    write_audio(ob);
  }
}
<#

(define-external (stdin_eval (c-string x)) void
  (condition-case (print (eval (with-input-from-string x read)))
   (e (exn) (print (get-condition-property e 'exn 'message)))
   (exn () (print 'unknown-input-error))))

(define init-stdin (foreign-safe-lambda void "init_stdin"))

(define fill-dsp! (foreign-safe-lambda void "fill_dsp" u8vector int))

(define poll-io (foreign-safe-lambda void "poll_io" (function void (c-string))))

(define init-audio-out (foreign-safe-lambda void "init_audio_out"
  (function void (int)) c-string int int int int bool))

(: io-loop (-> noreturn))
(define (io-loop)
  (poll-io (location stdin_eval))
  (io-loop))

(: DEFAULT-AUDIO-SETTINGS (list-of (list-of any)))
(define DEFAULT-AUDIO-SETTINGS
  '((name "default")
    (rate 48000)
    (out-ch 2)
    (in-ch 2)
    (bits 16)
    (read-write? #f)))

(: get-setting (any (list-of (list-of any)) --> any))
(define (get-setting x xs)
  (let ((setting (assoc x xs)))
    (if setting (cadr setting) (cadr (assoc x DEFAULT-AUDIO-SETTINGS)))))

(: start-audio (pointer (list-of (list-of any)) -> void))
(define (start-audio callback xs)
  (let ((name (get-setting 'name xs))
        (rate (get-setting 'rate xs))
        (out-ch (get-setting 'out-ch xs))
        (in-ch (get-setting 'in-ch xs))
        (bits (get-setting 'bits xs))
        (read-write? (if (get-setting 'read-write? xs) 1 0)))
    (init-audio-out callback name rate out-ch in-ch bits read-write?)))

(define-record dsp-buffer
  (bytes : u8vector)
  (data : any)
  (f : (u8vector any fixnum -> noreturn))
  (mutex : (struct mutex)))

(: adjust-buffer (u8vector fixnum -> u8vector))
(define (adjust-buffer buffer size)
  (if (< (u8vector-length buffer) size)
    (begin (release-number-vector buffer)
           (make-u8vector size 0 #t #f))
    buffer))

(: ignore-buffer (u8vector any fixnum -> noreturn))
(define (ignore-buffer bytes data bytes-to-fill) (void))

(: fill-noise! (u8vector any fixnum -> noreturn))
(define (fill-noise! u8 x n) (random-bytes (u8vector->blob/shared u8)) (void))

(: make-audio-out-condition-variable (-> (struct condition-variable)))
(define (make-audio-out-condition-variable)
  (let* ((cvar (make-condition-variable))
         (u8 (make-u8vector 128 0 #t #f))
         (buffer (make-dsp-buffer u8 '() ignore-buffer (make-mutex))))
    (condition-variable-specific-set! cvar buffer)
    cvar))

(: set-audio-out-data! ((struct condition-variable) any -> noreturn))
(define (set-audio-out-data! cvar x)
  (let ((mutex (dsp-buffer-mutex (condition-variable-specific cvar))))
    (mutex-lock! mutex)
    (dsp-buffer-data-set! (condition-variable-specific cvar) x)
    (mutex-unlock! mutex)))

(: set-audio-out-f! ((struct condition-variable)
                     (u8vector any fixnum -> noreturn) -> noreturn))
(define (set-audio-out-f! cvar f)
  (let* ((mutex (dsp-buffer-mutex (condition-variable-specific cvar)))
         (new-f (lambda (u8 x n)
                  (mutex-lock! mutex) (f u8 x n) (mutex-unlock! mutex))))
    (dsp-buffer-f-set! (condition-variable-specific cvar) new-f)))

(define-syntax make-audio-out-hdl
  (syntax-rules ()
    ((_ c-func-name cvar)
     (define-external (c-func-name (int bytes-to-fill)) void
       (let* ((buf (condition-variable-specific cvar))
              (nu8 (adjust-buffer (dsp-buffer-bytes buf) bytes-to-fill)))
         (dsp-buffer-bytes-set! buf nu8)
         ((dsp-buffer-f buf) nu8 (dsp-buffer-data buf) bytes-to-fill)
         (fill-dsp! nu8 bytes-to-fill)
         (condition-variable-broadcast! cvar))))))

; runtime
; TODO: 
; - multiple devices
;   - start with array of one
;   - add index arg on line 74
; - MIDI
; too many setup steps
(: SNDIO-0-COND (struct condition-variable))
(define SNDIO-0-COND (make-audio-out-condition-variable))
(make-audio-out-hdl sndio_0 SNDIO-0-COND)
(define SNDIO-0-HDL (location sndio_0))
(start-audio SNDIO-0-HDL '((read-write? #t)))
(init-stdin)
(io-loop)
