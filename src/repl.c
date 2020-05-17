/* The REPL is the user-facing interface for the program. It reads commands
 * from stdin and passes them to an already-running output thread via the 
 * Audio struct. The REPL can also echo commands to the file specified by
 * DEFAULT_ECHO_FILE, which can be used to pipe information between multiple
 * instances of boar. */

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
#include "key.h"
#include "parse.h"
#include "numerical.h"
#include "voice.h"
#include "wave.h"

/* headers */
static void echoNoteCheck(const Repl *);
static void echoString(const char *);
static void dispatchIntPointer(Repl *);
static Error dispatchCmd(Repl *);
static void printParseErr(const Error, const char *);

/* functions */
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
 * commands to other instances of boar in the pipeline. */

    fprintf(DEFAULT_ECHO_FILE, "%s\n", s);
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
       case 'u':
           selectTuningLayer(&r->Audio->Voices.Keyboard,
                   (TuningLayer)r->Cmd.Arg.I);
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
        case 'E':
            r->Pointer = r->Cmd.Arg.S[0];
            break;
        case 'e':
            echoString(r->Cmd.Arg.S);
            break;
        case 'K':
            selectWave(&r->Audio->Voices.Keyboard.Modulator.KeyFollowCurve,
                    r->Cmd.Arg.I);
            break;
        case 'k':
            selectWave(&r->Audio->Voices.Keyboard.Carrier.KeyFollowCurve,
                    r->Cmd.Arg.I);
            break;
        case 'L':
            setModulation(&r->Audio->Voices, r->Cmd.Arg.F);
            break;
        case 'l':
            setVolume(r->Audio, r->Cmd.Arg.F);
            break;
        case 'n':
            echoNoteCheck(r);
            voiceOn(&r->Audio->Voices, (uint16_t)r->Cmd.Arg.I);
            break;
        case 'o':
            echoNoteCheck(r);
            voiceOff(&r->Audio->Voices, (uint16_t)r->Cmd.Arg.I);
            break;
        case 'P':
            setPitchRatio(&r->Audio->Voices, false, r->Cmd.Arg.F);
            break;
        case 'p':
            setPitchRatio(&r->Audio->Voices, true, r->Cmd.Arg.F);
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
        case 'T':
            selectWave(&r->Audio->Voices.Keyboard.Modulator.VelocityCurve, 
                    r->Cmd.Arg.I);
            break;
        case 't':
            selectWave(&r->Audio->Voices.Keyboard.Carrier.VelocityCurve, 
                    r->Cmd.Arg.I);
            break;
        case 'U':
            selectTuningKey(&r->Audio->Voices.Keyboard, r->Cmd.Arg.I);
            break;
        case 'u':
            tuneKey(&r->Audio->Voices.Keyboard, r->Cmd.Arg.F);
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
        case 'X':
            setFixedRate(&r->Audio->Voices, false, r->Cmd.Arg.F);
            break;
        case 'x':
            setFixedRate(&r->Audio->Voices, true, r->Cmd.Arg.F);
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
    char *token = NULL;
    unsigned int span = 0;

    warnx("Welcome. You can exit at any time by pressing q + enter.");
    while (fgets(r->Buffer, DEFAULT_LINESIZE, stdin) != NULL) {
        span = strcspn(r->Buffer, "\n");
        if (span == 0) {
            /* ignore blank input */
            ;
        } else {
            /* read multiple commands buffered by semicolons */
            r->Buffer[span] = '\0';
            token = strtok(r->Buffer, ";");
            while (token != NULL) {
                err = parseLine(&r->Cmd, token);
                if (err != ERROR_OK) {
                    printParseErr(err, r->Buffer);
                } else {
                    err = dispatchCmd(r);
                    if (err == ERROR_EXIT) {
                        return;
                    }
                }
                token = strtok(NULL, ";");
            }
        }
    }
}
