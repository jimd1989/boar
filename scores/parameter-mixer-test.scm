(import boar-mixer boar-noise boar-slice boar-osc srfi-4 srfi-18)

; audio handle
(audio-start! SIO-0)

; 2 outputs, 2 inputs, buffer len 1184
(define mix (mixer-new-from-lengths 2 2 1184))
(mixer-new-master-volume-set!   mix     0.2)
(mixer-new-master-balance-set!  mix 0   1.0)
(mixer-new-master-balance-set!  mix 1   1.0)

; input channel 1 → left ear
(mixer-new-channel-volume-set!  mix 0   1.0)
(mixer-new-channel-balance-set! mix 0 0 1.0)
(mixer-new-channel-balance-set! mix 0 1 0.0)

; input channel 2 → right ear
(mixer-new-channel-volume-set!  mix 1   1.0)
(mixer-new-channel-balance-set! mix 1 0 0.0)
(mixer-new-channel-balance-set! mix 1 1 1.0)

; audio handle state = mixer
(audio-data-set! SIO-0 mix)

; noise
(define no (noise-from-length 1184))

; sine
(define sine (make-sine-wavetable))
(define o2 (osc-from-wavetable sine 48000))

; scratch
(osc-freq-set! o2 880.0)
(f32slice-vector (mixer-new-channel-slice mix 0))

; main audio loop
(audio-f-set! SIO-0
  (lambda (u8 x n)
    (if (> n 0)
      (let ((ch1-sl (mixer-new-channel-slice mix 0))
            (ch2-sl (mixer-new-channel-slice mix 1)))
        ; generate noise
        (fill-white-noise! no)
        ;(f32slice-copy-from-f32vector ch1-sl (noise-white-f32 no))
        (f32slice-copy-from-f32vector ch2-sl (noise-white-f32 no))

        ; advance oscillator
        (osc-fill-slice! o2 ch1-sl)

        ; mixdown
        (mixer-new-mix! x u8)))))

(##sys#gc)

; OLD
; (import boar-param boar-mixer boar-slice srfi-4 srfi-18 (chicken string))
; 
; (define mix (mixer-new-from-lengths 2 2 4))
; (mixer-new-master-volume-set!   mix     111.0)
; (mixer-new-master-balance-set!  mix 0   222.0)
; (mixer-new-master-balance-set!  mix 1   333.0)
; (mixer-new-channel-volume-set!  mix 0   444.0)
; (mixer-new-channel-volume-set!  mix 1   777.0)
; (mixer-new-channel-balance-set! mix 0 0 555.0)
; (mixer-new-channel-balance-set! mix 0 1 666.0)
; (mixer-new-channel-balance-set! mix 1 0 888.0)
; (mixer-new-channel-balance-set! mix 1 1 999.0)
; (params-new (mixer-new-params mix))
; (define output (make-u8vector 16 0 #t #f))
; (params-updated? (mixer-new-params mix))
; (mixer-new-mix! mix output)
; (mixer-new-audio mix)
; (vector-ref (mixer-new-slices mix) 0)
; (f32vector-length (mixer-new-audio mix))
; (f32slice-to-write (mixer-new-master-slice mix))
; (f32slice-written (mixer-new-channel-slice mix 0))
; (f32slice-written (mixer-new-channel-slice mix 1))
; mix


