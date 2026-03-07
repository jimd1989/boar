(module boar-vectors
  (vector-foldl vector-for-each vector-map 
   u8vector-foldl u8vector-for-each u8vector-map
   s8vector-foldl s8vector-for-each s8vector-map
   u16vector-foldl u16vector-for-each u16vector-map
   s16vector-foldl s16vector-for-each s16vector-map
   u32vector-foldl u32vector-for-each u32vector-map
   s32vector-foldl s32vector-for-each s32vector-map
   u64vector-foldl u64vector-for-each u64vector-map
   s64vector-foldl s64vector-for-each s64vector-map
   f32vector-foldl f32vector-for-each f32vector-map
   f64vector-foldl f64vector-for-each f64vector-map)
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

  (: vector-for-each ((any -> noreturn) vector -> noreturn))
  (define (vector-for-each f xs)
    (vector-foldl (lambda (acc x n) (f x) (void)) #f xs))

  (: vector-map ((any -> any) vector -> vector))
  (define (vector-map f xs)
    (vector-foldl (lambda (acx x n) (vector-set! xs n (f x))) #f xs) xs)

  ; for-each was written like a map! change this!
  (define-syntax define-for-vector
    (er-macro-transformer
      (lambda (exp rename compare)
        (let* ((type (symbol->string (cadr exp)))
               (reff (string->symbol (string-append type "vector-ref")))
               (sett (string->symbol (string-append type "vector-set!")))
               (lenf (string->symbol (string-append type "vector-length")))
               (fl (string->symbol (string-append type "vector-foldl")))
               (fo (string->symbol (string-append type "vector-for-each")))
               (mp (string->symbol (string-append type "vector-map"))))
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
               (,fl (lambda (acc x n) (f x)) #f xs))
             (define (,mp f xs)
               (,fl (lambda (acc x n) (,sett xs n (f x))) 0 xs) xs))))))

  (: u8vector-foldl ((any fixnum fixnum -> any) any u8vector -> any))
  (: u8vector-for-each ((fixnum -> noreturn) u8vector -> noreturn))
  (: u8vector-map ((fixnum -> fixnum) u8vector -> u8vector))
  (define-for-vector u8)

  (: s8vector-foldl ((any fixnum fixnum -> any) any s8vector -> any))
  (: s8vector-for-each ((fixnum -> noreturn) s8vector -> noreturn))
  (: s8vector-map ((fixnum -> fixnum) s8vector -> s8vector))
  (define-for-vector s8)

  (: u16vector-foldl ((any fixnum fixnum -> any) any u16vector -> any))
  (: u16vector-for-each ((fixnum -> noreturn) u16vector -> noreturn))
  (: u16vector-map ((fixnum -> fixnum) u16vector -> u16vector))
  (define-for-vector u16)

  (: s16vector-foldl ((any fixnum fixnum -> any) any s16vector -> any))
  (: s16vector-for-each ((fixnum -> noreturn) s16vector -> noreturn))
  (: s16vector-map ((fixnum -> fixnum) s16vector -> s16vector))
  (define-for-vector s16)

  (: u32vector-foldl ((any fixnum fixnum -> any) any u32vector -> any))
  (: u32vector-for-each ((fixnum -> noreturn) u32vector -> noreturn))
  (: u32vector-map ((fixnum -> fixnum) u32vector -> u32vector))
  (define-for-vector u32)

  (: s32vector-foldl ((any fixnum fixnum -> any) any s32vector -> any))
  (: s32vector-for-each ((fixnum -> noreturn) s32vector -> noreturn))
  (: s32vector-map ((fixnum -> fixnum) s32vector -> s32vector))
  (define-for-vector s32)

  (: u64vector-foldl ((any fixnum fixnum -> any) any u64vector -> any))
  (: u64vector-for-each ((fixnum -> noreturn) u64vector -> noreturn))
  (: u64vector-map ((fixnum -> fixnum) u64vector -> u64vector))
  (define-for-vector u64)

  (: s64vector-foldl ((any fixnum fixnum -> any) any s64vector -> any))
  (: s64vector-for-each ((fixnum -> noreturn) s64vector -> noreturn))
  (: s64vector-map ((fixnum -> fixnum) s64vector -> s64vector))
  (define-for-vector s64)

  (: f32vector-foldl ((any float fixnum -> any) any f32vector -> any))
  (: f32vector-for-each ((float -> noreturn) f32vector -> noreturn))
  (: f32vector-map ((float -> float) f32vector -> f32vector))
  (define-for-vector f32)

  (: f64vector-foldl ((any float fixnum -> any) any f64vector -> any))
  (: f64vector-for-each ((float -> noreturn) f64vector -> noreturn))
  (: f64vector-map ((float -> float) f64vector -> f64vector))
  (define-for-vector f64)
)
