(import boar-mixer boar-noise boar-slice boar-osc srfi-4)

; assumes stereo audio, 48Khz sample rate, 1184 buffsize
(audio-start! SIO-0)

; init 4 channel mixer
(define mm (mixer-from-lengths 4 2 1184))
(mixer-master-volume-set! mm 1.0)

; channel 1 → white noise (left ear)
(define ch1-sl (mixer-channel->slice mm 0))
(mixer-channel-volume-set! mm 0 0.6)
(mixer-channel-balance-set! mm 0 0 1.0)
(mixer-channel-balance-set! mm 0 1 0.0)
(define no (noise-from-length 1184))

; channel 2 → 440hz sine wave (right ear)
(define ch2-sl (mixer-channel->slice mm 1))
(mixer-channel-volume-set! mm 1 0.6)
(mixer-channel-balance-set! mm 0 0 0.0)
(mixer-channel-balance-set! mm 0 1 1.0)
(define sine (make-sine-wavetable))
(define o2 (osc-from-wavetable sine 48000))
(osc-freq-set! o2 440.0)

; main DSP loop, x = audio handle state (mixer)
(audio-data-set! SIO-0 mm)
(audio-f-set! SIO-0
  (lambda (u8 x n)
    ; generate noise
    (fill-white-noise! no)
    (f32slice-copy-from-f32vector ch1-sl (noise-white-f32 no))

    ; advance oscillator
    (osc-fill-slice! o2 ch2-sl)

    ; mixdown
    (mixer-mix! x u8)))
