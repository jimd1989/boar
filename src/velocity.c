#include <stdint.h>

#include "velocity.h"

#include "constants/maximums.h"
#include "numerical.h"
#include "wave.h"

float
applyVelocityCurve(const Wave *vw, const uint16_t note) {

/* Note is a set of two bytes. The lower byte is a value between 0 ... 127
 * representing the actual MIDI note, and the upper byte is a value between
 * 0 ... 127 representing the force with which the key was struck. This
 * function returns a value between 0.0 and 1.0 based upon the value of the
 * note's upper byte, which is used to modify the intensity of various
 * touch-sensitive parameters. Note is shifted to the right by 9 places
 * intead of the usual 8, since its value cannot exceed 127. */

  const float vel = (float)(note >> 9) / (float)MAX_MIDI_VALUE;

  return interpolateCycle(vw, truncateFloat(vel, 1.0f));
}
