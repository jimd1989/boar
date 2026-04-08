(module boar-envelope
  ()
  (import scheme (chicken base) (chicken foreign) (chicken string) 
          (chicken type) srfi-4 typed-records)

  (import boar-param)

  (foreign-declare "#include \"envelope.h\"")

  (define ENV-ATTACK (foreign-value "ENV_ATTACK" int))

  (define-record envelopes
    (currently-running : fixnum)
    (stages : u32vector)
    (phases : f32vector)
    (values : f32vector)
    (curves : (struct params)))

  (: no-envs-running? ((struct envelopes) --> boolean))
  (define (no-envelopes-running? es) 
    (> (envelopes-currently-running es) ENV-ATTACK))

)
