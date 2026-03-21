(module boar-slice
  (growable-u8slice f32slice f32slice-vector f32slice-written f32slice-to-write 
   f32slice-copy-to-f32vector f32slice-copy-from-f32vector 
   f32slice-copy-to-f32slice f32vector->slice
   make-slice slice-u8vector slice-bytes-written slice-bytes-to-write 
   slice-end-pos extend-slice slice-ref slice-set! slice-fill!) 
  (import scheme (chicken base) (chicken foreign) (chicken memory) 
          (chicken string) (chicken type) srfi-4 typed-records)

  (foreign-declare "#include \"slice.h\"")

  (define-type slice (list u8vector fixnum fixnum))

  (define-record f32slice
    (vector : f32vector)
    (written : fixnum)
    (to-write : fixnum))

  (: f32slice-copy-to-f32vector ((struct f32slice) f32vector -> noreturn))
  (define (f32slice-copy-to-f32vector sl f32)
    (if (<= (f32slice-to-write sl) (f32vector-length f32))
      ((foreign-safe-lambda void "f32slice_copy_to_f32vector"
                       f32vector int int f32vector)
       (f32slice-vector sl)
       (f32slice-written sl)
       (f32slice-to-write sl)
       f32)))

  (: f32slice-copy-from-f32vector ((struct f32slice) f32vector -> noreturn))
  (define (f32slice-copy-from-f32vector sl f32)
    (if (<= (f32vector-length f32) (f32slice-to-write sl))
      ((foreign-safe-lambda void "f32slice_copy_from_f32vector"
                       f32vector int int f32vector)
       (f32slice-vector sl)
       (f32slice-written sl)
       (f32slice-to-write sl)
       f32)))

  (: f32slice-copy-to-f32slice 
     ((struct f32slice) (struct f32slice) -> noreturn))
  (define (f32slice-copy-to-f32slice src-sl dest-sl)
    (if (<= (f32slice-to-write src-sl) (f32slice-to-write dest-sl))
      ((foreign-safe-lambda void "f32slice_copy_to_f32slice"
        f32vector int int f32vector int)
       (f32slice-vector src-sl)
       (f32slice-written src-sl)
       (f32slice-to-write src-sl)
       (f32slice-vector dest-sl)
       (f32slice-written dest-sl))))

  (: f32vector->slice (f32vector fixnum fixnum --> (struct f32slice)))
  (define (f32vector->slice f32 n m)
    (if (> (+ n m) (f32vector-length f32))
      (error (conc (+ n m) " beyond length " (f32vector-length f32)))
      (make-f32slice f32 n m)))

  ; eventually rewrite "slice" functions to use this
  (define-record growable-u8slice
    (vector : u8vector)
    (written : fixnum)
    (to-write : fixnum))

  (: extend-vector (u8vector -> u8vector))
  (define (extend-vector u8)
    (let* ((old-len (u8vector-length u8))
           (nu8 (make-u8vector (* 2 old-len) 0 #t #f)))
      (move-memory! u8 nu8 old-len)
      (release-number-vector u8)
      nu8))

  (: make-slice (fixnum u8vector --> slice))
  (define (make-slice bytes-to-write u8)
    (if (> bytes-to-write (u8vector-length u8))
      (make-slice bytes-to-write (extend-vector u8))
      (list u8 0 bytes-to-write)))
  
  (: slice-u8vector (slice --> u8vector))
  (define (slice-u8vector sl) (car sl))

  (: slice-bytes-written (slice --> fixnum))
  (define (slice-bytes-written sl) (cadr sl))

  (: slice-bytes-to-write (slice --> fixnum))
  (define (slice-bytes-to-write sl) (caddr sl))

  (: slice-end-pos (slice --> fixnum))
  (define (slice-end-pos sl)
    (+ (slice-bytes-written sl) (slice-bytes-to-write sl)))

  (: extend-slice (fixnum slice --> slice))
  (define (extend-slice bytes-to-write sl)
    (let* ((u8 (car sl))
           (bytes-written (+ (slice-bytes-written sl)
                             (slice-bytes-to-write sl)))
           (end-pos (+ bytes-written bytes-to-write)))
      (if (> end-pos (u8vector-length u8))
        (extend-slice bytes-to-write `(,(extend-vector u8) ,@(cdr sl)))
        (list u8 bytes-written bytes-to-write))))
  
  (: slice-ref (fixnum slice --> fixnum))
  (define (slice-ref n sl)
    (let* ((u8 (car sl))
           (bytes-written (slice-bytes-written sl))
           (bytes-to-write (slice-bytes-to-write sl))
           (m (+ bytes-written n)))
      (if (>= n bytes-to-write)
        (error (conc "slice len: " bytes-to-write " index: " n))
        (u8vector-ref u8 m))))

  (: slice-set! (fixnum fixnum slice -> noreturn))
  (define (slice-set! n x sl)
    (let* ((u8 (car sl))
           (bytes-written (slice-bytes-written sl))
           (bytes-to-write (slice-bytes-to-write sl))
           (m (+ bytes-written n)))
      (if (>= n bytes-to-write)
        (error (conc "slice len: " bytes-to-write " index: " n))
        (u8vector-set! u8 m x))))

  (: slice-fill! (slice (list-of fixnum) -> slice))
  (define (slice-fill! sl . xs)
    (let ((len (length xs))
          (u8 (car sl))
          (bytes-written (slice-bytes-written sl))
          (bytes-to-write (slice-bytes-to-write sl)))
      (if (> len bytes-to-write)
        (error (conc "slice len: " bytes-to-write " bytes: " len))
        (begin
          (foldl
            (lambda (n x) (u8vector-set! u8 n x) (+ n 1)) bytes-written xs)
          sl))))
)
