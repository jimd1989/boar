#pragma once

#include <poll.h>
#include <stdbool.h>

#include "../audio/audio.h"
#include "../parse/cmd.h"

#define REPL_LIMIT 4096

typedef struct Repl {
  bool            isRunning;
  int             nfds;
  CmdAlphabet     cmdAlphabet;  
  Audio           audio;
  char            input[REPL_LIMIT];
  struct pollfd * pollFds;
} Repl;

void repl(void);
