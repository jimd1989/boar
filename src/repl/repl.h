#pragma once

#include <poll.h>
#include <stdbool.h>

#include "../audio/audio.h"
#include "../control/control.h"
#include "../parse/args.h"
#include "../parse/cmd.h"

#define REPL_LIMIT 4096

typedef struct Repl {
  bool            isRunning;
  int             nfds;
  CmdAlphabet     cmdAlphabet;  
  Control         control;
  Audio           audio;
  char            input[REPL_LIMIT];
  struct pollfd * pollFds;
} Repl;

void repl(Args);
