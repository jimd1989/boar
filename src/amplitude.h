#pragma once

typedef struct Amplitude {

/* Amplitudes for master output, as well as left/right channel balance. */

  float L;
  float Master;
  float R;
} Amplitude;

Amplitude makeAmplitude(void);
void setBalance(Amplitude *, const float);
void setVolume(Amplitude *, const float);
