(import (chicken condition) (chicken file posix) (chicken foreign) (chicken io) 
        (chicken memory) (chicken port) srfi-4 srfi-18 
        typed-records)

(define-syntax λ (syntax-rules () ((_ . a) (lambda . a))))

(foreign-declare "#include \"audio.h\"")

; MIDI/audio buffers that live inside a condition-variable
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

; condition-variables that wait for MIDI/audio callbacks from C
(define-record midi-handle
  (condition-variable : (struct condition-variable))
  (mio : (or boolean pointer))
  (idx : fixnum))

(define-record audio-handle
  (condition-variable : (struct condition-variable))
  (sio : (or boolean pointer))
  (idx : fixnum))

; Scheme functions used by C (declare first?)
(define-external (stdin_eval (c-string x)) void
  (condition-case
    (for-each
      (λ (q) (let ((result (eval q)))
        (if (not (eq? (void) result)) (print result))))
      (with-input-from-string x read-list))
   (e (exn) (print (get-condition-property e 'exn 'message)
                   (get-condition-property e 'exn 'arguments)))
   (exn () (print 'unknown-input-error))))

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
         (audio-write! idx nu8 bytes-to-fill)
         (condition-variable-broadcast! cvar))))))

; C functions used by Scheme
(define stdin-init (foreign-safe-lambda void "stdin_init"))

(define audio-write! (foreign-safe-lambda void "audio_write" int u8vector int))

(define poll-io (foreign-safe-lambda void "poll_io"))

(define midi-init (foreign-safe-lambda c-pointer "midi_init"
  int u8vector c-string bool bool))

(define midi-write (foreign-safe-lambda int "midi_write"
  int u8vector int))

(define audio-init (foreign-safe-lambda c-pointer "audio_init"
  int c-string int int int int bool))

(define audio-close (foreign-safe-lambda void "audio_close" c-pointer))

; pure Scheme
(: io-loop (-> noreturn))
(define (io-loop)
  (poll-io)
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
  (let* ((name (get-setting 'name xs))
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
               (midi-init idx u8 name midi-in? midi-out?))))))

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
  (let* ((name (get-setting 'name xs))
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
          (audio-init idx name rate out-ch in-ch bits read-write?))))))

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

(: audio-over-data!
   ((struct audio-handle) #!optional (list-of (any -> any)) -> noreturn))
(define (audio-over-data! handle . fs)
  (let* ((cvar (audio-handle-condition-variable handle))
         (data (dsp-buffer-data (condition-variable-specific cvar)))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar))))
    (with-lock mutex
      (dsp-buffer-data-set!
        (condition-variable-specific cvar)
        (foldl (lambda (acc f) (f acc) acc) data fs)))))
    

(: audio-f-set! ((struct audio-handle)
                     (u8vector any fixnum -> noreturn) -> noreturn))
(define (audio-f-set! handle f)
  (let* ((cvar (audio-handle-condition-variable handle))
         (mutex (dsp-buffer-mutex (condition-variable-specific cvar)))
         (new-f (λ (u8 x n) (with-lock mutex (f u8 x n)))))
    (with-lock mutex
      (dsp-buffer-f-set! (condition-variable-specific cvar) new-f))))

; because they are closures around a specific condition variable, MIDI/audio
; handles are hard-limited and manually defined for now.
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

(make-midi-hdl mio_0_callback MIO-0-COND 0)
(make-midi-hdl mio_1_callback MIO-0-COND 1)
(make-midi-hdl mio_2_callback MIO-0-COND 2)
(make-midi-hdl mio_3_callback MIO-0-COND 3)

(make-audio-hdl sio_0_callback SIO-0-COND 0)
(make-audio-hdl sio_1_callback SIO-1-COND 1)
(make-audio-hdl sio_2_callback SIO-2-COND 2)
(make-audio-hdl sio_3_callback SIO-3-COND 3)

(: MIO-0 (struct midi-handle))
(define MIO-0 (make-midi-handle MIO-0-COND #f 0))

(: MIO-1 (struct midi-handle))
(define MIO-1 (make-midi-handle MIO-1-COND #f 1))

(: MIO-2 (struct midi-handle))
(define MIO-2 (make-midi-handle MIO-2-COND #f 2))

(: MIO-3 (struct midi-handle))
(define MIO-3 (make-midi-handle MIO-3-COND #f 3))

(: SIO-0 (struct audio-handle))
(define SIO-0 (make-audio-handle SIO-0-COND #f 0))

(: SIO-1 (struct audio-handle))
(define SIO-1 (make-audio-handle SIO-1-COND #f 1))

(: SIO-2 (struct audio-handle))
(define SIO-2 (make-audio-handle SIO-2-COND #f 2))

(: SIO-3 (struct audio-handle))
(define SIO-3 (make-audio-handle SIO-3-COND #f 3))

; runtime
(print "boar: available audio handles " '(SIO-0 SIO-1 SIO-2 SIO-3))
(print "boar: default audio settings " DEFAULT-AUDIO-SETTINGS)
(print "boar: please run (audio-start! AUDIO-HANDLE SETTINGS-OVERRIDES)")
(stdin-init)
(io-loop)
