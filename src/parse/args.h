#pragma once

#include "cursor.h"

/* User-passed command line arguments. Primitive types like numbers are parsed,
 * but more complex args like files are not opened until they are needed. */
typedef struct Args {
  int     chan;
  float   maxEnvTime;
  char  * audioOutputFile;
  char  * infoOutputFile;
} Args;

void args(Args *);
Cursor parseArgs(int, char **, Args *);
