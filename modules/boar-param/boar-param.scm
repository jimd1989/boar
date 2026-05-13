(module boar-param
  *
  (import scheme (chicken base) (chicken foreign) (chicken string) 
          (chicken type) srfi-4 typed-records)

  ; Calculates fade curves upon parameter changes, but actually mixing against
  ; them is the responsibility of the calling module. "Plural" by design:
  ; assumes a (channels × count) matrix of similar parameters.

  (foreign-declare "#include \"param.h\"")

  (define PARAM-SIZE (foreign-value "PARAM_SIZE" int))

  ; Potential new flow:
  ; if total-samples-to-fade > 0 then enter fade function for entire buffer
  ; increment phases with independent incs
  ; param value is (PARAM-SIZE - 1) × maxf(phase, 1.0f) = index of fade
  ; if samples-to-fade[i] = 0, 
  ;  .. then increment[i] = 0 → always read max val
  ;  .. IMPORTANT: (linear curve should be (0, max] instead now
  ; after fade, scan phases. if 1.0f, set inc to 0.0, subtract 
  ;   samples-to-fade from total-samples-to-fade
  ; fades can now take place at different speeds or transcend buffer fills
  ; should be no more need for weird interleaving thing or updated? field
  (define-record params
    (updated? : boolean)
    (channels : fixnum)  
    (count : fixnum)
    (fade-length : fixnum)
    (total-samples-to-fade : fixnum) ; NEW
    (old : f32vector)
    (new : f32vector)
    (phases : f32vector)             ; NEW
    (increments : f32vector)         ; NEW
    (samples-to-fade : u32vector)    ; NEW
    (vector : f32vector))

  (: params-from-lengths (fixnum fixnum --> (struct params)))
  (define (params-from-lengths channels count)
    (let* ((population (* channels count))
           (old (make-f32vector population 0.0 #t #f))
           (new (make-f32vector population 0.0 #t #f))
           (phases (make-f32vector population 0.0 #t #f))
           (increments (make-f32vector population 0.0 #t #f))
           (samples-to-fade (make-u32vector population 0 #t #f))
           (vec (make-f32vector (* population PARAM-SIZE) 0.0 #t #f)))
      (make-params #f channels count PARAM-SIZE 0
                   old new phases increments samples-to-fade vec)))

  (: params-free! ((struct params) -> noreturn))
  (define (params-free! p)
    (release-number-vector (params-old p))
    (release-number-vector (params-new p))
    (release-number-vector (params-phases p))
    (release-number-vector (params-increments p))
    (release-number-vector (params-samples-to-fade p))
    (release-number-vector (params-vector p)))

  (: params-set-linear!
     ((struct params) fixnum fixnum float #!optional fixnum -> noreturn))
  (define (params-set-linear! p ch n x #!optional (samples PARAM-SIZE))
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
                         (old (params-old p))
                         (phases (params-phases p))
                         (increments (params-increments p))
                         (to-fade (params-samples-to-fade p))
                         (total-samples
                           (+ samples (params-total-samples-to-fade p)))
                         (inc (/ 1.0 samples)))
                    (f32vector-set! new idx x)
                    (f32vector-set! old idx new-old)
                    (f32vector-set! phases idx 0.0)
                    (f32vector-set! increments idx inc)
                    (u32vector-set! to-fade idx samples)
                    (params-total-samples-to-fade-set! p total-samples)
                    ((foreign-lambda void "params_set_linear" 
                                     int int int float float f32vector)
                     pco ch n new-old x vec)
                    (params-updated?-set! p #t))))))

  (: params-should-fade? ((struct params) --> boolean))
  (define (params-should-fade? p) (> (params-total-samples-to-fade p) 0))

  (: params-after-fade-cleanup-new! ((struct params) -> noreturn))
  (define (params-after-fade-cleanup-new! p)
    (let* ((phases (params-phases p))
           (increments (params-increments p))
           (to-fade (params-samples-to-fade p))
           (len (f32vector-length phases))
           (total-to-fade (params-total-samples-to-fade p))
           (to-subtract ((foreign-lambda int "params_after_fade_cleanup_new"
                                         int f32vector f32vector u32vector)
                         len phases increments to-fade)))
      (params-total-samples-to-fade-set! p (- total-to-fade to-subtract))))

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
