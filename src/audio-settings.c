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
  aos->Bufsize = DEFAULT_BUFSIZE;
  aos->Chan = DEFAULT_CHAN;
  aos->Rate = DEFAULT_RATE;
  aos->Polyphony = DEFAULT_POLYPHONY;
  for (; i < argc ; i++) {
    arg = argv[i];
    if (isFlag(arg, "-bits") && i+1 < argc) {
      parseFlag(arg, argv[++i], MIN_BITS, MAX_BITS, &aos->Bits);
      if (aos->Bits % 8 != 0) {
        errx(ERROR_ARG, "-bits must be a multiple of 8");
      }
    } else if (isFlag(arg, "-bufsize") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_BUFSIZE, &aos->Bufsize);
    } else if (isFlag(arg, "-chan") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_CHAN, &aos->Chan);
    } else if (isFlag(arg, "-rate") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_RATE, &aos->Rate);
    } else if (isFlag(arg, "-polyphony") && i+1 < argc) {
      parseFlag(arg, argv[++i], 1, MAX_POLYPHONY, &aos->Polyphony);
    } else if (isFlag(arg, "-echo-notes")) {
      aos->EchoNotes = true;
    } else {
      errx(ERROR_ARG, "Malformed parameter: %s", arg);
    } 
  }
}
