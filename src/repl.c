/* The REPL is the user-facing interface for the program. It reads commands
 * from stdin and passes them to an already-running output thread via the 
 * Audio struct. The REPL can also echo commands to the file specified by
 * DEFAULT_ECHO_FILE, which can be used to pipe information between multiple
 * instances of boar. */

#include <err.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "repl.h"

#include "audio-init.h"
#include "audio-output.h"
#include "defaults.h"
#include "errors.h"
#include "funcs.h"
#include "key.h"
#include "parse.h"
#include "numerical.h"
#include "voice.h"
#include "wave.h"

static void echoNoteCheck(const Repl *);
static void echoString(const char *);
static Error dispatchCmd(Repl *);
static void printParseErr(const Error, const char *);

static void
echoNoteCheck(const Repl *r) {

/* If DEFAULT_ECHO_NOTES is enabled, all note on/off commands will be echoed
 * to DEFAULT_ECHO file. This is used to send the same note to multiple
 * instances of boar that have been piped together. */

  if (r->EchoNotes) {
    fprintf(DEFAULT_ECHO_FILE, "%c %d\n", r->Cmd.Func + DEFAULT_ASCII_A, 
        r->Cmd.Arg.I);
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

static Error
dispatchCmd(Repl *r) {

/* Runs a command against the Audio struct. */

  switch(r->Cmd.Func) {
    case FUNC_NOTE_ON:
      echoNoteCheck(r);
      voiceOn(&r->Audio->Voices, (uint16_t)r->Cmd.Arg.I);
      break;
    case FUNC_NOTE_OFF:
      echoNoteCheck(r);
      voiceOff(&r->Audio->Voices, (uint16_t)r->Cmd.Arg.I);
      break;
    case FUNC_MOD_ATTACK:
      setAttackLevel(&r->Audio->Voices.Modulator.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_ATTACK:
      setAttackLevel(&r->Audio->Voices.Carrier.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_MOD_ATTACK_WAVE:
      setAttackWave(&r->Audio->Voices.Modulator.Env, r->Cmd.Arg.I);
      break;
    case FUNC_ATTACK_WAVE:
      setAttackWave(&r->Audio->Voices.Carrier.Env, r->Cmd.Arg.I);
      break;
    case FUNC_MOD_DECAY:
      setDecayLevel(&r->Audio->Voices.Modulator.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_DECAY:
      setDecayLevel(&r->Audio->Voices.Carrier.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_MOD_DECAY_WAVE:
      setDecayWave(&r->Audio->Voices.Modulator.Env, r->Cmd.Arg.I);
      break;
    case FUNC_DECAY_WAVE:
      setDecayWave(&r->Audio->Voices.Carrier.Env, r->Cmd.Arg.I);
      break;
    case FUNC_MOD_ENV_LOOP:
      setLoop(&r->Audio->Voices.Modulator.Env, (bool)r->Cmd.Arg.I);
      break;
    case FUNC_ENV_LOOP:
      setLoop(&r->Audio->Voices.Carrier.Env, (bool)r->Cmd.Arg.I);
      break;
    case FUNC_ECHO:
      echoString(r->Cmd.Arg.S);
      break;
    case FUNC_MOD_KEY_FOLLOW:
      selectWave(&r->Audio->Voices.Keyboard.Modulator.KeyFollowCurve,
          r->Cmd.Arg.I);
      break;
    case FUNC_KEY_FOLLOW:
      selectWave(&r->Audio->Voices.Keyboard.Carrier.KeyFollowCurve,
          r->Cmd.Arg.I);
      break;
    case FUNC_MOD_AMPLITUDE:
      setModulation(&r->Audio->Voices, r->Cmd.Arg.F);
      break;
    case FUNC_AMPLITUDE:
      setVolume(r->Audio, r->Cmd.Arg.F);
      break;
    case FUNC_MOD_PITCH:
      setPitchRatio(&r->Audio->Voices, false, r->Cmd.Arg.F);
      break;
    case FUNC_PITCH:
      setPitchRatio(&r->Audio->Voices, true, r->Cmd.Arg.F);
      break;
    case FUNC_QUIT:
      fprintf(DEFAULT_ECHO_FILE, "q\n");
      r->Audio->Active = false;
      return ERROR_EXIT;
    case FUNC_MOD_RELEASE:
      setReleaseLevel(&r->Audio->Voices.Modulator.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_RELEASE:
      setReleaseLevel(&r->Audio->Voices.Carrier.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_MOD_RELEASE_WAVE:
      setReleaseWave(&r->Audio->Voices.Modulator.Env, r->Cmd.Arg.I);
      break;
    case FUNC_RELEASE_WAVE:
      setReleaseWave(&r->Audio->Voices.Carrier.Env, r->Cmd.Arg.I);
      break;
    case FUNC_MOD_SUSTAIN:
      setSustainLevel(&r->Audio->Voices.Modulator.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_SUSTAIN:
      setSustainLevel(&r->Audio->Voices.Carrier.Env,
          truncateFloat(r->Cmd.Arg.F, 1.0f));
      break;
    case FUNC_MOD_TOUCH:
      selectWave(&r->Audio->Voices.Keyboard.Modulator.VelocityCurve, 
          r->Cmd.Arg.I);
      break;
    case FUNC_TOUCH:
      selectWave(&r->Audio->Voices.Keyboard.Carrier.VelocityCurve, 
          r->Cmd.Arg.I);
      break;
    case FUNC_TUNE_NOTE:
      selectTuningKey(&r->Audio->Voices.Keyboard, r->Cmd.Arg.I);
      break;
    case FUNC_TUNE:
      tuneKey(&r->Audio->Voices.Keyboard, r->Cmd.Arg.F);
      break;
    case FUNC_TUNE_TARGET:
      selectTuningLayer(&r->Audio->Voices.Keyboard,
          (TuningLayer)r->Cmd.Arg.I);
      break;
    case FUNC_MOD_WAVE:
      selectWave(&r->Audio->Voices.Modulator.Wave, r->Cmd.Arg.I);
      break;
    case FUNC_WAVE:
      selectWave(&r->Audio->Voices.Carrier.Wave, r->Cmd.Arg.I);
      break;
    case FUNC_MOD_FIXED:
      setFixedRate(&r->Audio->Voices, false, r->Cmd.Arg.F);
      break;
    case FUNC_FIXED:
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

  warnx("Welcome. You can exit at any time by pressing q + enter.");
  while (fgets(r->Buffer, DEFAULT_LINESIZE, stdin) != NULL) {
    if (r->Buffer[0] == '\n' || r->Buffer[0] == '#') {
      /* ignore blank or commented input */
      ;
    } else {
      /* read multiple commands buffered by semicolons */
      err = parseLine(&r->Cmd, r->Buffer);
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
}
