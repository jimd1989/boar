/* The REPL is the user-facing interface for the program. It reads commands
 * from stdin and passes them to an already-running output thread via the 
 * Audio struct. The REPL can also echo commands to the file specified by
 * DEFAULT_ECHO_FILE, which can be used to pipe information between multiple
 * instances of boar. */

#include <ctype.h>
#include <err.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "repl.h"

#include "audio-init.h"
#include "audio-output.h"
#include "defaults.h"
#include "errors.h"
#include "numerical.h"
#include "valid-functions.h"
#include "voice.h"
#include "wave.h"

/* headers */
static bool isValidFunction(const char *);
static bool isValidArg(const char, const ArgStatus);
static bool isNiladicFunction(const char);
static Error identifyArg(char *);
static Error parseArg(const Error, char *, Cmd *);
static Error parseLine(char *, Cmd *); 
static void echoNoteCheck(const Repl *);
static void echoString(const char *);
static void dispatchIntPointer(Repl *);
static Error dispatchCmd(Repl *);
static void printParseErr(const Error, const char *);

/* functions */
static bool
isValidFunction(const char *line) {

/* Checks if the begining of a user input line contains a single valid
 * function character, followed by a single space. */

    return (VALID_FUNCTIONS[(uint8_t)line[0]] && (line[1] == ' '));
}

static bool
isValidArg(const char func, const ArgStatus argType) {

/* Checks if the argument provided to func is of the correct type. */

    return VALID_FUNCTIONS[(uint8_t)func] == argType;
}

static bool
isNiladicFunction(const char func) {

/* Checks if function character type is ARG_NIL, meaning no argument is
 * expected. At the moment, any parameters that are mistakenly provided to
 * niladic functions are silently ignored, and the function proceeds as usual.
 */

    return VALID_FUNCTIONS[(uint8_t)func] == ARG_NIL;
}

static Error
identifyArg(char *line) {

/* Determines whether an input string represents an integer, float, string, or 
 * is invalid. */

    Error status = ARG_INVALID;

    while (*line != 0 && *line != '\n') {
        if (*line == ' ') {
            ;
        } else if (*line == '.') {
            if (status == ARG_FLOAT) {
                return ARG_INVALID;
            } else if (status != ARG_STRING) {
                status = ARG_FLOAT;
            }
        } else if (isdigit((int)*line)) {
            if (status == ARG_INVALID) {
                status = ARG_INT;
            }
        } else if (isalpha((int)*line)) {
            if (status == ARG_INT || status == ARG_FLOAT) {
                return ARG_INVALID;
            }
            status = ARG_STRING;
        } else {
            return ARG_INVALID;
        }
        line++;
    }
    return status;
}

static Error
parseArg(const Error status, char *line, Cmd *c) {

/* Has the side effect of assigning a value to Cmd.Arg based upon the type
 * of "status". Returns errors if any exist. */

    switch((unsigned int)status) {
        case ARG_INVALID:
            return ERROR_INPUT;
            break;
        case ARG_INT:
            if (sscanf(line, "%u", &c->Arg.I) == 0) {
                return ERROR_INPUT;
            }
            break;
        case ARG_FLOAT:
            if (sscanf(line, "%f", &c->Arg.F) == 0) {
                return ERROR_INPUT;
            }
            break;
        case ARG_STRING:
            c->Arg.S = line;
            break;
    }
    return ERROR_OK;
}

static Error
parseLine(char *line, Cmd *c) {

/* Parses a line of user input into an audio procedure. For the time being,
 * all procedures are of the form {f Arg}, where f is a single character
 * followed by a space. Arg is a single integer, float, or string. All input
 * is assumed to be newline delimited. If the syntax is valid, the fields of
 * Cmd will be populated with the info parsed from the input. Various errors
 * are returned otherwise. */
    
    const int span = strcspn(line, "\n");
    Error status = ARG_INVALID;

    if (span == 0) {
        /* ignore blank input. */
        return ERROR_NOTHING;
    }
    /* Parse Cmd.Func */
    if (isNiladicFunction(line[0])) {
        c->Func = line[0];
        return ERROR_OK;
    }
    if (! isValidFunction(line)) {
        return ERROR_FUNCTION;
    }
    c->Func = line[0];
    line += 2;
    /* Parse Cmd.Arg */
    status = identifyArg(line);
    if (! isValidArg(c->Func, status)) {
        return ERROR_TYPE;
    }
    return parseArg(status, line, c);
}

static void
echoNoteCheck(const Repl *r) {

/* If DEFAULT_ECHO_NOTES is enabled, all note on/off commands will be echoed
 * to DEFAULT_ECHO file. This is used to send the same note to multiple
 * instances of boar that have been piped together. */

    if (r->EchoNotes) {
        fprintf(DEFAULT_ECHO_FILE, "%c %d\n", r->Cmd.Func, r->Cmd.Arg.I);
        fflush(DEFAULT_ECHO_FILE);
    }
}

static void
echoString(const char *s) {

/* Echoes a string to DEFAULT_ECHO_FILE. This is uses to send arbitrary
 * commands to other instances of boar in the pipeline. The get (g) command
 * is prepended to this string, telling the receiving end of the pipe to
 * parse the string's contents as a boar command. */

    fprintf(DEFAULT_ECHO_FILE, "%s", s);
    fflush(DEFAULT_ECHO_FILE);
}

static void
dispatchIntPointer(Repl *r) {

/* Pushes a value to the parameter pointed to by Repl.Pointer. */ 

   switch(r->Pointer) {
       case 'A':
           setAttackWave(&r->Audio->Voices.Modulator.Env, r->Cmd.Arg.I);
           break;
       case 'a':
           setAttackWave(&r->Audio->Voices.Carrier.Env, r->Cmd.Arg.I);
           break;
       case 'D':
           setDecayWave(&r->Audio->Voices.Modulator.Env, r->Cmd.Arg.I);
           break;
       case 'd':
           setDecayWave(&r->Audio->Voices.Carrier.Env, r->Cmd.Arg.I);
           break;
       case 'R':
           setReleaseWave(&r->Audio->Voices.Modulator.Env, r->Cmd.Arg.I);
           break;
       case 'r':
           setReleaseWave(&r->Audio->Voices.Carrier.Env, r->Cmd.Arg.I);
           break;
       default:
           warnx("No valid parameter specified");
   } 
}

static Error
dispatchCmd(Repl *r) {

/* Runs a command against the Audio struct. */

    switch(r->Cmd.Func) {
        case '#':
            break;
        case 'A':
            setAttackLevel(&r->Audio->Voices.Modulator.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'a':
            setAttackLevel(&r->Audio->Voices.Carrier.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'D':
            setDecayLevel(&r->Audio->Voices.Modulator.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'd':
            setDecayLevel(&r->Audio->Voices.Carrier.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'e':
            echoString(r->Cmd.Arg.S);
            break;
        case 'l':
            setVolume(r->Audio, truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'm':
            setModulation(&r->Audio->Voices, r->Cmd.Arg.F);
            break;
        case 'n':
            echoNoteCheck(r);
            voiceOn(&r->Audio->Voices, r->Cmd.Arg.I);
            break;
        case 'o':
            echoNoteCheck(r);
            voiceOff(&r->Audio->Voices, r->Cmd.Arg.I);
            break;
        case 'P':
            r->Pointer = r->Cmd.Arg.S[0];
            break;
        case 'p':
            setPitchRatio(&r->Audio->Voices, r->Cmd.Arg.F);
            break;
        case 'q':
            fprintf(DEFAULT_ECHO_FILE, "q\n");
            r->Audio->Active = false;
            return ERROR_EXIT;
        case 'R':
            setReleaseLevel(&r->Audio->Voices.Modulator.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'r':
            setReleaseLevel(&r->Audio->Voices.Carrier.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'S':
            setSustainLevel(&r->Audio->Voices.Modulator.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 's':
            setSustainLevel(&r->Audio->Voices.Carrier.Env,
                    truncateFloat(r->Cmd.Arg.F, 1.0f));
            break;
        case 'v':
            dispatchIntPointer(r);
            break;
        case 'W':
            selectWave(&r->Audio->Voices.Modulator.Wave, r->Cmd.Arg.I);
            break;
        case 'w':
            selectWave(&r->Audio->Voices.Carrier.Wave, r->Cmd.Arg.I);
            break;
    }
    return ERROR_OK;
}

static void
printParseErr(const Error err, const char *buffer) {
    switch((unsigned int)err) {
        case ERROR_NOTHING:
            break;
        case ERROR_INPUT:
            warnx("Invalid input");
            break;
        case ERROR_FUNCTION:
            warnx("Procedure not found: %c%c", buffer[0], buffer[1]);
            break;
        case ERROR_TYPE:
            warnx("Incorrect argument type for %c", buffer[0]);
            break;
    }
}

void
repl(Repl *r) {

/* The main user-facing loop. Reads lines of user input, parses them, and sends
 * them to the Audio struct for processing. */

    Error err = 0;

    warnx("Welcome. You can exit at any time by pressing q + enter.");
    while (fgets(r->Buffer, DEFAULT_LINESIZE, stdin) != NULL) {
        err = parseLine(r->Buffer, &r->Cmd);
        if (err != ERROR_OK) {
            printParseErr(err, r->Buffer);
        } else {
            err = dispatchCmd(r);
            if (err == ERROR_EXIT) {
                return;
            }
        }
    }
}
