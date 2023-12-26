#pragma once

#include "constants/errors.h"

typedef struct AudioSettings {

/* Contains all the constant settings that govern audio playback. Some
 * fields in this struct do not have to be referenced after initializing the
 * handle to the soundcard, but they are kept here in case future iterations
 * of this code need to make use of any of them. Other structs might contain
 * pointers to or local copies of these read-only values. */

  unsigned int  Bits;
  unsigned int  BufSizeFrames;
  unsigned int  BufBlocks;
  unsigned int  Rate;
  unsigned int  Polyphony;
} AudioSettings;

void makeAudioSettings(AudioSettings *, const int, char **);
