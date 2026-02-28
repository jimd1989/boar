(module boar-midi
  (note-on note-off note-offf fill-midi-buffer!)
  (import scheme (chicken base) (chicken string) (chicken type) srfi-4)
  (import boar-slice)

  (define-type slice (list u8vector fixnum fixnum))
  (define-type slice-f (fixnum any slice -> slice))

  (: note-on (fixnum #!optional fixnum -> slice-f))
  (define (note-on n #!optional (vel 127))
    (lambda (ch x sl)
      (let ((sll (extend-slice 3 sl))
            (chh (+ ch -1 144)))
        (slice-fill! sll chh n vel))))

  (: note-off (fixnum #!optional fixnum -> slice-f))
  (define (note-off n #!optional (vel 127))
    (lambda (ch x sl)
      (let ((sll (extend-slice 3 sl))
            (chh (+ ch -1 128)))
        (slice-fill! sll chh n vel))))

  (: note-offf (fixnum -> slice-f))
  (define (note-offf n) (note-on n 0))

  (: fill-midi-buffer! (fixnum any u8vector -> slice))
  (define (fill-midi-buffer! ch x u8 . fs)
    (foldl (lambda (sl f) (f ch x sl)) (make-slice 0 u8) fs))
)
