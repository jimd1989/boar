#include <err.h>

#include "wave.h"

#include "errors.h"
#include "wavetable-exponential.h"
#include "wavetable-flat.h"
#include "wavetable-logarithmic.h"
#include "wavetable-ramp.h"
#include "wavetable-sine.h"
#include "wavetable-square.h"
#include "wavetable-triangle.h"

/* functions */
void
selectWave(Wave *w, WaveType wt) {

/* Assigns a WaveType and pointer to wavetable to a Wave. */

    switch((unsigned int)wt) {
        case WAVE_TYPE_SINE:
            w->Table = WAVE_SINE;
            break;
        case WAVE_TYPE_SQUARE:
            w->Table = WAVE_SQUARE;
            break;
        case WAVE_TYPE_TRIANGLE:
            w->Table = WAVE_TRIANGLE;
            break;
        case WAVE_TYPE_RAMP:
            w->Table = WAVE_RAMP;
            break;
        case WAVE_TYPE_EXPONENTIAL:
            w->Table = WAVE_EXPONENTIAL;
            break;
        case WAVE_TYPE_LOGARITHMIC:
            w->Table = WAVE_LOGARITHMIC;
            break;
        case WAVE_TYPE_NOISE:
            break;
        case WAVE_TYPE_FLAT:
            w->Table = WAVE_FLAT;
            break;
        default:
            warnx("Choose a wave between 0 and %d", WAVE_TYPE_FLAT);
            return;
    }
    w->Type = wt;
}
