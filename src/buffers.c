/* Functions that initialize, fill, and clear buffer types. Initialization
 * errors are fatal. */

#include <err.h>
#include <limits.h>
#include <stdlib.h>

#include "buffers.h"

#include "audio-settings.h"
#include "constants/defaults.h"
#include "constants/errors.h"

Buffer *
makeBuffer(const size_t size) {

/* Allocates the space needed for a Buffer, then allocates the space needed for
 * Buffer.Values. */

  Buffer *b = malloc(sizeof(*b));
  if (b == NULL) {
    errx(ERROR_ALLOC, "Error initializing audio mixing buffer struct");
  }
  b->Values = malloc(sizeof(*b->Values) * size);
  if (b->Values == NULL) {
    errx(ERROR_ALLOC, "Error initializing audio mixing buffer");
  }
  b->Size = size;
  return b;
}

ByteBuffer *
makeByteBuffer(const AudioSettings *aos) {

/* Allocates the space needed for a ByteBuffer, then allocates the space needed
 * for ByteBuffer.Values. */

  ByteBuffer *bb = malloc(sizeof(*bb));
  if (bb == NULL) {
    errx(ERROR_ALLOC, "Error initializing audio output buffer struct");
  }
  bb->Values = malloc(sizeof(*bb->Values) * aos->BufsizeMain);
  if (bb->Values == NULL) {
    errx(ERROR_ALLOC, "Error initializing audio output buffer");
  }
  bb->ChanBytes = (size_t)DEFAULT_CHAN * 2;
  bb->Size = (size_t)aos->BufsizeMain;
  return bb;
}

void
killBuffer(Buffer *b) {

/* Frees all memory allocated by a Buffer. */

  free(b->Values);
  free(b);
}

void
killByteBuffer(ByteBuffer *bb) {

/* Frees all memory allocated by a ByteBuffer. */    

  free(bb->Values);
  free(bb);
}
