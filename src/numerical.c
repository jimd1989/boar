/* Defines miscellaneous numerical functions for use in multiple locations 
 * throughout the program. The procedures here should ideally be terse and 
 * referentially transparent, since they are decoupled from the  types, enums, 
 * etc. in other files that give functions proper context. */

#include <stdbool.h>
#include <stdlib.h>

#include "numerical.h"

#include "maximums.h"
#include "wavetables/exponential.h"

/* functions */
float
truncateFloat(const float f, const float max) {

/* Truncates the value of f in terms of max. */

  const bool over = (f > max);

  return ((float)over * max) + ((float)(! over) * f);
}

float
liftFloat(const float f, const float min) {

/* Ensures float f doesn't fall below min */

  const bool under = (f < min);

  return ((float)under * min) + ((float)(! under) * f);
}

float
clip(const float s) {

/* Clips a sample so that it is neither higher than 1.0 nor lower than -1.0.
 * This could potentially be re-written as a macro, but is preserved as a
 * function for readability's sake. */

    if (s > 1.0f) {
        return 1.0f;
    } else if (s < -1.0f) {
        return -1.0f;
    } else {
        return s;
    }
}

float
expCurve(const float f) {

/* Takes a float value between 0.0 and 1.0 and returns another value between
 * -1.0 and 1.0 from a wavetable representing an exponential function. This
 * value must be made unipolar again before it is of any use to envelopes.
 * The curve is applied to diminish the presence of extreme values rarely used
 * as arguments to envelopes. Interpolation is foresaken for basic truncation, 
 * since the wavetable itself already provides a far superior resolution to
 * traditional synthesis parameters, which are usually between 0 and 127. */

    return WAVE_EXPONENTIALS[0][(unsigned int)(f * (DEFAULT_WAVELEN - 1))];
}

float
unipolar(const float s) {

/* All wavetables in the program are bipolar (-1.0 ... 1.0). This function
 * takes a sample from one of these tables and converts it to its unipolar
 * (MIN_FLOAT ... 1.0) equivalent. This allows normal wavetables to have other
 * applications, such as stages in ASDR envelopes. */

    const float f = 0.5f + (s / 2.0f);
    
    if (f == 0.0f) {
    /* Envelopes shouldn't take zero values unless explicitly specified
     * by the user in the prompt. */
        return MIN_FLOAT;
    }
    return f;
}

float
interpolate(const float *table, const int len, const float phase) {

/* Where phase ∈ [0, len), find an index of table by truncating phase to an
 * integer. Then return a weighted average of the values between table[phase]
 * and table[phase+1] based upon the fractional part of the phase. The phase
 * must be modulo'd against len before invoking this function, as it will not
 * bounds check itself. */
 
    const int i = (int)phase;
    const float r = fabsf(phase) - abs(i);
    const float s1 = table[(unsigned int)i % len];
    const float s2 = table[((unsigned int)i+1) % len];

    return ((1.0f - r) * s1) + (r * s2);
}
