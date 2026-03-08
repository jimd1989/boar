#include <math.h>

#include "osc.h"

#define OSC_WAVETABLE_SIZE 4096
#define OSC_WAVETABLE_MASK (OSC_WAVETABLE_SIZE - 1)

void make_sine_wavetable(float *table) {
  int i           = 0;
  double sum      = 0.0;
  double avg      = 0.0;
  double phaseInc = ((2.0 * M_PI) / (double)OSC_WAVETABLE_SIZE);
  double phase    = 0.0;
  for (i = 0 ; i < OSC_WAVETABLE_SIZE ; i++, phase += phaseInc) {
    table[i] = sin(phase);
    sum     += table[i];
  }
  /* correct any DC offset */
  avg = sum / (float)OSC_WAVETABLE_SIZE;
  for (i = 0; i < OSC_WAVETABLE_SIZE ; i++) {
    table[i] -= avg;
  }
}

float osc_fill_slice(float *table, float freqInc, float phase,
                     float *slice, int written, int toWrite) {
  int i            = 0;
  float *buf       = &slice[written];
  int wavetableIdx = 0;
  float remainder  = 0.0f;
  for (i = 0 ; i < toWrite ; i++, phase += freqInc) {
    wavetableIdx = ((int)phase) & OSC_WAVETABLE_MASK;
    remainder    = phase - wavetableIdx; /* use for lerp */
    buf[i]       = table[wavetableIdx]; /* no lerp yet */
  }
  return fmodf(phase, (float)OSC_WAVETABLE_SIZE);
}
