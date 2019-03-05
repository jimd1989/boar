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
 * corresponds with one of boar's functions, and Cmd.Arg is the solitary
 * argument to this function. */

    char        Func;
    Arg         Arg;
} Cmd;

/* headers */
Error parseLine(Cmd *, char *);