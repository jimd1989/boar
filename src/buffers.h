#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "audio-settings.h"

typedef struct FloatBuffer {

/* An audio buffer for internal mixing and synthesis. */

  size_t        Size;
  float       * Values;
} FloatBuffer;

typedef struct ByteBuffer {

/* An audio buffer for output of mixed and interleaved sound data. */

  unsigned int          Chan;
  size_t                ChanBytes;
  size_t                Size;
  uint8_t             * Values;
} ByteBuffer;

FloatBuffer * makeBuffer(const size_t);
ByteBuffer * makeByteBuffer(const AudioSettings *);
void killBuffer(FloatBuffer *);
void killByteBuffer(ByteBuffer *);
