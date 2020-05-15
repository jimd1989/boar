/*Functions for selecting wave types and reading wavetables. */

#include <err.h>
#include <stdlib.h>

#include "wave.h"

#include "defaults.h"
#include "errors.h"
#include "maximums.h"
#include "numerical.h"
#include "wavetables/exponential.h"
#include "wavetables/flat.h"
#include "wavetables/logarithmic.h"
#include "wavetables/saw.h"
#include "wavetables/sine.h"
#include "wavetables/square.h"
#include "wavetables/triangle.h"

/* functions */
void
selectWave(Wave *w, const int wt) {

/* Assigns a WaveType and pointer to wavetable to a Wave. Sets Wave.Polarity
 * according to the sign of "wt". */

    unsigned int uwt = abs(wt);

    switch(uwt) {
        case WAVE_TYPE_FLAT:
            w->Table = WAVE_FLATS[0];
            break;
        case WAVE_TYPE_SINE:
            w->Table = WAVE_SINES[0];
            break;
        case WAVE_TYPE_SQUARE:
            w->Table = WAVE_SQUARES[0];
            break;
        case WAVE_TYPE_TRIANGLE:
            w->Table = WAVE_TRIANGLES[0];
            break;
        case WAVE_TYPE_RAMP:
            w->Table = WAVE_SAWS[0];
            break;
        case WAVE_TYPE_EXPONENTIAL:
            w->Table = WAVE_EXPONENTIALS[0];
            break;
        case WAVE_TYPE_LOGARITHMIC:
            w->Table = WAVE_LOGARITHMICS[0];
            break;
        case WAVE_TYPE_NOISE:
            break;
        default:
            warnx("Choose a wave between 0 and %d", WAVE_TYPE_NOISE);
            return;
    }
    w->Type = uwt;
    if (wt < 0) {
        w->Polarity = -1.0f;
    } else {
        w->Polarity = 1.0f;
    }
}

float
interpolate(const Wave *w, const float phase) {

/* Performs a linear interpolation on the Wave's table in terms of phase. This
 * is the primary means of pitching tones in the program. */

    float r, s1, s2;
    int i = 0;
    
    if (w->Type == WAVE_TYPE_NOISE) {
        /* Generate white noise instead of wavetable lookup. */
        return (float)arc4random() / (float)UINT_MAX;
    }
    i = (int)phase;
    r = fabsf(phase) - abs(i);
    s1 = w->Table[(unsigned int)i % DEFAULT_WAVELEN];
    s2 = w->Table[((unsigned int)i+1) % DEFAULT_WAVELEN];
    return ((1.0f - r) * s1) + (r * s2);
}

float
interpolateCycle(const Wave *w, const float phase) {

/* Many wavetable referencing types, such as envelopes, apply a single cycle
 * of a wave to a value. This function performs an interpolation over a
 * wavetable using a phase between 0.0 and 1.0. */

    return unipolar(
            interpolate(w, phase * (float)MAX_WAVE_INDEX * w->Polarity));
}
