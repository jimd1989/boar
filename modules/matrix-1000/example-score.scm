(import matrix-1000)
(midi-start! MIO-0)
(midi-write! MIO-0
  (fill-midi-buffer! 1 (note-on 72)
                       (note-on 76)
                       (note-on 79)
                       (vcf-cutoff 45)))
(midi-write! MIO-0
  (fill-midi-buffer! 1 (note-off 72)
                       (note-off 76)
                       (note-off 79)))

