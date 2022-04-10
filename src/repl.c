/* The REPL is the user-facing interface for the program. It reads commands
 * from stdin and passes them to sndio thread via the Audio struct. */ 

#include <err.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "repl.h"

#include "amplitude.h"
#include "audio-init.h"
#include "audio-output.h"
#include "constants/defaults.h"
#include "constants/errors.h"
#include "constants/funcs.h"
#include "key.h"
#include "parse.h"
#include "numerical.h"
#include "voice.h"
#include "wave.h"

static void dispatchCmd(Repl *);
static void printParseErr(const Error, const char *);
static void readLine(Repl *);

static void
dispatchCmd(Repl *r) {

/* Runs a command against the Audio struct. */

  Arg *arg = &r->Cmd.Arg;
  Operators *carrier = &r->Audio->Voices.Carrier;
  Operators *modulator = &r->Audio->Voices.Modulator;
  Voices *voices = &r->Audio->Voices;

  switch(r->Cmd.Func) {
    case FUNC_NOTE_ON:
      voiceOn(voices, (uint16_t)arg->I);
      break;
    case FUNC_NOTE_OFF:
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
      setVolume(&r->Audio->Amplitude, arg->F);
      break;
    case FUNC_MOD_PITCH:
      setPitchRatio(voices, false, arg->F);
      break;
    case FUNC_PITCH:
      setPitchRatio(voices, true, arg->F);
      break;
    case FUNC_CHAN_BALANCE:
      setBalance(&r->Audio->Amplitude, arg->F);
      break;
    case FUNC_QUIT:
      r->Cmd.Error = ERROR_EXIT;
      return;
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
  r->Cmd.Error = ERROR_OK;
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

static void
readLine(Repl *r) {

/* Reads a full line of user input, which can be a single command terminated
 * by a newline, or multiple commands delimited by semicolons, but also ending
 * in a newline. Dispatches a command immediately after it is parsed. Has to use
 * read() in order to leave audio playback unblocked. This is more troublesome
 * than fgets(). */  

  int bytesParsed = 0;
  int totalBytesParsed = 0;
  int bytesRead = 0;
  char *line = NULL;

  bytesRead = read(STDIN_FILENO, r->Buffer, DEFAULT_LINESIZE);
  if (bytesRead < 1 || r->Buffer[0] == '\n' || r->Buffer[0] == '#') {
    r->Cmd.Error = ERROR_NOTHING;
    return;
  }
  /* Since all input is newline buffered, should terminate string one char
   * early to avoid persisting the \n. */
  r->Buffer[--bytesRead] = '\0';
  line = r->Buffer;
  while (totalBytesParsed < bytesRead) {
    bytesParsed = parseCmd(&r->Cmd, line);
    totalBytesParsed += bytesParsed;
    if (r->Cmd.Error != ERROR_OK) {
      printParseErr(r->Cmd.Error, line);
    } else {
      if (r->Cmd.Error == ERROR_EXIT) {
        return;
      }
      dispatchCmd(r);
    }
    line += bytesParsed;
  }
}

void
repl(Repl *r) {

/* The main user-facing loop. Reads lines of user input, parses them, and sends
 * them to the Audio struct for processing. */

  struct pollfd pfds[1] = {{0}};

  pfds[0].fd = STDIN_FILENO;
  pfds[0].events = POLLIN;
  warnx("Welcome. You can exit at any time by pressing q + enter.");
  while(poll(pfds, 1, 0) != -1) {
    if (pfds[0].revents & POLLIN) {
      readLine(r);
      if (r->Cmd.Error == ERROR_EXIT) {
        return;
      }
    }
    play(r->Audio);
  }
}
