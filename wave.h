#pragma once

/* types */
typedef enum WaveType {

/* The type of a wave. */

    WAVE_TYPE_SINE = 0,
    WAVE_TYPE_SQUARE,
    WAVE_TYPE_TRIANGLE,
    WAVE_TYPE_RAMP,
    WAVE_TYPE_EXPONENTIAL,
    WAVE_TYPE_LOGARITHMIC,
    WAVE_TYPE_NOISE,
    WAVE_TYPE_FLAT
} WaveType;

typedef struct Wave {

/* A wrapper around the actual wavetable with type information. */

    WaveType      Type;
    const float * Table;
} Wave;

/* headers */
void selectWave(Wave *, const WaveType);
