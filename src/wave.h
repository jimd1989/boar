#pragma once

/* types */
typedef enum WaveType {

/* The type of a wave. */

    WAVE_TYPE_FLAT = 0,
    WAVE_TYPE_SINE,
    WAVE_TYPE_SQUARE,
    WAVE_TYPE_TRIANGLE,
    WAVE_TYPE_SAW,
    WAVE_TYPE_EXPONENTIAL,
    WAVE_TYPE_LOGARITHMIC,
    WAVE_TYPE_NOISE
} WaveType;

typedef struct Wave {

/* A wrapper around the actual wavetable with type information. Wave. Polarity
 * marks the direction a wavetable should be read in. */

    float          Polarity;
    WaveType       Type;
    const float ** Table;
} Wave;

/* headers */
void selectWave(Wave *, const int);
float interpolate(const Wave *, const float *, const float);
float interpolateCycle(const Wave *, const float);
