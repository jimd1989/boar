(module boar-osc
  (osc make-sine-wavetable osc-from-wavetable osc-table osc-table-set! 
       osc-freq osc-freq-set! osc-phase osc-phase-set! osc-fill-slice!)
  (import scheme (chicken base) (chicken foreign) (chicken type) 
          srfi-4 typed-records)
  (import boar-slice)

  (foreign-declare "#include \"osc.h\"")
  (define-constant WAVETABLE-SIZE 4096)

  ; should eventually be an osc slice bank over contiguous array
  (define-record osc
    (table : f32vector)
    (inc : float)
    (freq : float)
    (phase : float))

  (: make-sine-wavetable (-> f32vector))
  (define (make-sine-wavetable)
    (let ((f32 (make-f32vector WAVETABLE-SIZE 0.0 #t #f)))
      ((foreign-safe-lambda void "make_sine_wavetable" f32vector) f32) f32))

  (: osc-from-wavetable (f32vector fixnum --> (struct osc)))
  (define (osc-from-wavetable f32 sample-rate)
    (let* ((len (f32vector-length f32))
           (inc (exact->inexact (/ len sample-rate))))
      (make-osc f32 inc 1.0 0.0)))

  (: osc-fill-slice! ((struct osc) (struct f32slice) -> noreturn))
  (define (osc-fill-slice! o sl)
    (let* ((table (osc-table o))
           (freq-inc (* (osc-inc o) (osc-freq o)))
           (phase (osc-phase o))
           (f32 (f32slice-vector sl))
           (written (f32slice-written sl))
           (to-write (f32slice-to-write sl))
           (new-phase ((foreign-safe-lambda float "osc_fill_slice" 
                          f32vector float float f32vector int int)
                       table freq-inc phase f32 written to-write)))
      (osc-phase-set! o new-phase)))


)
