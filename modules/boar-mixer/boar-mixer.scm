(module boar-mixer
  *
  (import scheme (chicken base) (chicken foreign) (chicken string) 
          (chicken type) srfi-4 typed-records)
  (import boar-param boar-slice)

  (foreign-declare "#include \"mixer.h\"")

  (define-record mixer-new
    (input-channels : fixnum)
    (output-channels : fixnum)
    (buffer-length-frames : fixnum)
    (params : (struct params))
    (audio : f32vector)
    (slices : (vector-of (struct f32slice))))

  (: audio->slices
     (f32vector fixnum fixnum fixnum  --> (vector-of (struct f32slice))))
  (define (audio->slices f32 in-ch out-ch buf-len)
    (letrec* ((audio-len (f32vector-length f32))
              (master-len (* out-ch buf-len))
              (master-slice (f32vector->slice f32 0 master-len))
              (loop (lambda (n)
                      (let ((m (+ n buf-len)))
                        (if (>= n audio-len)
                          '()
                          (cons (f32vector->slice f32 n buf-len) (loop m)))))))
      (list->vector (cons master-slice (loop master-len)))))

  (: mixer-new-from-lengths (fixnum fixnum fixnum -> (struct mixer-new)))
  (define (mixer-new-from-lengths mixer-inputs channels buffer-length-frames)
    (let* ((params-channels (+ 1 channels))
           (params-count (+ 1 channels))
           (audio-length (+ (* channels buffer-length-frames)
                            (* mixer-inputs buffer-length-frames)))
           (audio (make-f32vector audio-length 0.0 #t #f)))
      (make-mixer-new
        mixer-inputs
        channels
        buffer-length-frames
        (params-from-lengths params-channels params-count)
        audio
        (audio->slices audio mixer-inputs channels buffer-length-frames))))

  (: mixer-new-master-volume ((struct mixer-new) --> float))
  (define (mixer-new-master-volume m)
    (f32vector-ref (params-new (mixer-new-params m)) 0))

  (: mixer-new-master-volume-set! ((struct mixer-new) float -> noreturn))
  (define (mixer-new-master-volume-set! m n)
    (params-set-linear! (mixer-new-params m) 0 0 n))

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
        (params-set-linear! params 0 (+ 1 b) n))))

  (: mixer-new-channel-volume ((struct mixer-new) fixnum --> float))
  (define (mixer-new-channel-volume m ch)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ param-count (* param-count ch)))
           (params (mixer-new-params m)))
      (if (> (+ 1 ch) in-ch)
        (error (conc in-ch " input channels; got " (+ 1 ch)))
        (f32vector-ref (params-new params) idx))))

  (: mixer-new-channel-volume-set! ((struct mixer-new) fixnum float -> noreturn))
  (define (mixer-new-channel-volume-set! m ch n)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ param-count (* param-count ch)))
           (params (mixer-new-params m)))
      (if (> (+ 1 ch) in-ch)
        (error (conc in-ch " input channels; got " (+ 1 ch)))
        (params-set-linear! params (+ 1 ch) 0 n))))

  (: mixer-new-channel-balance ((struct mixer-new) fixnum fixnum --> float))
  (define (mixer-new-channel-balance m ch b)
    (let* ((in-ch (mixer-new-input-channels m))
           (out-ch (mixer-new-output-channels m))
           (param-count (+ 1 out-ch))
           (idx (+ b (+ param-count (* param-count ch))))
           (params (mixer-new-params m)))
      (cond ((> (+ 1 ch) in-ch)
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
      (cond ((> (+ 1 ch) in-ch)
             (error (conc in-ch " input channels; got " (+ 1 ch))))
             ((>= b out-ch)
              (error (conc out-ch " channels; got " (+ 1 b))))
             (else
               (params-set-linear! params (+ 1 ch) (+ 1 b) n)))))

  (: mixer-new-master-slice ((struct mixer-new) -> (struct f32slice)))
  (define (mixer-new-master-slice m)
    (vector-ref (mixer-new-slices m) 0))

  (: mixer-new-channel-slice ((struct mixer-new) fixnum --> (struct f32slice)))
  (define (mixer-new-channel-slice m ch)
    (let ((in-ch (mixer-new-input-channels m)))
      (if (> (+ 1 ch) in-ch)
        (error (conc in-ch " input channels; got " (+ 1 ch)))
        (vector-ref (mixer-new-slices m) (+ ch 1)))))

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
           (fade-len 128))
      ((foreign-lambda void "mixer_zero" int int f32vector)
       out-ch buf-len-frames audio)
      (if updated?
        (begin
          ((foreign-lambda int "mix_f32_fade"
            int int f32vector int int int f32vector)
           fade-len param-len params-curves in-ch out-ch buf-len-frames audio)
          ((foreign-lambda void "mix_f32"
            int f32vector int int int f32vector)
           fade-len static-params in-ch out-ch buf-len-frames audio)
          ((foreign-lambda int "mix_s16_fade"
            int int f32vector int int f32vector u8vector)
           fade-len param-len params-curves out-ch buf-len-frames audio u8)
          (params-after-fade-cleanup! params)
          ((foreign-lambda void "mix_s16"
            int f32vector int int f32vector u8vector)
           fade-len static-params out-ch buf-len-frames audio u8))
        (begin
          ((foreign-lambda void "mix_f32"
            int f32vector int int int f32vector)
           0 static-params in-ch out-ch buf-len-frames audio)
          ((foreign-lambda void "mix_s16"
            int f32vector int int f32vector u8vector)
           0 static-params out-ch buf-len-frames audio u8)))))
)
