(module boar-noise
  (noise noise-white-u32 noise-white-f32 noise-from-length free-noise!
   fill-white-noise!)
  (import scheme (chicken base) (chicken foreign) (chicken type) 
          srfi-4 typed-records)

  (foreign-declare "#include \"noise.h\"")

  (define-record noise
    (white-u32 : u32vector)
    (white-f32 : f32vector))

  (: noise-from-length (fixnum -> (struct noise)))
  (define (noise-from-length len)
    (let ((u32 (make-u32vector len 0 #t #f))
          (f32 (make-f32vector len 0.0 #t #f)))
      (make-noise u32 f32)))

  (: free-noise! ((struct noise) -> noreturn))
  (define (free-noise! no)
    (release-number-vector (noise-white-u32 no))
    (release-number-vector (noise-white-f32 no)))

  (define fill-white-noise
    (foreign-lambda void "fill_white_noise" u32vector f32vector int))

  (: fill-white-noise! ((struct noise) -> noreturn))
  (define (fill-white-noise! no)
    (fill-white-noise (noise-white-u32 no)
                      (noise-white-f32 no)
                      (u32vector-length (noise-white-u32 no))))
)
