#pragma once

#include <stdint.h>

#include "../audio/sample.h"
#include "../chunk/chunk.h"
#include "../noise/noise.h"
#include "../voices/voice.h"

#define MIXER_SIZE 16

typedef struct Mixer {
  int   chan;
  float volumes[MIXER_SIZE];
} Mixer;

void mixer(Mixer *, int);

/* WIP */
/* - An individual Voice needs to update its bit of activeZones.
 * - When time comes to render, derive 8 item index array from activeZones.
 * - -1 = NIL
 * - Volumes should be grouped by channel? ie 8 lefts, 8 rights, etc. */
typedef struct NewMixer {
  int           chan;
  uint32_t      activeZones;
  Noise         noise;
  AudioSample   audio[VOICES_SIZE][AUDIO_CHUNK_SIZE];
  float       * volumes;
} NewMixer;
