#pragma once

#include <stdint.h>

#include "wave.h"

float applyVelocityCurve(const Wave *, const uint16_t);
unsigned int getNote(const uint16_t);
