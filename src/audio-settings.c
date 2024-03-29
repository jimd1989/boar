/* Functions for parsing command-line flags and assigning them to the 
 * AudioSettings type, which serves as a read-only record of playback
 * information. */

#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "audio-settings.h"

#include "constants/defaults.h"
#include "constants/errors.h"
#include "constants/maximums.h"

static bool isFlag(const char *, const char *);
static void parseFlag(const char *, const char *, const unsigned int, 
    const unsigned int, unsigned int *);

static bool
isFlag(const char *arg, const char *flag) {

/* Returns a boolean value indicating if arg matches flag. */

  const size_t l1 = strnlen(arg, MAX_FLAG_LEN);
  const size_t l2 = strnlen(flag, MAX_FLAG_LEN);

  if (l1 != l2) {
    return false;
  }
  return ! strncmp(arg, flag, l1);
}

static void
parseFlag(const char *flag, const char *val, const unsigned int min, 
    const unsigned int max, unsigned int *field) {

/* Changes flag's value to integer and assigns it to field. */

  *field = strtonum(val, min, max, NULL);
  if (! *field) {
    errx(ERROR_ARG, "invalid value for %s", flag);
  }
}

void
makeAudioSettings(AudioSettings *aos, const int argc, char **argv) {

/* Parses user flags into AudioSettings struct. Errors are fatal. */

  int i = 1;
  char *arg = NULL;

  aos->Bits = DEFAULT_BITS;
  aos->BufBlocks = DEFAULT_BUF_BLOCKS;
  aos->BufSizeFrames = DEFAULT_BUFSIZE;
  aos->Rate = DEFAULT_RATE;
  aos->Polyphony = DEFAULT_POLYPHONY;
  for (; i < argc ; i++) {
    arg = argv[i];
    if (isFlag(arg, "-rate") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_RATE, &aos->Rate);
    } else if (isFlag(arg, "-polyphony") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_POLYPHONY, &aos->Polyphony);
    } else if (isFlag(arg, "-blocks") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_BUF_BLOCKS, &aos->BufBlocks);
    } else {
      errx(ERROR_ARG, "Malformed parameter: %s", arg);
    } 
  }
}
