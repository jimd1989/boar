(module boar-mixer
  (mixer-from-buffers mixer-free!)
  (import scheme (chicken base) (chicken bitwise) (chicken fixnum)
          (chicken type) srfi-4 typed-records)

  (define-record mixer-input
    (volume : float)
    (balance : f32vector)
    (buffer : f32vector))

  (define-record mixer
    (channels : fixnum)
    (master : f32vector)
    (inputs : (list-of mixer-input)))

  (: mixer-from-buffers (fixnum (list-of f32vector) --> (struct mixer)))
  (define (mixer-from-buffer ch . f32s)
    (make-mixer
      ch
      (make-f32vector (* ch (f32vector-size (car f32s))) #t #f)
      (map (lambda (f32)
             (make-mixer-input 0.0 (make-f32vector ch 0.0 #t #f) f32))
           f32s)))

  (: mixer-free! ((struct mixer) -> noreturn))
  (define (mixer-free! m)
    (release-number-vector (mixer-master m))
    (for-each! (lambda (i) (release-number-vector (mixer-input-balance i)))
               (mixer-inputs m)))

  ; probably its own egg later
  (: f32-foldl ((any float -> any) any f32vector -> any))
  (define (f32-foldl f iacc xs)
    (letrec ((len (f32vector-length xs))
             (loop (lambda (n acc)
                     (if (< n len)
                       (let* ((s (f32vector-ref xs n))
                              (nacc (f acc s)))
                         (loop (+ n 1) nacc))
                       acc))))
      (loop 0 iacc)))

  (: f32-for-each ((float -> noreturn) f32vector -> noreturn))
  (define (f32-for-each f xs) (f32-foldl (lambda (acc x) (f x) (void)) #f xs))

  (: balance (fixnum f32vector f32vector float float fixnum -> noreturn))
  (define (balance ch master balances v s n)
    (f32-foldl (lambda (nn b) (f32vector-set! master nn (* v b s)) (+ nn 1))
               (* ch n)
               balances)
    (void))

  ; mix everything as floats before mixdown
  ;(define (mix-master m)
  ;  (let ((ch (mixer-channels m))
  ;        (master (mixer-master m))
)
