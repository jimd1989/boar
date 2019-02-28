#include <stdint.h>

#include "velocity.h"

#include "maximums.h"
#include "numerical.h"
#include "wave.h"

/* functions */
float
getVelocity(const uint16_t note) {

/* Note is a set of two bytes. The lower byte is a value between 0 ... 127
 * representing the actual MIDI note, and the upper byte is a value between
 * 0 ... 127 representing the force with which the key was struck. This
 * function returns a value between 0.0 and 1.0 based upon the value of the
 * note's upper byte, which is used to modify the intensity of various
 * touch-sensitive parameters. */

    return (float)(note >> 8) / (float)MAX_MIDI_VALUE;
}

void
setSensitivity(Velocity *v, const float vel) {

/* Set Velocity.Sensitivity based upon a truncated lookup to the velocity
 * curve wavetable. */

    const unsigned int i = (unsigned int)(truncateFloat(vel, 1.0f) *
            (float)MAX_WAVE_INDEX);
    
    v->Sensitivity = unipolar(v->Wave->Table[i]);
}

unsigned int
getNote(const uint16_t note) {

/* Returns the lower byte of note, indicating which MIDI note to play. */

    return note & MAX_MIDI_VALUE;
}
