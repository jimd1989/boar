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
#define TEXT_FD_LIMIT 1
#define AUDIO_FD_LIMIT 4
#define FD_LIMIT (TEXT_FD_LIMIT + AUDIO_FD_LIMIT)

typedef struct AudioBuffer {
  int               fdIdx;
  int               dspSizeBytes;
  int               writeSizeBytes;
  int               dspPos;
  int               writePos;
  struct sio_hdl  * sio;
  struct sio_par    parameters;
  uint8_t         * dspData;
  uint8_t         * writeData;
  void              (*schemeCallback)(int);
} AudioBuffer;

static struct pollfd POLLFDS[FD_LIMIT]           = {0};
static uint8_t STDIN_BUFFER[STDIN_BUFFER_SIZE]   = {0};
static AudioBuffer AUDIO_BUFFERS[AUDIO_FD_LIMIT] = {0};

void stdin_init(void) {
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
}

void audio_out_callback(void *arg, int deltaFrames) {
  AudioBuffer *ob = (AudioBuffer *)arg;
  int chans       = ob->parameters.pchan;
  int byteDepth   = ob->parameters.bits >> 3;
  int deltaBytes  = deltaFrames * chans * byteDepth;
  ob->schemeCallback(deltaBytes);
}

void fill_silence(AudioBuffer *ob) {
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

struct sio_hdl * audio_init(int idx, void (*schemeCallback)(int), char *name, 
                            int rate, int outCh, int inCh, int bits, 
                            bool readWrite) {
  int bytes           = 0;
  struct sio_hdl *sio = NULL;
  struct sio_par par  = {0};
  AudioBuffer *ab     = NULL;
  if (idx < 0 || (idx - 1) > AUDIO_FD_LIMIT) {
    warnx("%d audio devices available, requested #%d", AUDIO_FD_LIMIT, idx + 1);
  }
  ab = &AUDIO_BUFFERS[idx];
  if (readWrite) {
    sio = sio_open(name, SIO_REC | SIO_PLAY, true);
  } else {
    sio = sio_open(name, SIO_PLAY, true);
  }
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return NULL;
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
  ab->fdIdx          = TEXT_FD_LIMIT + idx;
  ab->sio            = sio;
  ab->parameters     = par;
  ab->dspSizeBytes   = par.pchan * par.bufsz * bytes;
  ab->writeSizeBytes = par.pchan * par.appbufsz * bytes;
  ab->dspData        = malloc(ab->dspSizeBytes);
  ab->writeData      = malloc(ab->writeSizeBytes);
  ab->schemeCallback = schemeCallback;
  sio_onmove(sio, &audio_out_callback, (void *)ab);
  sio_start(sio);
  warnx("%dch %dHz %d frame buffer", par.pchan, par.rate, par.round);
  fill_silence(ab);
  return ab->sio;
}

void audio_close(struct sio_hdl *sio) {
  sio_stop(sio);
  sio_close(sio);
  sio = NULL;
}

void fill_dsp(int idx, uint8_t *data, int sizeBytes) {
  AudioBuffer *ob = &AUDIO_BUFFERS[idx];
  memcpy(&ob->dspData[ob->dspPos], data, sizeBytes);
  ob->dspPos = (ob->dspPos + sizeBytes) % ob->dspSizeBytes;
  //warnx("Δ %d → %d", sizeBytes, ob->dspPos);
}

static void write_audio(AudioBuffer *ob) {
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
  int i           = 0;
  int mask        = 0;
  int bytesRead   = 0;
  AudioBuffer *ob = NULL;
  for (i = 0 ; i < AUDIO_FD_LIMIT ; i++) {
    ob = &AUDIO_BUFFERS[i];
    if (ob->sio != NULL) {
      sio_pollfd(ob->sio, &POLLFDS[ob->fdIdx], POLLIN | POLLOUT);
    }
  }
  poll(POLLFDS, FD_LIMIT, -1);
  if (POLLFDS[STDIN_IDX].revents & POLLIN) {
    bytesRead = read(STDIN_FILENO, STDIN_BUFFER, STDIN_BUFFER_SIZE - 1);
    if (bytesRead > 1) { 
      eval(STDIN_BUFFER);
    }
  }
  /* MIO HDL loop eventually */
  for (i = 0 ; i < AUDIO_FD_LIMIT ; i++) {
    ob = &AUDIO_BUFFERS[i];
    if (ob->sio != NULL) {
      mask = sio_revents(ob->sio, &POLLFDS[ob->fdIdx]);
      if (mask & POLLIN) {
        sio_read(ob->sio, ob->writeData, ob->writeSizeBytes);
      }
      if (mask & POLLOUT) {
        write_audio(ob);
      }
    }
  }
}
<#

(define-record dsp-buffer
  (bytes : u8vector)
  (data : any)
  (f : (u8vector any fixnum -> noreturn))
  (mutex : (struct mutex)))

(define-record audio-handle
  (condition-variable : (struct condition-variable))
  (callback : pointer)
  (sio : (or boolean pointer))
  (idx : fixnum))

(define-external (stdin_eval (c-string x)) void
  (condition-case (print (eval (with-input-from-string x read)))
   (e (exn) (print (get-condition-property e 'exn 'message)))
   (exn () (print 'unknown-input-error))))

(define stdin-init (foreign-safe-lambda void "stdin_init"))

(define fill-dsp! (foreign-safe-lambda void "fill_dsp" int u8vector int))

(define poll-io (foreign-safe-lambda void "poll_io" (function void (c-string))))

(define audio-init (foreign-safe-lambda c-pointer "audio_init"
  int (function void (int)) c-string int int int int bool))

(define audio-close (foreign-safe-lambda void "audio_close" c-pointer))

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

(: audio-start!
  ((struct audio-handle) #!optional (list-of (list-of any)) -> noreturn))
(define (audio-start! handle #!optional (xs'()))
  (let* ((callback (audio-handle-callback handle))
         (name (get-setting 'name xs))
         (rate (get-setting 'rate xs))
         (out-ch (get-setting 'out-ch xs))
         (in-ch (get-setting 'in-ch xs))
         (bits (get-setting 'bits xs))
         (read-write? (if (get-setting 'read-write? xs) 1 0))
         (cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar)))
         (sio (audio-handle-sio handle))
         (idx (audio-handle-idx handle)))
    (if sio
      (print "audio is already playing")
      (begin (mutex-lock! mutex)
             (audio-handle-sio-set! handle
              (audio-init idx callback name rate out-ch in-ch bits read-write?))
             (mutex-unlock! mutex)))))

(: audio-stop! ((struct audio-handle) -> noreturn))
(define (audio-stop! handle)
  (let* ((sio (audio-handle-sio handle))
         (cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar))))
    (if sio
      (begin (mutex-lock! mutex) (audio-close sio) (mutex-unlock! mutex)
             (audio-handle-sio-set! handle #f))
      (print "audio is not playing"))))

(: adjust-buffer (u8vector fixnum -> u8vector))
(define (adjust-buffer buffer size)
  (if (< (u8vector-length buffer) size)
    (begin (release-number-vector buffer)
           (make-u8vector size 0 #t #f))
    buffer))

(: ignore-buffer! (u8vector any fixnum -> noreturn))
(define (ignore-buffer! bytes data bytes-to-fill) (void))

(: fill-noise! (u8vector any fixnum -> noreturn))
(define (fill-noise! u8 x n) (random-bytes (u8vector->blob/shared u8)) (void))

(: make-audio-out-condition-variable (-> (struct condition-variable)))
(define (make-audio-out-condition-variable)
  (let* ((cvar (make-condition-variable))
         (u8 (make-u8vector 128 0 #t #f))
         (mutex (make-mutex))
         (silence (lambda (u8 x n) (mutex-lock! mutex)
                                   (ignore-buffer! u8 x n)
                                   (mutex-unlock! mutex)))
         (buffer (make-dsp-buffer u8 '() silence mutex)))
    (condition-variable-specific-set! cvar buffer)
    cvar))

(: audio-data-set! ((struct audio-handle) any -> noreturn))
(define (audio-data-set! handle x)
  (let* ((cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar))))
    (mutex-lock! mutex)
    (dsp-buffer-data-set! (condition-variable-specific cvar) x)
    (mutex-unlock! mutex)))

(: audio-f-set! ((struct audio-handle)
                     (u8vector any fixnum -> noreturn) -> noreturn))
(define (audio-f-set! handle f)
  (let* ((cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar)))
         (new-f (lambda (u8 x n)
                  (mutex-lock! mutex) (f u8 x n) (mutex-unlock! mutex))))
    (mutex-lock! mutex)
    (dsp-buffer-f-set! (condition-variable-specific cvar) new-f)
    (mutex-unlock! mutex)))

(define-syntax make-audio-out-hdl
  (syntax-rules ()
    ((_ c-func-name cvar idx)
     (define-external (c-func-name (int bytes-to-fill)) void
       (let* ((buf (condition-variable-specific cvar))
              (nu8 (adjust-buffer (dsp-buffer-bytes buf) bytes-to-fill)))
         (dsp-buffer-bytes-set! buf nu8)
         ((dsp-buffer-f buf) nu8 (dsp-buffer-data buf) bytes-to-fill)
         (fill-dsp! idx nu8 bytes-to-fill)
         (condition-variable-broadcast! cvar))))))

; audio/MIDI handles hard limited at compile time because C callback pointers
; are not available in interpreted mode
(: SIO-0-COND (struct condition-variable))
(define SIO-0-COND (make-audio-out-condition-variable))

(: SIO-1-COND (struct condition-variable))
(define SIO-1-COND (make-audio-out-condition-variable))

(: SIO-2-COND (struct condition-variable))
(define SIO-2-COND (make-audio-out-condition-variable))

(: SIO-3-COND (struct condition-variable))
(define SIO-3-COND (make-audio-out-condition-variable))

(make-audio-out-hdl sio_0 SIO-0-COND 0)
(make-audio-out-hdl sio_1 SIO-1-COND 1)
(make-audio-out-hdl sio_2 SIO-2-COND 2)
(make-audio-out-hdl sio_3 SIO-3-COND 3)

(: SIO-0 (struct audio-handle))
(define SIO-0 (make-audio-handle SIO-0-COND (location sio_0) #f 0))

(: SIO-1 (struct audio-handle))
(define SIO-1 (make-audio-handle SIO-1-COND (location sio_1) #f 1))

(: SIO-2 (struct audio-handle))
(define SIO-2 (make-audio-handle SIO-2-COND (location sio_2) #f 2))

(: SIO-3 (struct audio-handle))
(define SIO-3 (make-audio-handle SIO-3-COND (location sio_3) #f 3))

; runtime
(print "boar: available audio handles " '(SIO-0 SIO-1 SIO-2 SIO-3))
(print "boar: default audio settings " DEFAULT-AUDIO-SETTINGS)
(print "boar: please run (audio-start! AUDIO-HANDLE SETTINGS-OVERRIDES)")
(stdin-init)
(io-loop)
