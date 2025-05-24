#pragma once

#include <limits.h>
#include <poll.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdint.h>

/* sizes */
#define CC_LIMIT 128
#define MIDI_LIMIT 1024 /* Hardcoded in kernel, last I checked */
#define REPL_LIMIT 4096

/* fd indicies */
#define STDIN_IDX 0
#define MIDI_IDX 1

/* default CC */
typedef enum Cc {
  CC_VOL = 7
} Cc;

/* MIDI events (top 4 bits) */
#define GET_EVENT(X) ((X) & (15 << 4))
#define NOTE_OFF 128
#define NOTE_ON 144
#define POLY_AFTERTOUCH 160
#define CC 176
#define PROGRAM_CHANGE 192
#define CHAN_AFTERTOUCH 208
#define PITCH_BEND 224
#define NOT_EVENT(X) ((X) < 128)
#define SYSEX_START 240
#define SYSEX_END 247
#define MTC 241
#define SONG_POSITION 242
#define SONG_SELECT 243
#define TUNE_REQUEST 246
#define REAL_TIME_EVENT 248

/* Channel (bottom 4 bits) */
#define GET_CHAN(X) ((X) & 15)

typedef struct pollfd PollFd;
typedef struct mio_hdl * Mio;

typedef struct Repl {
  bool      isRunning;
  bool      inSysex;
  int       chan;
  int       nfds;
  Mio       mio;
  PollFd  * pollFds;
  char      ccs[CC_LIMIT][3]; /* 3 might be too small */
  uint8_t   midi[MIDI_LIMIT];
  char      input[REPL_LIMIT];
} Repl;
