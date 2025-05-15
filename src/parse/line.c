#include "cmd.h"
#include "cursor.h"
#include "eval.h"
#include "line.h"

void parseLine(CmdAlphabet a, char *s) {
  Cursor c = cursor(s);
  while (c.breakReason != CURSOR_LINE_END) {
    c.cmdPos = c.pos;
    eatWhitespace(&c);
    if (c.input[c.pos] == '\n') { return; } /* empty input */
    parseCmd(&c, a);
    if (c.breakReason != CURSOR_WAITING_PARAMS) { return; }
    eatWhitespace(&c);
    eval(&c);
    if (c.breakReason == CURSOR_ERROR) { printCursorErr(&c); }
  }
}
