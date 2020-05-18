# pragma once

#include <stdint.h>

#include "errors.h"

typedef union Arg {

/* A union where textual user input is parsed to. Can be an int, float, or a
 * string. */

    int32_t       I;
    float         F;
    char        * S;
} Arg;

typedef struct Cmd {

/* The representation of a single boar command. Cmd.Func is a character that
 * corresponds with one of boar's functions, Cmd.Type is the signature of the
 * expected argument, and Cmd.Arg is the solitary argument to this function. */

    char                Func;
    unsigned int        NewFunc;
    unsigned int        Type;
    Arg                 Arg;
} Cmd;

/* headers */
Error newParseLine(Cmd *, char *);
Error parseLine(Cmd *, char *);
