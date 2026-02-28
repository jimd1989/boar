(module boar-slice
  (make-slice slice-u8vector slice-bytes-written slice-bytes-to-write 
   slice-end-pos extend-slice slice-ref slice-set! slice-fill!) 
  (import scheme (chicken base) (chicken string) (chicken type) srfi-4)

  (define-type slice (list u8vector fixnum fixnum))

  (: make-slice (fixnum u8vector --> slice))
  (define (make-slice bytes-to-write u8)
    (if (> bytes-to-write (u8vector-length u8))
      (error (conc "not enough free bytes to create slice of: " bytes-to-write))
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
        (error (conc "not enough free bytes to extend slice to: " end-pos))
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
