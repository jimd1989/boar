/* Functions that initialize, fill, and clear buffer types. Initialization
 * errors are fatal. */

#include <err.h>
#include <stdlib.h>

#include "buffers.h"

#include "constants/defaults.h"
#include "constants/errors.h"

Buffer
makeBuffer(const size_t size) {

/* Sets Buffer size constants, and allocates Output array. */

  Buffer b = {0};
  b.SizeFrames = size;
  b.SizeBytes = size * DEFAULT_CHAN * DEFAULT_BYTES;
  b.Output = malloc(sizeof(*b.Output) * b.SizeBytes);
  if (b.Output == NULL) {
    errx(ERROR_ALLOC, "Error initializing audio buffer");
  }
  return b;
}

void
killBuffer(Buffer *b) {

/* Free the Buffer's Output array allocation. */

  free(b->Output);
}
