(module boar-noise
  (fill-white-noise!)
  (import scheme (chicken base) (chicken foreign) (chicken type) srfi-4)

  (foreign-declare "#include \"noise.h\"")

  (define fill-white-noise
    (foreign-lambda void "fill_white_noise" u8vector int))

  (: fill-white-noise! (u8vector -> noreturn))
  (define (fill-white-noise! u8) (fill-white-noise u8 (u8vector-length u8)))
)
