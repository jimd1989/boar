(module boar-mixer
  *
  (import scheme (chicken base) (chicken foreign) (chicken string) 
          (chicken type) srfi-4 typed-records)
  (import boar-param boar-slice)

  (foreign-declare "#include \"mixer.h\"")

  (define mix-s16
    (foreign-safe-lambda void "mix_s16" 
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

  (: mixer-channel->slice ((struct mixer) fixnum --> (struct f32slice)))
  (define (mixer-channel->slice m n)
    (let ((len (mixer-buffer-length-frames m))
          (in-ch (mixer-input-channels m))
          (out-ch (mixer-output-channels m)))
      (if (>= n in-ch)
        (f32vector->slice (mixer-audio m) 0 0)
        (f32vector->slice (mixer-audio m) (+ (* out-ch len) (* n len)) len))))

  (: mixer-mix! ((struct mixer) u8vector -> noreturn))
  (define (mixer-mix! m u8)
    (mix-s16 (mixer-input-channels m)
             (mixer-output-channels m)
             (mixer-buffer-length-frames m)
             (mixer-volumes m)
             (mixer-balances m)
             (mixer-audio m)
             u8))

  ; refactor starts here
  (define-record mixer-new
    (input-channels : fixnum)
    (output-channels : fixnum)
    (buffer-length-frames : fixnum)
    (params : (struct params))
    (audio : f32vector)
    (slices : (vector-of (struct boar-slice))))

  (: audio->slices
     (f32vector fixnum fixnum fixnum  --> (vector-of (struct boar-slice))))
  (define (audio->slices f32 in-ch out-ch buf-len)
    (letrec* ((audio-len (f32vector-length f32))
              (master-len (* out-ch buf-len))
              (master-slice (f32vector->slice f32 0 master-len))
              (loop (lambda (n)
                      (if (> n audio-len)
                        '()
                        (let ((m (+ n buf-len)))
                          (cons (f32vector->slice f32 n m) (loop m)))))))
      (list->vector (cons master-slice (loop master-len)))))

  (: mixer-new-from-lengths (fixnum fixnum fixnum -> (struct mixer-new)))
  (define (mixer-new-from-lengths mixer-inputs channels buffer-length-frames)
    (let* ((params-count (* (+ 1 channels) (+ 1 mixer-inputs)))
           (audio-length (+ (* channels buffer-length-frames)
                            (* mixer-inputs buffer-length-frames)))
           (audio (make-f32vector audio-length 0.0 #t #f)))
      (make-mixer-new
        mixer-inputs
        channels
        buffer-length-frames
        (params-from-length params-count)
        audio
        (audio->slices audio mixer-inputs channels buffer-length-frames))))

  (: mixer-new-master-volume ((struct mixer-new) --> float))
  (define (mixer-new-master-volume m)
    (f32vector-ref (params-new (mixer-new-params m)) 0))

  (: mixer-new-master-volume-set! ((struct mixer-new) float -> noreturn))
  (define (mixer-new-master-volume-set! m n)
    (params-set-linear! (mixer-new-params m) 0 n))

  (: mixer-new-master-balance ((struct mixer-new) fixnum --> float))
  (define (mixer-new-master-balance m b)
    (let ((out-ch (mixer-new-output-channels m))
          (params (mixer-new-params m)))
      (if (>= b out-ch)
        (error (conc out-ch " channels; got " (+ 1 b)))
        (f32vector-ref (params-new params) (+ 1 b)))))

  (: mixer-new-master-balance-set! ((struct mixer-new) fixnum float -> noreturn))
  (define (mixer-new-master-balance-set! m b n)
    (let ((out-ch (mixer-new-output-channels m))
          (params (mixer-new-params m)))
      (if (>= b out-ch)
        (error (conc out-ch " channels; got " (+ 1 b)))
        (params-set-linear! params (+ 1 b) n))))

  (: mixer-new-channel-volume ((struct mixer-new) fixnum --> float))
  (define (mixer-new-channel-volume m ch)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ param-count (* param-count ch)))
           (params (mixer-new-params m)))
      (if (>= ch in-ch)
        (error (conc in-ch " input channels; got " (+ 1 ch)))
        (f32vector-ref (params-new params) idx))))

  (: mixer-new-channel-volume-set! ((struct mixer-new) fixnum float -> noreturn))
  (define (mixer-new-channel-volume-set! m ch n)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ param-count (* param-count ch)))
           (params (mixer-new-params m)))
      (if (>= ch in-ch)
        (error (conc in-ch " input channels; got " (+ 1 ch)))
        (params-set-linear! params idx n))))

  (: mixer-new-channel-balance ((struct mixer-new) fixnum fixnum --> float))
  (define (mixer-new-channel-balance m ch b)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ b (+ param-count (* param-count ch))))
           (params (mixer-new-params m)))
      (cond ((>= ch in-ch)
             (error (conc in-ch " input channels; got " (+ 1 ch))))
             ((>= b out-ch)
              (error (conc out-ch " channels; got " (+ 1 b))))
             (else
               (f32vector-ref (params-new params) idx)))))

  (: mixer-new-channel-balance-set! ((struct mixer-new) fixnum fixnum float -> noreturn))
  (define (mixer-new-channel-balance-set! m ch b n)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ 1 b (+ param-count (* param-count ch))))
           (params (mixer-new-params m)))
      (cond ((>= ch in-ch)
             (error (conc in-ch " input channels; got " (+ 1 ch))))
             ((>= b out-ch)
              (error (conc out-ch " channels; got " (+ 1 b))))
             (else
               (params-set-linear! params idx n)))))

  (: mixer-new-mix! ((struct mixer-new) u8vector -> noreturn))
  (define (mixer-new-mix! m u8)
    (let* ((params (mixer-new-params m))
           (updated? (params-updated? params))
           (param-len (params-fade-length params))
           (params-curves (params-vector params))
           (static-params (params-new params))
           (in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (buf-len-frames (mixer-new-buffer-length-frames m))
           (audio (mixer-new-audio m))
           (fade-len 32))
      ((foreign-lambda void "mixer_zero" int int f32vector)
       out-ch buf-len-frames audio)
      (if updated?
        (begin
          ((foreign-lambda int "mix_f32_fade"
            int int f32vector int int int f32vector)
           fade-len param-len params-curves in-ch out-ch buf-len-frames audio)
          (params-after-fade-cleanup! params)
          ((foreign-lambda void "mix_f32_new"
            int f32vector int int int f32vector)
           fade-len static-params in-ch out-ch buf-len-frames audio)
          ((foreign-lambda int "mix_s16_fade"
            int int f32vector int int f32vector u8vector)
           fade-len param-len static-params out-ch buf-len-frames audio u8)
          ((foreign-lambda void "mix_s16_new"
            int f32vector int int f32vector u8vector)
           fade-len static-params out-ch buf-len-frames audio u8))
        (begin
          ((foreign-lambda void "mix_f32_new"
            int f32vector int int int f32vector)
           0 static-params in-ch out-ch buf-len-frames audio)
          ((foreign-lambda void "mix_s16_new"
            int f32vector int int f32vector u8vector)
           0 static-params out-ch buf-len-frames audio u8)))))
)
