/* Parses text from stdin into proper boar commands and their arguments. boar
 * functions are always monadic or niladic, and expect an argument of a
 * specific type. Numerical parameters can be cast from ints to floats, and
 * vice-versa. Please read "types.h" for more information. */

#include <ctype.h>
#include <err.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "parse.h"

#include "defaults.h"
#include "errors.h"
#include "types.h"

static unsigned int readArg(char *);
static bool isValidArg(const unsigned int, const unsigned int);
static int parseFunc(Cmd *, char *);
static Error parseArg(Cmd *, const unsigned int, char *);
static char * printArg(unsigned int);

#define ACTIVATE_FLAG(n, x) (n | x)
#define IS_FLAG_ACTIVE(n, x) ((bool)(n & x))

static unsigned int
readArg(char *line) {

/* Reads a line of input, and engages relevant TYPE_FLAG_* bits based upon the
 * characters it encounters. It will not break early even if contradictory
 * flags are engaged. The task of actually making sense of the returned type
 * value falls upon isValidArg() */

  unsigned int t = TYPE_NIL;

  while (*line != '\0') {
    if (isblank((int)*line)) {
      ;
    } else if (*line == '-') {
      t = ACTIVATE_FLAG(t, TYPE_FLAG_SIGNED);
    }else if (*line == '.') {
      t = ACTIVATE_FLAG(t, TYPE_FLAG_FLOATING);
    } else if (isdigit((int)*line)) {
      t = ACTIVATE_FLAG(t, TYPE_FLAG_NUMBER);
    } else if (isalpha((int)*line)) {
      t = ACTIVATE_FLAG(t, TYPE_FLAG_TEXT); 
    } else {
      t = ACTIVATE_FLAG(t, TYPE_FLAG_MISC);
    }
    line++;
  }
  return t;
}

static bool
isValidArg(const unsigned int expected, const unsigned int t) {

/* Compares a parsed type against an expected type, and determines if they
 * match one another. There are various instances where slightly different
 * types can still be a valid combination. */

  if (t == TYPE_NIL && expected != TYPE_NIL) {
    return false;
  }
  if (expected == t || expected == TYPE_ANY) {
    return true;
  }
  if (IS_FLAG_ACTIVE(t, TYPE_FLAG_MISC)) {
    /* All non-alphanumeric characters are unacceptable unless the expected
     * type is TYPE_ANY. */
    return false;
  }
  if (IS_FLAG_ACTIVE(t, TYPE_FLAG_SIGNED) && 
      (expected == TYPE_UFLOAT || expected == TYPE_UINT)) {
    /* A negative value cannot satisfy functions that expect unipolar
     * integers and floats. */
    return false;
  }
  if (IS_FLAG_ACTIVE(t, TYPE_FLAG_TEXT) && t != TYPE_TEXT) {
    /* TYPE_TEXT only expects alphabetical characters. */
    return false;
  }
  if ((IS_FLAG_ACTIVE(t, TYPE_FLAG_NUMBER) && expected == TYPE_TEXT) ||
      (IS_FLAG_ACTIVE(t, TYPE_FLAG_TEXT) &&
       IS_FLAG_ACTIVE(expected, TYPE_FLAG_NUMBER))) {
    /* Numerical characters and alphabetical characters cannot intermingle
     * unless the expected type is TYPE_ANY. */
    return false;
  }
  if (t != TYPE_NIL && expected == TYPE_NIL) {
    /* It's most likely needlessly pedantic, as arguments can simply be
     * ignored, but TYPE_NIL demands no arguments whatsoever. */
    return false;
  }
  return true;
}

static int
parseFunc(Cmd *c, char *line) {

/* Populates a Cmd struct with the Func and Type of the command depicted in
 * line. Returns the length of command (and trailing whitespace) to aid further 
 * parsing. Returns -1 if the command is invalid. */

  int span = 1;
  unsigned int type = TYPE_UNDEFINED;
  unsigned int typeIndex = 26; /* defaults to always undefined index */

  for (; isblank((int)*line); line++, span++) {
    /* chew up any leading whitespace before command */
    ;
  }
  if (!isalpha((int)line[0])) {
    return -1;
  }
  typeIndex = line[0] - DEFAULT_ASCII_A;
  if (typeIndex > 57) {
    /* prevent out of bounds access */
    return -1;
  }
  switch (line[1]) {
    case '.':
      type = TYPE_SIGNATURES_PERIOD[typeIndex];
      c->Func = typeIndex | TYPE_PERIOD;
      span++;
      break;
    case ':':
      type = TYPE_SIGNATURES_COLON[typeIndex];
      c->Func = typeIndex | TYPE_COLON;
      span++;
      break;
    default:
      type = TYPE_SIGNATURES_PURE[typeIndex];
      c->Func = typeIndex;
  }
  if (type == TYPE_UNDEFINED) {
    return -1;
  }
  c->Type = type;
  for (line += 2; isblank((int)*line); line++, span++) {
    /* chew up any trailing whitespace after command */
    ;
  }
  return span;
}

static Error
parseArg(Cmd *c, const unsigned int t, char *line) {

/* After isValidArg() has ruled out any contradictory type flags, parseArg()
 * reads the user input into an actual numerical value that can be used by
 * boar commands. Floats and ints can be cast to one another to meet function
 * argument expectations. */

  if (IS_FLAG_ACTIVE(t, TYPE_FLAG_FLOATING) &&
      (! IS_FLAG_ACTIVE(c->Type, TYPE_FLAG_FLOATING)) &&
      (c->Type != TYPE_ANY)) {
    /* If an integer is c->Type but a float is provided, the float is
     * truncated and converted to an int. */
    if (sscanf(line, "%f", &c->Arg.F) == 0) {
      return ERROR_INPUT;
    }
    c->Arg.I = floorf(c->Arg.F);
  } else if (IS_FLAG_ACTIVE(c->Type, TYPE_FLAG_FLOATING)) {
    if (sscanf(line, "%f", &c->Arg.F) == 0) {
      return ERROR_INPUT;
    }
  } else if (IS_FLAG_ACTIVE(c->Type, TYPE_FLAG_NUMBER)) {
    if (sscanf(line, "%d", &c->Arg.I) == 0) {
      return ERROR_INPUT;
    }
  } else {
    /* "line" is the REPL's buffer. Because no value of Cmd.Arg needs to live
     * in lone-term memory, it is acceptable to point Cmd.Arg.S directly to
     * the buffer, which is guaranteed not be overwritten during the lifetime
     * of function evaluation. */
    c->Arg.S = line;
  }
  return ERROR_OK;
}

Error
parseLine(Cmd *c, char *line) {

/* Reads a full line of user input into a Cmd struct, returning any errors
 * it encounters. */ 

  int span = 0;
  unsigned int t = TYPE_UNDEFINED;

  span = parseFunc(c, line);
  if (span == -1) {
    return ERROR_FUNCTION;
  }
  line += span;
  t = readArg(line);
  if (!isValidArg(c->Type, t)) {
    warnx("Invalid argument %s. Expected %s, got %s", line,
        printArg(c->Type), printArg(t));
    return ERROR_ARG;
  }
  return parseArg(c, t, line);
}

static char *
printArg(unsigned int t) {

/* Returns TYPE_* in plaintext for error messages. */

  switch (t) {
    case TYPE_NIL:
      return "nil";
    case TYPE_UINT:
      return "unipolar int";
    case TYPE_INT:
      return "bipolar int";
    case TYPE_UFLOAT:
      return "unipolar float";
    case TYPE_FLOAT:
      return "bipolar float";
    case TYPE_TEXT:
      return "char";
    default:
      return "generic text";
  }
}
