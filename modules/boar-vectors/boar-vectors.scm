(module boar-vectors
  (vector-foldl vector-for-each u8vector-foldl u8vector-for-each 
   s8vector-foldl s8vector-for-each u16vector-foldl u16vector-for-each 
   s16vector-foldl s16vector-for-each u32vector-foldl u32vector-for-each 
   s32vector-foldl s32vector-for-each u64vector-foldl u64vector-for-each 
   s64vector-foldl s64vector-for-each f32vector-foldl f32vector-for-each 
   f64vector-foldl f64vector-for-each)
  (import scheme (chicken base) (chicken type) srfi-4)

  (: vector-foldl ((any any fixnum -> any) any vector -> any))
  (define (vector-foldl f iacc xs)
    (letrec ((len (vector-length xs))
             (loop (lambda (acc n)
                     (if (< n len)
                       (let* ((x (vector-ref xs n))
                              (nacc (f acc x n)))
                         (loop nacc (+ n 1)))
                       acc))))
      (loop iacc 0)))

  (: vector-for-each ((any -> any) vector -> vector))
  (define (vector-for-each f xs)
    (vector-foldl (lambda (acc x n) (vector-set! xs n (f x))) #f xs) xs)

  (define-syntax define-for-vector
    (er-macro-transformer
      (lambda (exp rename compare)
        (let* ((type (symbol->string (cadr exp)))
               (reff (string->symbol (string-append type "vector-ref")))
               (sett (string->symbol (string-append type "vector-set!")))
               (lenf (string->symbol (string-append type "vector-length")))
               (fl (string->symbol (string-append type "vector-foldl")))
               (fo (string->symbol (string-append type "vector-for-each"))))
          `(begin
             (define (,fl f iacc xs)
               (letrec ((len (,lenf xs))
                        (loop (lambda (acc n)
                                (if (< n len)
                                  (let* ((s (,reff xs n))
                                         (nacc (f acc s n)))
                                    (loop nacc (+ n 1)))
                                  acc))))
                 (loop iacc 0)))
             (define (,fo f xs)
               (,fl (lambda (acc x n) (,sett xs n (f x))) #f xs) xs))))))

  (: u8vector-foldl ((any fixnum fixnum -> any) any u8vector -> any))
  (: u8vector-for-each ((fixnum -> fixnum) u8vector -> u8vector))
  (define-for-vector u8)

  (: s8vector-foldl ((any fixnum fixnum -> any) any s8vector -> any))
  (: s8vector-for-each ((fixnum -> fixnum) s8vector -> s8vector))
  (define-for-vector s8)

  (: u16vector-foldl ((any fixnum fixnum -> any) any u16vector -> any))
  (: u16vector-for-each ((fixnum -> fixnum) u16vector -> u16vector))
  (define-for-vector u16)

  (: s16vector-foldl ((any fixnum fixnum -> any) any s16vector -> any))
  (: s16vector-for-each ((fixnum -> fixnum) s16vector -> s16vector))
  (define-for-vector s16)

  (: u32vector-foldl ((any fixnum fixnum -> any) any u32vector -> any))
  (: u32vector-for-each ((fixnum -> fixnum) u32vector -> u32vector))
  (define-for-vector u32)

  (: s32vector-foldl ((any fixnum fixnum -> any) any s32vector -> any))
  (: s32vector-for-each ((fixnum -> fixnum) s32vector -> s32vector))
  (define-for-vector s32)

  (: u64vector-foldl ((any fixnum fixnum -> any) any u64vector -> any))
  (: u64vector-for-each ((fixnum -> fixnum) u64vector -> u64vector))
  (define-for-vector u64)

  (: s64vector-foldl ((any fixnum fixnum -> any) any s64vector -> any))
  (: s64vector-for-each ((fixnum -> fixnum) s64vector -> s64vector))
  (define-for-vector s64)

  (: f32vector-foldl ((any float fixnum -> any) any f32vector -> any))
  (: f32vector-for-each ((float -> float) f32vector -> f32vector))
  (define-for-vector f32)

  (: f64vector-foldl ((any float fixnum -> any) any f64vector -> any))
  (: f64vector-for-each ((float -> float) f64vector -> f64vector))
  (define-for-vector f64)
)
