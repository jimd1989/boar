/* Functions related to the Noise type. Consult "noise.h" for more info. */

#include <stdlib.h>

#include "noise.h"

#include "constants/defaults.h"
#include "math.h"

float
readNoise(Noise *n, const float pitch) {

/* Returns a random sample from a Noise unit. */

  n->Phase += pitch;
  if (n->Phase > DEFAULT_WAVELEN) {
    n->Phase = fmodf(n->Phase, (float)DEFAULT_WAVELEN);
    n->Amplitude = (((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f;
  }
  return n->Amplitude;
}

void
makeNoise(Noise *n) {

/* Initialize a noise generator. */

  n->Phase = 0.0f;
  n->Amplitude = 0.0f;
}
