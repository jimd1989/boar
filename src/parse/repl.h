#pragma once

#include "cmd.h"

#define REPL_LIMIT 4096

typedef struct Repl {
  CmdAlphabet cmdAlphabet;  
  char        input[REPL_LIMIT];
} Repl;

void repl(void);
