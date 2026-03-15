(import boar-mixer boar-noise boar-slice boar-osc srfi-4 srfi-18)

; assumes stereo audio, 48Khz sample rate, 1184 buffsize
; eventually needs automated way to access playback params
(audio-start! SIO-0)

; init 4 channel mixer
(define mm (mixer-from-lengths 4 2 1184))

; audio handle state = mixer
(audio-data-set! SIO-0 mm)

; all state changes behind mutex when done in (audio-over-data!)
; this may not be needed if program is truly single-threaded
(audio-over-data! SIO-0 (lambda (m) (mixer-master-volume-set! m 0.4)))

; slightly terser syntax
; consider more curry friendly order eventually
(define-syntax audio-params-set!
  (syntax-rules ()
    ((_ handle (f args ...) ...)
     (audio-over-data! handle (lambda (data) (f data args ...)) ...))))

; finish master mix setup
(audio-params-set! SIO-0
  (mixer-master-volume-set!  0.4)
  (mixer-master-balance-set! 0 1.0)
  (mixer-master-balance-set! 1 1.0))

; channel 1 → white noise (left ear)
(define no (noise-from-length 1184))
(audio-params-set! SIO-0
  (mixer-channel-volume-set!  0 0.2)
  (mixer-channel-balance-set! 0 0 1.0)
  (mixer-channel-balance-set! 0 1 0.0))

; channel 2 → 440hz sine wave (right ear)
(define sine (make-sine-wavetable))
(define o2 (osc-from-wavetable sine 48000))
(osc-freq-set! o2 440.0)
(audio-params-set! SIO-0
  (mixer-channel-volume-set!  1 0.8)
  (mixer-channel-balance-set! 1 0 0.0)
  (mixer-channel-balance-set! 1 1 1.0))

; main DSP loop, x = audio handle state (mixer)
(audio-f-set! SIO-0
  (lambda (u8 x n)
    (if (> n 0)
      (let ((ch1-sl (mixer-channel->slice x 0))
            (ch2-sl (mixer-channel->slice x 1)))
        ; generate noise
        (fill-white-noise! no)
        (f32slice-copy-from-f32vector ch1-sl (noise-white-f32 no))

        ; advance oscillator
        (osc-fill-slice! o2 ch2-sl)

        ; mixdown
        (mixer-mix! x u8)))))

(##sys#gc)

; MIDI handling
(midi-start! MIO-0)
(midi-f-set! MIO-0
  (lambda (u8 x n)
    (let ((note (u8vector-ref u8 1)))
      (osc-freq-set! o2 (* 8.1757989156 (expt 2 (/ note 12)))))))
