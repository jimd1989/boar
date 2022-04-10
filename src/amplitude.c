/* Functions for the Amplitude type, which provides independent
 * channel volumes on the master audio output. */

#include <math.h>

#include "amplitude.h"

#include "numerical.h"

Amplitude
makeAmplitude(void) {

/* Initialize an Amplitude struct with default channel volumes. */

  Amplitude a = {1.0f, 0.1f, 1.0f};
  return a;
}

void
setBalance(Amplitude *a, const float f) {

/* Sets the stereo balance of the output, where 0.5 represents full amplitude
 * on both channels, 0.0 represents full amplitude to the left, and 1.0
 * represents full amplitude to the right. */

  const float tf = truncateFloat(f, 1.0f);
  a->L = truncateFloat(1.0f - (2.0f * (tf - 0.5f)), 1.0f);
  a->R = truncateFloat(1.0f - (2.0f * (0.5f - tf)), 1.0f); 
}

void
setVolume(Amplitude *a, const float f) {

/* Sets master amplitude without getting too loud. */

  a->Master = unipolar(expCurve(truncateFloat(f, 1.0f)));
}

