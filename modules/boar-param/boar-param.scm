(module boar-param
  (params params-updated? params-updated?-set! params-channels params-count
   params-fade-length params-old params-new params-vector params-from-lengths
   params-free! params-set-linear! params-after-fade-cleanup!)
  (import scheme (chicken base) (chicken foreign) (chicken string) 
          (chicken type) srfi-4 typed-records)

  ; Calculates fade curves upon parameter changes, but actually mixing against
  ; them is the responsibility of the calling module. "Plural" by design:
  ; assumes a (channels × count) matrix of similar parameters.

  (foreign-declare "#include \"param.h\"")

  (define PARAM-SIZE (foreign-value "PARAM_SIZE" int))

  (define-record params
    (updated? : boolean)
    (channels : fixnum)
    (count : fixnum)
    (fade-length : fixnum)
    (old : f32vector)
    (new : f32vector)
    (vector : f32vector))

  (: params-from-lengths (fixnum fixnum --> (struct params)))
  (define (params-from-lengths channels count)
    (let* ((population (* channels count))
           (old (make-f32vector population 0.0 #t #f))
           (new (make-f32vector population 0.0 #t #f))
           (vec (make-f32vector (* population PARAM-SIZE) 0.0 #t #f)))
      (make-params #f channels count PARAM-SIZE old new vec)))

  (: params-free! ((struct params) -> noreturn))
  (define (params-free! p)
    (release-number-vector (params-old p))
    (release-number-vector (params-new p))
    (release-number-vector (params-vector p)))

  (: params-set-linear! ((struct params) fixnum fixnum float -> noreturn))
  (define (params-set-linear! p ch n x)
    (let ((pch (params-channels p))
          (pco (params-count p)))
      (cond ((>= ch pch) 
             (error (conc "Only " pch " param chans; got idx " ch)))
            ((>= n pco)
             (error (conc "Only " pco " params; got idx " n)))
            (else (let* ((vec (params-vector p))
                         (idx (+ n (* ch (params-count p))))
                         (new-old (f32vector-ref (params-new p) idx))
                         (new (params-new p))
                         (old (params-old p)))
                    (f32vector-set! new idx x)
                    (f32vector-set! old idx new-old)
                    ((foreign-lambda void "params_set_linear" 
                                     int int int float float f32vector)
                     pco ch n new-old x vec)
                    (params-updated?-set! p #t))))))

  ; When in a batch-fading block for 1 param, the program must not refade the
  ; other old params, but querying param age would involve needless branching.
  ; After a param fade has been applied, run this function to set all params
  ; to a flat curve of new, new, new ... for future branchless fading.
  (: params-after-fade-cleanup! ((struct params) -> noreturn))
  (define (params-after-fade-cleanup! p)
    (let ((vec (params-vector p))
          (new (params-new p))
          (pch (params-channels p))
          (pco (params-count p)))
      ((foreign-lambda void "params_after_fade_cleanup"
          int int f32vector f32vector)
       pch pco new vec)
      (params-updated?-set! p #f)))
)
