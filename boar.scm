(import (chicken condition) (chicken file posix) (chicken foreign) (chicken io) 
        (chicken memory) (chicken port) (chicken random) srfi-4 srfi-18 
        typed-records)

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
#define STDIN_BUFFER_BLOCK_SIZE 4096
#define MIDI_BUFFER_SIZE 1024
#define STDIN_IDX 0
#define SNDIO_OUT_IDX 1
#define TEXT_FD_LIMIT 1
#define AUDIO_FD_LIMIT 4
#define MIDI_FD_LIMIT 4
#define FD_LIMIT (TEXT_FD_LIMIT + AUDIO_FD_LIMIT + MIDI_FD_LIMIT)

typedef struct StdinBuffer {
  int     sizeBlocks;
  int     readPos;
  char    block[STDIN_BUFFER_BLOCK_SIZE];
  char  * data;
} StdinBuffer;

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

typedef struct MidiBuffer {
  int               fdIdx;
  struct mio_hdl  * mio;
  void              (*schemeCallback)(int);
  uint8_t         * midiData;
} MidiBuffer;

static struct pollfd POLLFDS[FD_LIMIT]           = {0};
static StdinBuffer STDIN_BUFFER                  = {0};
static AudioBuffer AUDIO_BUFFERS[AUDIO_FD_LIMIT] = {0};
static MidiBuffer MIDI_BUFFERS[MIDI_FD_LIMIT]    = {0};

void stdin_init(void) {
  int blockSize = 2;
  int flags     = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
  STDIN_BUFFER.sizeBlocks   = blockSize;
  STDIN_BUFFER.data         = malloc(blockSize * STDIN_BUFFER_BLOCK_SIZE);
}

void stdin_read(void (*eval)(char *)) {
  int bytesRead    = 0;
  int currentBlock = 0;
  StdinBuffer *sb  = &STDIN_BUFFER;
  char *newBuffer  = NULL;
  sb->readPos      = 0;
  do {
    currentBlock++;
    if (currentBlock > sb->sizeBlocks) {
      newBuffer       = malloc(sb->sizeBlocks * 2 * STDIN_BUFFER_BLOCK_SIZE);
      memcpy(newBuffer, sb->data, sb->sizeBlocks * STDIN_BUFFER_BLOCK_SIZE);
      free(sb->data);
      sb->data        = newBuffer;
      sb->sizeBlocks *= 2;
    }
    bytesRead    = read(STDIN_FILENO, &sb->data[sb->readPos], 
                        STDIN_BUFFER_BLOCK_SIZE - 1);
    if (bytesRead > 0) { 
      sb->readPos += bytesRead;
    }
  } while (bytesRead > 0);
  sb->data[sb->readPos] = '\0';
  if (sb->readPos > 1) {
    eval(STDIN_BUFFER.data);
  }
}

struct mio_hdl * midi_init(int idx, uint8_t *buffer, void(*schemeCallback)(int), 
                               char *name, bool in, bool out) {
  int mode            = (in ? MIO_IN : 0) | (out ? MIO_OUT : 0);
  struct mio_hdl *mio = NULL;
  MidiBuffer *mb      = NULL;
  if (idx < 0 || idx >= MIDI_FD_LIMIT) {
    warnx("%d midi devices available, requested #%d", MIDI_FD_LIMIT, idx + 1);
  }
  mb  = &MIDI_BUFFERS[idx];
  mio = mio_open(name, mode, true);
  if (mio == NULL) {
    warnx("could not open midi device %s", name);
  }
  mb->mio            = mio;
  mb->fdIdx          = TEXT_FD_LIMIT + AUDIO_FD_LIMIT + idx;
  mb->schemeCallback = schemeCallback;
  mb->midiData       = buffer;
  mio_pollfd(mb->mio, &POLLFDS[mb->fdIdx], POLLIN | POLLOUT);
  return mb->mio;
}

int midi_write(int idx, uint8_t *buffer, int bytes) {
  int mask = 0;
  int bytesWritten = 0;
  int bytesToWrite = 0;
  struct pollfd pfd[1] = {0};
  MidiBuffer *mb = &MIDI_BUFFERS[idx];
  while (bytesWritten < bytes) {
    bytesToWrite = bytes > MIDI_BUFFER_SIZE ? MIDI_BUFFER_SIZE : bytes;
    mio_pollfd(mb->mio, pfd, POLLOUT);
    poll(pfd, 1, -1);
    mask = mio_revents(mb->mio, pfd);
    if (mask & POLLOUT) {
      bytesWritten += mio_write(mb->mio, buffer, bytesToWrite); 
    }
  }
  return bytesWritten;
}

void audio_callback(void *arg, int deltaFrames) {
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
  if (idx < 0 || idx >= AUDIO_FD_LIMIT) {
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
  sio_onmove(sio, &audio_callback, (void *)ab);
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

static void audio_write(AudioBuffer *ob) {
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
  MidiBuffer *mb  = NULL;
  AudioBuffer *ob = NULL;
  /* seemingly must run every time for audio, but not for MIDI. */
  for (i = 0 ; i < AUDIO_FD_LIMIT ; i++) {
    ob = &AUDIO_BUFFERS[i];
    if (ob->sio != NULL) {
      sio_pollfd(ob->sio, &POLLFDS[ob->fdIdx], POLLIN | POLLOUT);
    }
  }
  poll(POLLFDS, FD_LIMIT, -1);
  if (POLLFDS[STDIN_IDX].revents & POLLIN) {
    stdin_read(eval);
  }
  for (i = 0 ; i < MIDI_FD_LIMIT ; i++) {
    mb = &MIDI_BUFFERS[i];
    if (mb->mio != NULL) {
      mask = mio_revents(mb->mio, &POLLFDS[mb->fdIdx]);
      if (mask & POLLIN) {
        bytesRead = mio_read(mb->mio, mb->midiData, MIDI_BUFFER_SIZE);
        mb->schemeCallback(bytesRead);
      }
      if (mask & POLLOUT) {
        /* This probably is not right. Leaving branch here, but writes are
           likely a separate, direct call. */
        warnx("the MIDI POLLOUT was actually triggered");
        mio_write(mb->mio, mb->midiData, MIDI_BUFFER_SIZE);
      }
    }
  }
  for (i = 0 ; i < AUDIO_FD_LIMIT ; i++) {
    ob = &AUDIO_BUFFERS[i];
    if (ob->sio != NULL) {
      mask = sio_revents(ob->sio, &POLLFDS[ob->fdIdx]);
      if (mask & POLLIN) {
        sio_read(ob->sio, ob->writeData, ob->writeSizeBytes);
      }
      if (mask & POLLOUT) {
        audio_write(ob);
      }
    }
  }
}
<#

(define-syntax λ (syntax-rules () ((_ . a) (lambda . a))))

(define-record midi-buffer
  (bytes : u8vector)
  (data : any)
  (f : (u8vector any fixnum -> noreturn))
  (mutex : (struct mutex)))

(define-record dsp-buffer
  (bytes : u8vector)
  (data : any)
  (f : (u8vector any fixnum -> noreturn))
  (mutex : (struct mutex)))

(define-record midi-handle
  (condition-variable : (struct condition-variable))
  (callback : pointer)
  (mio : (or boolean pointer))
  (idx : fixnum))

(define-record audio-handle
  (condition-variable : (struct condition-variable))
  (callback : pointer)
  (sio : (or boolean pointer))
  (idx : fixnum))

(define-external (stdin_eval (c-string x)) void
  (condition-case
    (for-each 
      (lambda (q) (print (eval q))) (with-input-from-string x read-list))
   (e (exn) (print (get-condition-property e 'exn 'message)
                   (get-condition-property e 'exn 'arguments)))
   (exn () (print 'unknown-input-error))))

(define stdin-init (foreign-safe-lambda void "stdin_init"))

(define fill-dsp! (foreign-safe-lambda void "fill_dsp" int u8vector int))

(define poll-io (foreign-safe-lambda void "poll_io" (function void (c-string))))

(define midi-init (foreign-safe-lambda c-pointer "midi_init"
  int u8vector (function void (int)) c-string bool bool))

(define midi-write (foreign-safe-lambda int "midi_write"
  int u8vector int))

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
    (read-write? #f)
    (midi-in? #t)
    (midi-out? #t)))

(: get-setting (any (list-of (list-of any)) --> any))
(define (get-setting x xs)
  (let ((setting (assoc x xs)))
    (if setting (cadr setting) (cadr (assoc x DEFAULT-AUDIO-SETTINGS)))))

(define-syntax with-lock
  (syntax-rules ()
    ((_ m f ...) (dynamic-wind (λ () (mutex-lock! m))
                               (λ () f ...)
                               (λ () (mutex-unlock! m))))))
(: midi-start!
  ((struct midi-handle) #!optional (list-of (list-of any)) -> noreturn))
(define (midi-start! handle #!optional (xs '()))
  (let* ((callback (midi-handle-callback handle))
         (name (get-setting 'name xs))
         (midi-in? (if (get-setting 'midi-in? xs) 1 0))
         (midi-out? (if (get-setting 'midi-out? xs) 1 0))
         (cvar (midi-handle-condition-variable handle))
         (buf (condition-variable-specific cvar))
         (u8 (midi-buffer-bytes buf))
         (mutex (midi-buffer-mutex buf))
         (mio (midi-handle-mio handle))
         (idx (midi-handle-idx handle)))
    (if mio
      (print "midi is already started")
      (with-lock mutex
        (midi-handle-mio-set! handle
               (midi-init idx u8 callback name midi-in? midi-out?))))))

(: make-midi-condition-variable (-> (struct condition-variable)))
(define (make-midi-condition-variable)
  (let* ((cvar (make-condition-variable))
         (mutex (make-mutex))
         (printer (λ (u8 x n) (with-lock mutex (print (subu8vector u8 0 n)))))
         (u8 (make-u8vector 1024 0 #t #f))
         (buffer (make-midi-buffer u8 '() printer mutex)))
    (condition-variable-specific-set! cvar buffer)
    cvar))

(: midi-data-set! ((struct midi-handle) any -> noreturn))
(define (midi-data-set! handle x)
  (let* ((cvar (midi-handle-condition-variable handle))
         (mutex (midi-buffer-mutex (condition-variable-specific cvar))))
    (with-lock mutex
      (midi-buffer-data-set! (condition-variable-specific cvar) x))))

(: midi-f-set! ((struct midi-handle) any -> noreturn))
(define (midi-f-set! handle f)
  (let* ((cvar (midi-handle-condition-variable handle))
         (mutex (midi-buffer-mutex (condition-variable-specific cvar)))
         (new-f (λ (u8 x n) (with-lock mutex (f u8 x n)))))
    (with-lock mutex
      (midi-buffer-f-set! (condition-variable-specific cvar) new-f))))

(: midi-write!
   ((struct midi-handle) (u8vector any -> fixnum u8vector) -> fixnum))
(define (midi-write! handle f)
  (let* ((mio (midi-handle-mio handle))
         (cvar (midi-handle-condition-variable handle))
         (buf (condition-variable-specific cvar))
         (mutex (midi-buffer-mutex buf))
         (bytes (midi-buffer-bytes buf))
         (data (midi-buffer-data buf))
         (idx (midi-handle-idx handle)))
    (if mio
      (with-lock mutex 
        (receive (bytes-to-write . extra) (f bytes data)
          (if (not (null? extra))
            (let ((nu8 (car extra)))
              (if (not (eq? nu8 bytes)) (midi-buffer-bytes-set! buf nu8))))
          (midi-write idx (midi-buffer-bytes buf) bytes-to-write)))
      (begin (print "run (midi-start!) on this handle first") 0))))

(: audio-start!
  ((struct audio-handle) #!optional (list-of (list-of any)) -> noreturn))
(define (audio-start! handle #!optional (xs '()))
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
      (with-lock mutex
        (audio-handle-sio-set! handle
          (audio-init idx callback name rate out-ch in-ch bits read-write?))))))

(: audio-stop! ((struct audio-handle) -> noreturn))
(define (audio-stop! handle)
  (let* ((sio (audio-handle-sio handle))
         (cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar))))
    (if sio
      (begin (with-lock mutex (audio-close sio))
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
         (silence (λ (u8 x n) (with-lock mutex (ignore-buffer! u8 x n))))
         (buffer (make-dsp-buffer u8 '() silence mutex)))
    (condition-variable-specific-set! cvar buffer)
    cvar))

(: audio-data-set! ((struct audio-handle) any -> noreturn))
(define (audio-data-set! handle x)
  (let* ((cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar))))
    (with-lock mutex
      (dsp-buffer-data-set! (condition-variable-specific cvar) x))))

(: audio-f-set! ((struct audio-handle)
                     (u8vector any fixnum -> noreturn) -> noreturn))
(define (audio-f-set! handle f)
  (let* ((cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar)))
         (new-f (λ (u8 x n) (with-lock mutex (f u8 x n)))))
    (with-lock mutex
      (dsp-buffer-f-set! (condition-variable-specific cvar) new-f))))

; audio/MIDI handles hard limited at compile time because C callback pointers
; are not available in interpreted mode
(define-syntax make-midi-hdl
  (syntax-rules ()
    ((_ c-func-name cvar idx)
     (define-external (c-func-name (int bytes-to-fill)) void
       (let* ((buf (condition-variable-specific cvar))
              (u8 (midi-buffer-bytes buf)))
         ((midi-buffer-f buf) u8 (midi-buffer-data buf) bytes-to-fill)
         (condition-variable-broadcast! cvar))))))

(define-syntax make-audio-hdl
  (syntax-rules ()
    ((_ c-func-name cvar idx)
     (define-external (c-func-name (int bytes-to-fill)) void
       (let* ((buf (condition-variable-specific cvar))
              (nu8 (adjust-buffer (dsp-buffer-bytes buf) bytes-to-fill)))
         ; mutex-lock?
         (dsp-buffer-bytes-set! buf nu8)
         ((dsp-buffer-f buf) nu8 (dsp-buffer-data buf) bytes-to-fill)
         (fill-dsp! idx nu8 bytes-to-fill)
         (condition-variable-broadcast! cvar))))))

(: MIO-0-COND (struct condition-variable))
(define MIO-0-COND (make-midi-condition-variable))

(: MIO-1-COND (struct condition-variable))
(define MIO-1-COND (make-midi-condition-variable))

(: MIO-2-COND (struct condition-variable))
(define MIO-2-COND (make-midi-condition-variable))

(: MIO-3-COND (struct condition-variable))
(define MIO-3-COND (make-midi-condition-variable))

(: SIO-0-COND (struct condition-variable))
(define SIO-0-COND (make-audio-out-condition-variable))

(: SIO-1-COND (struct condition-variable))
(define SIO-1-COND (make-audio-out-condition-variable))

(: SIO-2-COND (struct condition-variable))
(define SIO-2-COND (make-audio-out-condition-variable))

(: SIO-3-COND (struct condition-variable))
(define SIO-3-COND (make-audio-out-condition-variable))

(make-midi-hdl mio_0 MIO-0-COND 0)
(make-midi-hdl mio_1 MIO-0-COND 1)
(make-midi-hdl mio_2 MIO-0-COND 2)
(make-midi-hdl mio_3 MIO-0-COND 3)

(make-audio-hdl sio_0 SIO-0-COND 0)
(make-audio-hdl sio_1 SIO-1-COND 1)
(make-audio-hdl sio_2 SIO-2-COND 2)
(make-audio-hdl sio_3 SIO-3-COND 3)

(: MIO-0 (struct midi-handle))
(define MIO-0 (make-midi-handle MIO-0-COND (location mio_0) #f 0))

(: MIO-1 (struct midi-handle))
(define MIO-1 (make-midi-handle MIO-1-COND (location mio_1) #f 1))

(: MIO-2 (struct midi-handle))
(define MIO-2 (make-midi-handle MIO-2-COND (location mio_2) #f 2))

(: MIO-3 (struct midi-handle))
(define MIO-3 (make-midi-handle MIO-3-COND (location mio_3) #f 3))

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
;(midi-init 0 (location sio_0) "default" #t #t)
(io-loop)
