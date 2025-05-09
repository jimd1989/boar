#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include "cmd.h"
#include "cursor.h"
#include "eval.h"
#include "repl.h"

static void initRepl(Repl *);

static void initRepl(Repl *r) {
  cmdAlphabet(&r->cmdAlphabet);
  /* audio init here */
}

void repl(void) {
  Repl r                = {0};
  Cursor c              = cursor(r.input);
  struct pollfd pfds[1] = {{0}};
  pfds[0].fd     = STDIN_FILENO;
  pfds[0].events = POLLIN;
  initRepl(&r);
  while (poll(pfds, 1, 0) != -1) {
    if (pfds[0].revents & POLLIN) {
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
        return; /* EOF */
      }
    }
  }
}
