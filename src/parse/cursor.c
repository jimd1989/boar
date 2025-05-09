#include <err.h>
#include <stdbool.h>

#include "cursor.h"

Cursor cursor(char *input) {
  Cursor c = {0};
  c.pos = 0;
  c.input = input;
  return c;
}

CursorBreakReason breakReason(char c) {
  CursorBreakReason r = c == ' '  ? CURSOR_PARAMETER_END :
                        c == '\t' ? CURSOR_PARAMETER_END :
                        c == ';'  ? CURSOR_CMD_END       :
                        c == '\n' ? CURSOR_LINE_END      :
                        c == '\0' ? CURSOR_LINE_END      : CURSOR_ERROR;
  return r;
}

void eatWhitespace(Cursor *c) {
  while (CURSOR_HEAD(c) == ' ' || CURSOR_HEAD(c) == '\t') { c->pos++; }
}

void printCursorErr(Cursor *c) {
  warnx("Invalid input starting at %d: rest: %s", c->pos, &c->input[c->pos]);
}
