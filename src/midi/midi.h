#pragma once

#include <poll.h>
#include <sndio.h>
#include <stdbool.h>

#define MIDI_LIMIT 1024 /* Hardcoded in kernel, last I checked */
#define REPL_LIMIT 4096

typedef struct pollfd PollFd;
typedef struct mio_hdl * Mio;

typedef struct Repl {
  bool      isRunning;
  int       chan;
  int       nfds;
  Mio       mio;
  PollFd  * pollFds;
  char      midi[MIDI_LIMIT];
  char      input[REPL_LIMIT];
} Repl;
