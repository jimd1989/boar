#include <err.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../audio/audio.h"
#include "../audio/sndio.h"
#include "../parse/line.h"
#include "repl.h"

#define STDIN_IDX 0
#define SNDIO_IDX 1

static void startRepl(Repl *);
static void waitForIO(Repl *);
static void readInput(Repl *);
static void playAudio(Repl *);
static void stopRepl(Repl *);

static void startRepl(Repl *r) {
  cmdAlphabet(&r->cmdAlphabet);
  audio(&r->audio);
  r->nfds = r->audio.sio.nfds + 1;
  r->pollFds = malloc(r->nfds * sizeof(*r->pollFds));
  r->pollFds[STDIN_IDX].fd = STDIN_FILENO;
  r->pollFds[STDIN_IDX].events = POLLIN;
  r->isRunning = true;
}

static void stopRepl(Repl *r) {
  stopAudio(&r->audio);
  free(r->pollFds);
}

static void waitForIO(Repl *r) {
  int nfds = 0;
  /* No need to reset this every loop, but recommended */
  sio_pollfd(r->audio.sio.port, &r->pollFds[SNDIO_IDX], POLLOUT);
  nfds = poll(r->pollFds, r->nfds, -1);
  if (nfds < 1) { errx(1, "Polling failure in main REPL"); }
}

static void readInput(Repl *r) {
  if (!(r->pollFds[STDIN_IDX].revents & POLLIN)) { return; }
  r->pollFds[STDIN_IDX].revents = 0;
  if (fgets(r->input, REPL_LIMIT, stdin) == NULL) { 
    r->isRunning = false;
    return;
  }
  parseLine(r->cmdAlphabet, r->input);
}

static void playAudio(Repl *r) {
  if(!(sio_revents(r->audio.sio.port, &r->pollFds[SNDIO_IDX]) & POLLOUT)) {
    return;
  }
  r->pollFds[SNDIO_IDX].revents = 0;
  writeAudio(&r->audio);
}

void repl(void) {
  Repl r = {0};
  startRepl(&r);
  while (r.isRunning) {
    waitForIO(&r);
    readInput(&r);
    playAudio(&r);
  }
  stopRepl(&r);
}
