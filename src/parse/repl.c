#include <stdio.h>

#include "cmd.h"
#include "cursor.h"
#include "repl.h"

static void initRepl(Repl *);

static void initRepl(Repl *r) {
  cmdAlphabet(&r->cmdAlphabet);
  /* audio init here */
}

void repl(void) {
  Repl r   = {0};
  Cursor c = cursor(r.input);
  initRepl(&r);
  while (1) {
   if (fgets(r.input, REPL_LIMIT, stdin) != NULL) {
     c = cursor(r.input);
     while(c.breakReason != CURSOR_LINE_END) {
       eatWhitespace(&c);
       if (c.input[c.pos] == '\n') { break; }
       parseCmd(&c, r.cmdAlphabet);
       if (c.breakReason != CURSOR_WAITING_PARAMS) { break; }
       /* Eval */
     }
   } else {
     return; /* EOF */
   }
  }
}
