#include <err.h>
#include <stdbool.h>
#include <string.h>

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
  char *end = strpbrk(&c->input[c->cmdPos], ";\n");
  if (end != NULL) { *end = '\0'; }
  warnx("Invalid input for: \"%s\"", &c->input[c->cmdPos]);
}
