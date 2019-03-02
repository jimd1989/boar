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

#include "errors.h"
#include "types.h"

/* headers */
static bool isValidFunc(const char *);
static unsigned int activateFlag(const unsigned int, const unsigned int);
static bool isFlagActive(const unsigned int, const unsigned int);
static unsigned int readArg(char *);
static bool isValidArg(const unsigned int, const unsigned int);
static Error parseArg(Cmd *, const unsigned int, char *);
static char * printArg(unsigned int);

/* functions */
static bool
isValidFunc(const char *func) {

/* Determines whether the command char provided actually corresponds to a
 * defined boar function. Also ensures a space follows any monadic function
 * invocation. */

    const unsigned int t = TYPE_SIGNATURES[(unsigned int)func[0]];

    return t == TYPE_NIL || t == TYPE_ANY ||
        (t != TYPE_UNDEFINED && func[1] == ' ');
}

static unsigned int
activateFlag(const unsigned int n, const unsigned int x) {

/* Engages a single TYPE_FLAG_* bit in n, which is a running accumulator of
 * such flags while parsing text. */

    return n | x;
}

static bool
isFlagActive(const unsigned int n, const unsigned int x) {

/* Determines if a TYPE_FLAG_* bit exists in n. */

    return (bool)(n & x);
}

static unsigned int
readArg(char *line) {

/* Reads a line of input, and engages relevant TYPE_FLAG_* bits based upon the
 * characters it encounters. It will not break early even if contradictory
 * flags are engaged. The task of actually making sense of the returned type
 * value falls upon isValidArg() */

    unsigned int t = TYPE_NIL;

    while (*line != 0 && *line != '\n') {
        if (*line == ' ' || *line == '\t') {
            ;
        } else if (*line == '-') {
            t = activateFlag(t, TYPE_FLAG_SIGNED);
        }else if (*line == '.') {
            t = activateFlag(t, TYPE_FLAG_FLOATING);
        } else if (isdigit((int)*line)) {
            t = activateFlag(t, TYPE_FLAG_NUMBER);
        } else if (isalpha((int)*line)) {
            t = activateFlag(t, TYPE_FLAG_TEXT); 
        } else {
            t = activateFlag(t, TYPE_FLAG_MISC);
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

    if (expected == t || expected == TYPE_ANY) {
        return true;
    }
    if (isFlagActive(t, TYPE_FLAG_MISC)) {
        /* All non-alphanumeric characters are unacceptable unless the expected
         * type is TYPE_ANY. */
        return false;
    }
    if (isFlagActive(t, TYPE_FLAG_SIGNED) && 
            (expected == TYPE_UFLOAT || expected == TYPE_UINT)) {
        /* A negative value cannot satisfy functions that expect unipolar
         * integers and floats. */
        return false;
    }
    if (isFlagActive(t, TYPE_FLAG_TEXT) && t != TYPE_TEXT) {
        /* TYPE_TEXT only expects alphabetical characters. */
        return false;
    }
    if ((isFlagActive(t, TYPE_FLAG_NUMBER) && expected == TYPE_TEXT) ||
        (isFlagActive(t, TYPE_FLAG_TEXT) &&
         isFlagActive(expected, TYPE_FLAG_NUMBER))) {
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

static Error
parseArg(Cmd *c, const unsigned int t, char *line) {

/* After isValidArg() has ruled out any contradictory type flags, parseArg()
 * reads the user input into an actual numerical value that can be used by
 * boar commands. Floats and ints can be cast to one another to meet function
 * argument expectations. */

    const unsigned int expected = TYPE_SIGNATURES[(unsigned int)c->Func];

    if (isFlagActive(t, TYPE_FLAG_FLOATING) &&
            (! isFlagActive(expected, TYPE_FLAG_FLOATING))) {
        /* If an integer is expected but a float is provided, the float is
         * truncated and converted to an int. */
        if (sscanf(line, "%f", &c->Arg.F) == 0) {
            return ERROR_INPUT;
        }
        c->Arg.I = floorf(c->Arg.F);
    } else if (isFlagActive(expected, TYPE_FLAG_FLOATING)) {
        if (sscanf(line, "%f", &c->Arg.F) == 0) {
            return ERROR_INPUT;
        }
    } else if (isFlagActive(expected, TYPE_FLAG_NUMBER)) {
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

    const int span = strcspn(line, "\n");
    unsigned int t = TYPE_UNDEFINED;

    if (span == 0) {
        /* ignore blank input */
        return ERROR_NOTHING;
    }
    /* Parse Cmd.Func */
    if (! isValidFunc(line)) {
        return ERROR_FUNCTION;
    }
    c->Func = *line;
    line += 2;
    /* Parse Cmd.Arg */
    t = readArg(line);
    if (! isValidArg(TYPE_SIGNATURES[(unsigned int)c->Func], t)) {
        warnx("Invalid argument to %c. Expected %s and got %s.",
                c->Func, printArg(TYPE_SIGNATURES[(unsigned int)c->Func]),
                printArg(t));
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
