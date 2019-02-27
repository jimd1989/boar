#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "audio-init.h"
#include "defaults.h"

/* types */
typedef union Arg {

/* Can be either a 32bit int, float, or string, depending on context. Arg.S
 * is not an independently-allocated string, but rather a pointer to 
 * Repl.Buffer[2], where the argument string of fgets()-snatched user input
 * begins. This is an acceptable solution because the lifetime of an Arg does 
 * not last beyond a single cycle of the REPL. If some long-term reference to 
 * Arg.S is ever required by a different part of the program, then it must be 
 * copied to a newly-allocated stretch of memory. Arg.S also retains its
 * terminating newline, which is useful when echoing strings further down
 * the pipe. */

    uint32_t      I;
    float         F;
    char        * S;
} Arg;

typedef struct Cmd {

/* Represents a single user-issued command, where (uint8_t)Func != ARG_INVALID
 * in the VALID_FUNCTIONS array. */ 

    char        Func;
    Arg         Arg;
} Cmd;

typedef struct Repl {

/* A struct containing everything needed for a user-facing loop. During every
 * cycle of the REPL, user input is read with fgets() into Repl.Buffer. This
 * string is then parsed, and used to populate the fields of Repl.Cmd. An audio
 * function is then performed using Repl.Cmd as its argument. */

    char          Pointer;
    bool          EchoNotes;
    Cmd           Cmd;
    char          Buffer[DEFAULT_LINESIZE];
    Audio       * Audio;
} Repl;

/* headers */
void repl(Repl *);
