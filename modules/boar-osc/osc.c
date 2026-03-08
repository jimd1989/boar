#include <math.h>

#include "osc.h"

void make_sine_wavetable(float *table, int tableSize) {
  int i           = 0;
  double sum      = 0.0;
  double avg      = 0.0;
  double phaseInc = ((2.0 * M_PI) / (double)tableSize);
  double phase    = 0.0;
  for (i = 0 ; i < tableSize ; i++, phase += phaseInc) {
    table[i] = sin(phase);
    sum     += table[i];
  }
  /* correct any DC offset */
  avg = sum / (float)tableSize;
  for (i = 0; i < tableSize ; i++) {
    table[i] -= avg;
  }
}
