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

  Arg *arg = &r->Cmd.Arg;
  Operators *carrier = &r->Audio->Voices.Carrier;
  Operators *modulator = &r->Audio->Voices.Modulator;
  Voices *voices = &r->Audio->Voices;

  switch(r->Cmd.Func) {
    case FUNC_NOTE_ON:
      echoNoteCheck(r);
      voiceOn(voices, (uint16_t)arg->I);
      break;
    case FUNC_NOTE_OFF:
      echoNoteCheck(r);
      voiceOff(voices, (uint16_t)arg->I);
      break;
    case FUNC_MOD_ATTACK:
      setAttackLevel(&modulator->Env, arg->F);
      break;
    case FUNC_ATTACK:
      setAttackLevel(&carrier->Env, arg->F);
      break;
    case FUNC_MOD_ATTACK_WAVE:
      setAttackWave(&modulator->Env, arg->I);
      break;
    case FUNC_ATTACK_WAVE:
      setAttackWave(&carrier->Env, arg->I);
      break;
    case FUNC_MOD_DECAY:
      setDecayLevel(&modulator->Env, arg->F);
      break;
    case FUNC_DECAY:
      setDecayLevel(&carrier->Env, arg->F);
      break;
    case FUNC_MOD_DECAY_WAVE:
      setDecayWave(&modulator->Env, arg->I);
      break;
    case FUNC_DECAY_WAVE:
      setDecayWave(&carrier->Env, arg->I);
      break;
    case FUNC_MOD_ENV_LOOP:
      setLoop(&modulator->Env, (bool)arg->I);
      break;
    case FUNC_ENV_LOOP:
      setLoop(&carrier->Env, (bool)arg->I);
      break;
    case FUNC_ECHO:
      echoString(arg->S);
      break;
    case FUNC_MOD_KEY_FOLLOW:
      selectWave(&voices->Keyboard.Modulator.KeyFollowCurve, arg->I);
      break;
    case FUNC_KEY_FOLLOW:
      selectWave(&voices->Keyboard.Carrier.KeyFollowCurve, arg->I);
      break;
    case FUNC_MOD_AMPLITUDE:
      setModulation(voices, arg->F);
      break;
    case FUNC_AMPLITUDE:
      setVolume(r->Audio, arg->F);
      break;
    case FUNC_MOD_PITCH:
      setPitchRatio(voices, false, arg->F);
      break;
    case FUNC_PITCH:
      setPitchRatio(voices, true, arg->F);
      break;
    case FUNC_QUIT:
      fprintf(DEFAULT_ECHO_FILE, "q\n");
      r->Audio->Active = false;
      return ERROR_EXIT;
    case FUNC_MOD_RELEASE:
      setReleaseLevel(&modulator->Env, arg->F);
      break;
    case FUNC_RELEASE:
      setReleaseLevel(&carrier->Env, arg->F);
      break;
    case FUNC_MOD_RELEASE_WAVE:
      setReleaseWave(&modulator->Env, arg->I);
      break;
    case FUNC_RELEASE_WAVE:
      setReleaseWave(&carrier->Env, arg->I);
      break;
    case FUNC_MOD_SUSTAIN:
      setSustainLevel(&modulator->Env, arg->F);
      break;
    case FUNC_SUSTAIN:
      setSustainLevel(&carrier->Env, arg->F);
      break;
    case FUNC_MOD_ENV_DEPTH:
      setDepth(&modulator->Env, arg->F);
      break;
    case FUNC_ENV_DEPTH:
      setDepth(&carrier->Env, arg->F);
      break;
    case FUNC_MOD_TOUCH:
      selectWave(&voices->Keyboard.Modulator.VelocityCurve, arg->I);
      break;
    case FUNC_TOUCH:
      selectWave(&voices->Keyboard.Carrier.VelocityCurve, arg->I);
      break;
    case FUNC_TUNE_NOTE:
      selectTuningKey(&voices->Keyboard, arg->I);
      break;
    case FUNC_TUNE:
      tuneKey(&voices->Keyboard, arg->F);
      break;
    case FUNC_TUNE_TARGET:
      selectTuningLayer(&voices->Keyboard, (TuningLayer)arg->I);
      break;
    case FUNC_MOD_WAVE:
      selectWave(&modulator->Wave, arg->I);
      break;
    case FUNC_WAVE:
      selectWave(&carrier->Wave, arg->I);
      break;
    case FUNC_MOD_FIXED:
      setFixedRate(voices, false, arg->F);
      break;
    case FUNC_FIXED:
      setFixedRate(voices, true, arg->F);
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
