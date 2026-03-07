(module boar-mixer
  (mixer mixer-output-channels mixer-input-channels
         mixer-from-lengths mixer-free! mixer-master-volume 
         mixer-master-volume-set! mixer-channel-volume
         mixer-channel-volume-set! mixer-master-balance
         mixer-master-balance-set! mixer-channel-balance
         mixer-channel-balance-set! mixer-channel->slice mixer-mix!)
  (import scheme (chicken base) (chicken foreign) (chicken type) srfi-4 
          typed-records)

  (foreign-declare "#include \"mixer.h\"")

  (define mix-s16
    (foreign-lambda void "mix_s16" 
      int int int f32vector f32vector f32vector u8vector))

  (define-record mixer
    (input-channels : fixnum)
    (output-channels : fixnum)
    (buffer-length-frames : fixnum)
    (volumes : f32vector)
    (balances : f32vector)
    (audio : f32vector))

  ; + 1 for master balances/mix/etc
  (: mixer-from-lengths (fixnum fixnum fixnum -> (struct mixer)))
  (define (mixer-from-lengths mixer-inputs channels buffer-length-frames)
    (let ((volumes-length (+ 1 mixer-inputs))
          (balances-length (* channels (+ 1 mixer-inputs)))
          (audio-length (+ (* channels buffer-length-frames)
                           (* mixer-inputs buffer-length-frames))))
    (make-mixer
      mixer-inputs
      channels
      buffer-length-frames
      (make-f32vector volumes-length 0.0 #t #f)
      (make-f32vector balances-length 1.0 #t #f)
      (make-f32vector audio-length 0.0 #t #f))))

  (: mixer-free! ((struct mixer) -> noreturn))
  (define (mixer-free! m)
    (release-number-vector (mixer-volumes m))
    (release-number-vector (mixer-balances m))
    (release-number-vector (mixer-audio m)))

  (: mixer-master-volume ((struct mixer) -> float))
  (define (mixer-master-volume m) (f32vector-ref (mixer-volumes m) 0))

  (: mixer-master-volume-set! ((struct mixer) float -> noreturn))
  (define (mixer-master-volume-set! m x) (f32vector-set! (mixer-volumes m) 0 x))

  (: mixer-channel-volume ((struct mixer) fixnum -> float))
  (define (mixer-channel-volume m n) (f32vector-ref (mixer-volumes m) (+ n 1)))

  (: mixer-channel-volume-set! ((struct mixer) fixnum float -> noreturn))
  (define (mixer-channel-volume-set! m n x)
    (f32vector-set! (mixer-volumes m) (+ n 1) x))

  (: mixer-master-balance ((struct mixer) fixnum -> float))
  (define (mixer-master-balance m n)
    (if (or (>= n (mixer-output-channels m))
            (< n 0))
      -1.0
      (f32vector-ref (mixer-balances m) n)))

  (: mixer-master-balance-set! ((struct mixer) fixnum float -> noreturn))
  (define (mixer-master-balance-set! m b x)
    (if (or (>= b (mixer-output-channels m))
            (< b 0))
      (void)
      (f32vector-set! (mixer-balances m) b x)))

  (: mixer-channel-balance ((struct mixer) fixnum fixnum -> float))
  (define (mixer-channel-balance m n b)
    (if (or (>= n (mixer-input-channels m))
            (>= b (mixer-output-channels m))
            (< n 0))
      -1.0
      (f32vector-ref
        (mixer-balances m) (+ b (* (+ n 1) (mixer-output-channels m))))))

  (: mixer-channel-balance-set!
     ((struct mixer) fixnum fixnum float -> noreturn))
  (define (mixer-channel-balance-set! m n b x)
    (if (or (>= n (mixer-input-channels m))
            (>= b (mixer-output-channels m))
            (< n 0))
      (void)
      (f32vector-set!
        (mixer-balances m) (+ b (* (+ n 1) (mixer-output-channels m))) x)))

  (: mixer-channel->slice
     ((struct mixer) fixnum -> (list f32vector fixnum fixnum)))
  (define (mixer-channel->slice m n)
    (let ((len (mixer-buffer-length-frames m))
          (in-ch (mixer-input-channels m))
          (out-ch (mixer-output-channels m)))
      (if (>= n in-ch)
        `(,(mixer-audio m) 0 0)
        `(,(mixer-audio m) ,(+ (* out-ch len) (* n len)) ,len))))

  (: mixer-mix! ((struct mixer) u8vector -> noreturn))
  (define (mixer-mix! m u8)
    (mix-s16 (mixer-input-channels m)
             (mixer-output-channels m)
             (mixer-buffer-length-frames m)
             (mixer-volumes m)
             (mixer-balances m)
             (mixer-audio m)
             u8))
)
