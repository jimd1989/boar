#pragma once

#include <stdbool.h>

#include "audio-settings.h"
#include "buffers.h"
#include "voice.h"

typedef struct Audio {

/* During an audio playback cycle, all synthesis takes place in Audio.Voices.
 * The sample values of individual Voices are summed up in Audio.MixingBuffer.
 * The samples in the MixingBuffer are multiplied against the master volume
 * specified by Audio.Amplitude, then broken down into individual bytes and
 * written to Audio.MainBuffer, which is finally converted to sound by
 * Audio.Output. */

  float                   Amplitude;
  Bufferx                 Buffer;
  Buffer                * MixingBuffer;
  ByteBuffer            * MainBuffer;
  struct sio_hdl        * Output;
  AudioSettings           Settings;
  Voices                  Voices;
} Audio;

void makeAudio(Audio *, const int, char **);
void killAudio(Audio *);
