#pragma once

typedef union {
  int     n;
  float   f;
  char    c;
  char  * s;
} CursorValue;

typedef enum {
  CURSOR_ERROR          = 0,
  CURSOR_PARAMETER_END  = 1,
  CURSOR_CMD_END        = 2,
  CURSOR_LINE_END       = 3,
  CURSOR_WAITING_PARAMS = 4
} CursorBreakReason;

typedef struct Cursor {
  int                 pos;
  char              * input;
  CursorBreakReason   breakReason;
  CursorValue         val;
} Cursor;

#define CURSOR_HEAD(X) (X->input[X->pos])

Cursor cursor(char *);
CursorBreakReason breakReason(char);
void eatWhitespace(Cursor *);
void printCursorErr(Cursor *);
