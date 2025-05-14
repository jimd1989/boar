#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../audio/audio.h"
#include "../parse/cmd.h"
#include "../parse/cursor.h"
#include "../parse/eval.h"
#include "repl.h"

static void initRepl(Repl *);
static void stopRepl(Repl *);

static void initRepl(Repl *r) {
  cmdAlphabet(&r->cmdAlphabet);
  audio(&r->audio);
  r->pollFds = malloc(r->audio.sio.nfds * sizeof(*r->pollFds));
  r->pollFds[0].fd = STDIN_FILENO;
  r->pollFds[0].events = POLLIN;
}

static void stopRepl(Repl *r) {
  free(r->pollFds);
  stopAudio(&r->audio);
}

void repl(void) {
  Repl r   = {0};
  Cursor c = cursor(r.input);
  initRepl(&r);
  /* TODO
   * run pollSio()
   * poll on all fds()
   * check std and sio after poll
   */
  while (poll(r.pollFds, 1, 0) != -1) {
    if (r.pollFds[0].revents & POLLIN) {
      if (fgets(r.input, REPL_LIMIT, stdin) != NULL) {
        c = cursor(r.input);
        while(c.breakReason != CURSOR_LINE_END) {
          c.cmdPos = c.pos;
          eatWhitespace(&c);
          if (c.input[c.pos] == '\n') { break; } /* empty input */
          parseCmd(&c, r.cmdAlphabet);
          if (c.breakReason != CURSOR_WAITING_PARAMS) { break; }
          eatWhitespace(&c);
          eval(&c);
          if (c.breakReason == CURSOR_ERROR) { printCursorErr(&c); break; }
        }
      } else {
        stopRepl(&r);
        return; /* EOF */
      }
    }
  }
}
