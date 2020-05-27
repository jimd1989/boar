#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "audio-settings.h"

typedef struct Buffer {

/* An audio buffer for internal mixing and synthesis. */

  size_t        Size;
  float       * Values;
} Buffer;

typedef struct ByteBuffer {

/* An audio buffer for output of mixed and interleaved sound data. */

  unsigned int          Chan;
  size_t                ChanBytes;
  size_t                Size;
  uint8_t             * Values;
} ByteBuffer;

Buffer * makeBuffer(const size_t);
ByteBuffer * makeByteBuffer(const AudioSettings *);
void killBuffer(Buffer *);
void killByteBuffer(ByteBuffer *);
