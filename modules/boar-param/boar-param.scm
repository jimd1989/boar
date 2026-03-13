(module boar-param
  (make-empty-param param-free! param-set-linear!)

  (import scheme (chicken base) (chicken foreign) (chicken type) srfi-4 
          typed-records)

  (foreign-declare "#include \"param.h\"")

  (define PARAM-SIZE (foreign-value "PARAM_SIZE" int))

  (define-record param
    (updated? : boolean)
    (old : float)
    (new : float)
    (vector : f32vector))

  (: make-empty-param (--> (struct param)))
  (define (make-empty-param)
    (make-param #f 0.0 0.0 (make-f32vector PARAM-SIZE 0.0 #t #f)))

  (: param-free! ((struct param) -> noreturn))
  (define (param-free! p) (release-number-vector (param-vector p)))

  (: param-set-linear! ((struct param) float -> noreturn))
  (define (param-set-linear! p x)
    (let ((new-old (param-new p))
          (vec (param-vector p)))
      (param-updated?-set! p #t)
      (param-old-set! p new-old)
      (param-new-set! p x)
      ((foreign-lambda void "param_set_linear" float float f32vector)
       new-old x vec)))
)
