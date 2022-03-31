#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "audio-init.h"
#include "constants/defaults.h"
#include "parse.h"

typedef struct Repl {

/* A struct containing everything needed for a user-facing loop. During every
 * cycle of the REPL, user input is read with read() into Repl.Buffer. This
 * string is then parsed, and used to populate the fields of Repl.Cmd. An audio
 * function is then performed using Repl.Cmd as its argument. */

  bool          EchoNotes;
  Cmd           Cmd;
  char          Buffer[DEFAULT_LINESIZE];
  Audio       * Audio;
} Repl;

void repl(Repl *);
