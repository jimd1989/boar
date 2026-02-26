(import matrix-1000)
(midi-start! MIO-0)
(midi-write! MIO-0
  (fill-midi-buffer! 1 (note-on 72)
                       (note-on 76)
                       (note-on 79)
                       (vcf-cutoff 20)
                       (vcf-resonance 63)
                       (dco1-waveform 3)
                       (dco2-waveform 2)))
(midi-write! MIO-0
  (fill-midi-buffer! 1 (note-off 72)
                       (note-off 76)
                       (note-off 79)))

