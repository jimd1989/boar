#include <err.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "midi.h"

/* fd indicies */
#define STDIN_IDX 0
#define MIDI_IDX 1

#define EVENT_MASK 240
#define CHAN_MASK 15

/* MIDI mappings */
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

/* Wishlist:
 * - User can map any CC number to arbitrary string output */

static void repl(Repl *);
static void waitForIO(Repl *);
static void readMidi(Repl *);
static void parseMidi(Repl *, int);
static void readStdin(Repl *);
static void freeRepl(Repl *);

static void repl(Repl *r) {
  r->mio                       = mio_open(MIO_PORTANY, MIO_IN, true);
  if (r->mio == NULL) { errx(1, "Error opening MIDI input %s", MIO_PORTANY); }
  r->chan                      = 0;
  r->nfds                      = 1 + mio_nfds(r->mio);
  r->pollFds                   = calloc(r->nfds, sizeof(*r->pollFds));
  r->pollFds[STDIN_IDX].fd     = STDIN_FILENO;
  r->pollFds[STDIN_IDX].events = POLLIN;
  r->isRunning                 = true;
}

static void waitForIO(Repl *r) {
  int nfds = 0;
  /* No need to reset this every loop, but recommended */
  mio_pollfd(r->mio, &r->pollFds[MIDI_IDX], POLLIN);
  nfds = poll(r->pollFds, r->nfds, -1);
  if (nfds < 1) { errx(1, "Polling failure in main REPL"); }
}

static void readMidi(Repl *r) {
  int bytesRead = 0;
  if (!(r->pollFds[MIDI_IDX].revents & POLLIN)) { return; }
  bytesRead = mio_read(r->mio, r->midi, MIDI_LIMIT);
  parseMidi(r, bytesRead);
}

static void parseMidi(Repl *r, int bytesRead) {
  int i = 0;
  char *buf = r->midi;
  for (; i < bytesRead ; i++) {
    warnx("%d", *buf);
    buf++;
  }
}

static void readStdin(Repl *r) {
  if (!(r->pollFds[STDIN_IDX].revents & POLLIN)) { return; }
  if (fgets(r->input, REPL_LIMIT, stdin) == NULL) { 
    r->isRunning = false;
    return;
  }
  warnx("%s", r->input);
}

static void freeRepl(Repl *r) {
  free(r->pollFds);
}

int main(void) {
  Repl r = {0};
  repl(&r);
  while (r.isRunning) {
    waitForIO(&r);
    readMidi(&r);
    readStdin(&r);
  }
  freeRepl(&r);
  return 0;
}
