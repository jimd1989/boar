#pragma once

typedef struct Noise {

/* A unit for generating random signals. On every sampling instance, the Phase
 * is incremented by the Oscillator's pitch. If the Phase is greater than
 * DEFAULT_WAVELEN, Amplitude is set to a random value ∈ [-1,1] and the Phase
 * is reset. The Oscillator's buffer will be populated with the Amplitude value
 * until the Phase surpasses DEFAULT_WAVELEN again. */

  float       Amplitude;
  float       Phase;
} Noise;

float readNoise(Noise *, const float);
void makeNoise(Noise *);
