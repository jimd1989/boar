#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bounds.h"
#include "cursor.h"

void parseInt(Cursor *c) {
  int n    = 0;
  int sign = 1;
  char ch  = '\0';
  while (strchr(BOUNDS_CHARS, CURSOR_HEAD(c)) == NULL) {
    ch = CURSOR_HEAD(c);
    if (ch == '-')        { sign = -1; }
    else if (isdigit(ch)) { n = n * 10 + (ch - '0'); }
    else                  { break; }
    c->pos++;
  }
  c->val.n = sign * n;
  c->breakReason = breakReason(CURSOR_HEAD(c));
  c->pos++;
}

void parseBoundInt(Cursor *c, int min, int max) {
  int n = 0;
  parseInt(c);
  n = c->val.n;
  if (n < min || n > max) {
    c->breakReason = CURSOR_ERROR;
  }
}

void parseFloat(Cursor *c) {
  bool beforeDecimal = true;
  float f            = 0.0f;
  float frac         = 0.0f;
  float div          = 1.0f;
  float sign         = 1.0f;
  char ch            = '\0';
  while ((strchr(BOUNDS_CHARS, CURSOR_HEAD(c)) == NULL) ) {
    ch = CURSOR_HEAD(c);
    if (ch == '-')        { sign = -1.0f; }
    else if (ch == '.')   { beforeDecimal = false; }
    else if (isdigit(ch)) {
      if (beforeDecimal) { f = f * 10.0f + (ch - '0'); }
      else               { frac = frac * 10.0f + (ch - '0'); div = div * 10.f; }
    }
    else                  { break; }
    c->pos++;
  }
  c->val.f = sign * (f + (frac / div));
  c->breakReason = breakReason(CURSOR_HEAD(c));
  c->pos++;
}

void parseBoundFloat(Cursor *c, float min, float max) {
  float f = 0.0f;
  parseFloat(c);
  f = c->val.f;
  if (f < min || f > max) {
    c->breakReason = CURSOR_ERROR;
  }
}

void parseString(Cursor *c) {
  int start = c->pos;
  while (strchr(BOUNDS_CHARS, CURSOR_HEAD(c)) == NULL) { c->pos++; }
  c->breakReason = breakReason(CURSOR_HEAD(c));
  c->input[c->pos] = '\0';
  c->val.s = &c->input[start];
  c->pos++;
}

void parseNullableBoundFloat(Cursor *c, float min, float max) {
  float f = 0.0f;
  if (CURSOR_HEAD(c) == '_') {
    c->pos++;
    c->val.f = NAN;
    return;
  }
  parseFloat(c);
  f = c->val.f;
  if (f < min || f > max) {
    c->breakReason = CURSOR_ERROR;
  }
}
