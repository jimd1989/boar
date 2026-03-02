(module boar-mixer
  (mixer mixer-channels mixer-volume mixer-volume-set! mixer-from-buffers
         mixer-free! mixer-channel-volume mixer-channel-volume-set!
         mixer-channel-balance mixer-channel-balance-set!)
  (import scheme (chicken base) (chicken bitwise) (chicken fixnum)
          (chicken memory) (chicken type) srfi-4 typed-records)
  (import boar-vectors)

  (define-record mixer-input
    (volume : float)
    (balance : f32vector)
    (buffer : f32vector))

  (define-record mixer
    (channels : fixnum)
    (volume : float)
    (master : f32vector)
    (silence : f32vector)
    (inputs : (vector-of (struct mixer-input))))

  (: mixer-from-buffers (fixnum (list-of f32vector) --> (struct mixer)))
  (define (mixer-from-buffers ch . f32s)
    (make-mixer
      ch
      0.0
      (make-f32vector (* ch (f32vector-length (car f32s))) 0.0 #t #f)
      (make-f32vector (* ch (f32vector-length (car f32s))) 0.0 #t #f)
      (list->vector
        (map (lambda (f32)
               (make-mixer-input 0.0 (make-f32vector ch 0.0 #t #f) f32))
             f32s))))

  (: mixer-free! ((struct mixer) -> noreturn))
  (define (mixer-free! m)
    (release-number-vector (mixer-master m))
    (release-number-vector (mixer-silence m))
    (vector-for-each
      (lambda (i) (release-number-vector (mixer-input-balance i)))
      (mixer-inputs m)))

  ; 1 monophonic sample mixed to interleaved master channel
  ; for n channel audio, returns the current index of master channel + n
  (: mix-sample-to-master (f32vector fixnum float f32vector float -> fixnum))
  (define (mix-sample-to-master master-buffer master-idx volume balances s)
    (f32vector-foldl
      (lambda (master-n balance local-n)
        (let* ((old (f32vector-ref master-buffer master-n))
               (new (+ old (* volume balance s))))
          (f32vector-set! master-buffer master-n new)
          (+ master-n 1)))
      master-idx 
      balances))

  (: mix-input-to-master (f32vector (struct mixer-input) -> fixnum))
  (define (mix-input-to-master master-buffer input)
    (let ((volume (mixer-input-volume input))
          (balances (mixer-input-balance input))
          (local-buffer (mixer-input-buffer input)))
      (f32vector-foldl
        (lambda (master-n local-sample local-n)
          (mix-sample-to-master
            master-buffer master-n volume balances local-sample))
        0
        local-buffer)))

  (: mix-master-to-f32 ((struct mixer) -> f32vector))
  (define (mix-master-to-f32 mixer)
    (let ((master-buffer (mixer-master mixer))
          (silence (mixer-silence mixer))
          (inputs (mixer-inputs mixer)))
      (move-memory! silence master-buffer)
      (vector-for-each
        (lambda (input) (mix-input-to-master master-buffer input))
        inputs)
      master-buffer))

  ;(: mix-master-to-s16 ((struct mixer) u8vector -> u8vector))
  ;(define (mix-master-to-s16 mixer output-buffer)
  ;  (let* ((output-s16 (blob->s16vector/shared (u8vector->blob/shared xs))))
  ;    (f32vector-foldl (lambda (acc f32-sample n)
  ;                       (let ((
  ;  (mix-master-to-f32)


  ; "input" is an internal name—"channel" is external name to avoid clashing
  (: mixer-input-ref (fixnum (struct mixer) --> (struct mixer-input)))
  (define (mixer-input-ref n m) (vector-ref (mixer-inputs m) n))

  (: mixer-channel-volume (fixnum (struct mixer) --> float))
  (define (mixer-channel-volume n m) (mixer-input-volume (mixer-input-ref n m)))

  (: mixer-channel-volume-set! (fixnum float (struct mixer) -> noreturn))
  (define (mixer-channel-volume-set! n x m)
    (mixer-input-volume-set! x (mixer-input-ref n m)))

  (: mixer-channel-balance (fixnum fixnum (struct mixer) -> noreturn))
  (define (mixer-channel-balance n b m)
    (f32vector-ref (mixer-input-balance (mixer-input-ref n m)) b))

  (: mixer-channel-balance-set!
     (fixnum fixnum float (struct mixer) -> noreturn))
  (define (mixer-channel-balance-set! n b x m)
    (f32vector-set! (mixer-input-balance (mixer-input-ref n m)) b x))
)
