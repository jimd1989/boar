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
  int                 cmdPos;
  char              * input;
  CursorBreakReason   breakReason;
  CursorValue         val;
} Cursor;

#define CURSOR_HEAD(X) (X->input[X->pos])
#define CURSOR_PARSE(F, X) \
  do { eatWhitespace((X)); F((X)); if((X)->breakReason == CURSOR_ERROR) \
  { return; } } while (0)
#define CURSOR_BOUND_PARSE(F, X, N, M) \
  do { eatWhitespace((X)); F((X), (N), (M)); \
  if((X)->breakReason == CURSOR_ERROR) { return; } } while (0)

Cursor cursor(char *);
CursorBreakReason breakReason(char);
void eatWhitespace(Cursor *);
void printCursorErr(Cursor *);
