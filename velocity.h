#pragma once

#include <stdint.h>

#include "wave.h"

/* types */
typedef struct Velocity {

/* Velocity.Wave contains a velocity sensitivity curve. It points to a Wave
 * determined in the parent Operators type. Velocity.Sensitivity is derived
 * from lookups to this Wave's table. */

    float         Sensitivity;
    Wave        * Wave;
} Velocity;

/* headers */
float getVelocity(const uint16_t);
void setSensitivity(Velocity *, const float);
unsigned int getNote(const uint16_t);
