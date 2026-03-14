(module boar-param
  (params params-updated? params-updated?-set! params-population 
   params-fade-length params-old params-new params-vector params-from-length
   params-free! params-set-linear! params-after-fade-cleanup!)
  (import scheme (chicken base) (chicken foreign) (chicken string) 
          (chicken type) srfi-4 typed-records)

  ; Calculates fade curves upon parameter changes, but actually mixing against
  ; them is the responsibility of the calling module.

  (foreign-declare "#include \"param.h\"")

  (define PARAM-SIZE (foreign-value "PARAM_SIZE" int))

  (define-record params
    (updated? : boolean)
    (population : fixnum)
    (fade-length : fixnum)
    (old : f32vector)
    (new : f32vector)
    (vector : f32vector))

  (: params-from-length (fixnum --> (struct params)))
  (define (params-from-length n)
    (let ((old (make-f32vector n 0.0 #t #f))
          (new (make-f32vector n 0.0 #t #f))
          (vec (make-f32vector (* n PARAM-SIZE) 0.0 #t #f)))
      (make-params #f n PARAM-SIZE old new vec)))

  (: params-free! ((struct params) -> noreturn))
  (define (params-free! p)
    (release-number-vector (params-old p))
    (release-number-vector (params-new p))
    (release-number-vector (params-vector p)))

  (: params-set-linear! ((struct params) fixnum float -> noreturn))
  (define (params-set-linear! p n x)
    (if (>= n (params-population p))
      (error (conc "Only " (params-population p) " params; got idx " n)) 
      (let ((vec (params-vector p))
            (new-old (f32vector-ref (params-new p) n)))
        (f32vector-set! (params-new p) n x)
        (f32vector-set! (params-old p) n new-old)
        ((foreign-lambda void "params_set_linear" int int float float f32vector)
         (params-population p) n new-old x vec)
        (params-updated?-set! p #t))))

  ; When in a batch-fading block for 1 param, the program must not refade the
  ; other old params, but querying param age would involve needless branching.
  ; After a param fade has been applied, run this function to set all params
  ; to a flat curve of new, new, new ... for future branchless fading.
  (: params-after-fade-cleanup! ((struct params) -> noreturn))
  (define (params-after-fade-cleanup! p)
    (let ((vec (params-vector p))
          (new (params-new p))
          (population (params-population p)))
      ((foreign-lambda void "params_after_fade_cleanup" int f32vector f32vector)
       population new vec)
      (params-updated?-set! #f)))
)
