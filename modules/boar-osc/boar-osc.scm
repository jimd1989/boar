(module boar-osc
  (make-sine-wavetable)
  (import scheme (chicken base) (chicken foreign) (chicken type) 
          srfi-4 typed-records)
  (import boar-slice)

  (foreign-declare "#include \"osc.h\"")

  (: make-sine-wavetable (fixnum -> f32vector))
  (define (make-sine-wavetable n)
    (let ((f32 (make-f32vector n 0.0 #t #f)))
      ((foreign-lambda void "make_sine_wavetable" f32vector int) f32 n)
      f32))
)
