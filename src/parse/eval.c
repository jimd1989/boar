#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <err.h> /* temp */

#include "../control/control.h"
#include "../control/volume.h"
#include "../keyboard/keyboard.h"
#include "../voices/voice.h"
#include "../voices/voice_zones.h"
#include "cmd.h"
#include "cursor.h"
#include "eval.h"
#include "parameter.h"

#define IS_FLOAT_NULL isnan

static void evalPure(CmdPure, Cursor *, Control *);
static void evalDot(CmdDot, Cursor *, Control *);
static void evalPlus(CmdPlus, Cursor *, Control *);
static void evalVol(Cursor *, Control *);
static void evalAttack(Cursor *);
static void evalToggleZone(Cursor *, Control *);
static void evalComment(Cursor *);
static void evalNoteOff(Cursor *, Control *);
static void evalNoteOn(Cursor *, Control *);
static void evalWave(Cursor *);
static void evalMixBalance(Cursor *, Control *);
static void evalSplitZone(Cursor *, Control *);

static void evalVol(Cursor *c, Control *co) {
  CURSOR_BOUND_PARSE(parseBoundFloat, c, 0.0f, 1.0f);
  setVol(&co->vol, c->val.f);
}

static void evalAttack(Cursor *c) {
  int env       = 0;
  float seconds = 0.0f;
  CURSOR_PARSE(parseInt, c);
  env           = c->val.n; 
  CURSOR_PARSE(parseFloat, c);
  seconds       = c->val.f;
  warnx("Env %d attack %fs", env, seconds);
}

static void evalComment(Cursor *c) {
  c->breakReason = CURSOR_LINE_END;
}

static void evalNoteOff(Cursor *c, Control *co) {
  /* o n   → Note off, 0 velocity
   * o n v → Note off, v velocity */
  int8_t note = 0;
  int8_t vel  = 0;
  CURSOR_BOUND_PARSE(parseBoundInt, c, 0, KEYBOARD_SIZE - 1);
  note        = c->val.n;
  if (c->breakReason == CURSOR_PARAMETER_END) {
    /* optional velocity argument */
    CURSOR_BOUND_PARSE(parseBoundInt, c, 0, 127);
    vel = c->val.n;
  }
  noteOff(co, note, vel);
}

static void evalNoteOn(Cursor *c, Control *co) {
  /* n m   → Note on, full velocity
   * n m v → Note on, v velocity
   * n m 0 → Note off (Some MIDI devices signal off this way) */
  int8_t note = 0;
  int8_t vel  = 127;
  CURSOR_BOUND_PARSE(parseBoundInt, c, 0, KEYBOARD_SIZE - 1);
  note        = c->val.n;
  if (c->breakReason == CURSOR_PARAMETER_END) {
    /* optional velocity argument */
    CURSOR_BOUND_PARSE(parseBoundInt, c, 0, 127);
    vel = c->val.n;
  }
  vel == 0 ? noteOff(co, note, vel) : noteOn(co, note, vel);
}

static void evalWave(Cursor *c) {
  int osc    = 0;
  char *wave = NULL;
  CURSOR_PARSE(parseInt, c);
  osc        = c->val.n;
  CURSOR_PARSE(parseString, c);
  wave       = c->val.s;
  warnx("Osc %d set to wave %s", osc, wave);
}

static void evalToggleZone(Cursor *c, Control *co) {
  CURSOR_BOUND_PARSE(parseBoundInt, c, 1, VOICES_SIZE);
  co->voiceZones.currentZone = c->val.n - 1; 
}

static void evalMixBalance(Cursor *c, Control *co) {
  /* v. n … → set zone mixer vol for channels n … */
  int i   = 0;
  for (; i < co->args.chan ; i++) {
    if (c->breakReason & (CURSOR_WAITING_PARAMS | CURSOR_PARAMETER_END)) {
      CURSOR_BOUND_PARSE(parseBoundFloat, c, 0.0f, 1.0f);
    }
  }
}

static void evalSplitZone(Cursor *c, Control *co) {
  /* z+ n     → split zones evenly 
   * z+ _ n m → split zones with leftovers
   * Will instantly mute all voices. Might be a click. */
  int n = 0;
  int m = 0;
  CURSOR_BOUND_PARSE(parseNullableBoundFloat, c, 1.0f, (float)VOICES_SIZE);
  if (IS_FLOAT_NULL(c->val.f)) {
    CURSOR_BOUND_PARSE(parseBoundInt, c, 0, KEYBOARD_SIZE - VOICES_SIZE);
    n = c->val.n;
    CURSOR_BOUND_PARSE(parseBoundInt, c, 1, VOICES_SIZE - 1);
    m = c->val.n;
    splitZonesWithLeftovers(&co->voiceZones, &co->keyboard, n, m);
  } else {
    n  = (int)c->val.f;
    /* Truncate [1, 8] to power of 2 for even zoning. */
    n |= n >> 1;
    n |= n >> 2;
    n  = n - (n >> 1);
    splitZonesEvenly(&co->voiceZones, &co->keyboard, n);
  }
}

static void evalPure(CmdPure cmd, Cursor *c, Control *co) {
  switch (cmd) {
    case CMD_NOTE_ON:
      evalNoteOn(c, co);
      break;
    case CMD_NOTE_OFF:
      evalNoteOff(c, co);
      break;
    case CMD_BEND:
      break;
    case CMD_AFTERTOUCH:
      break;
    case CMD_VOL:
      evalVol(c, co);
      break;
    case CMD_ZONE:
      evalToggleZone(c, co);
      break;
    case CMD_ATTACK:
      evalAttack(c);
      break;
    case CMD_WAVE:
      evalWave(c);
      break;
    case CMD_COMMENT:
      evalComment(c);
      break;
  }
}

static void evalDot(CmdDot cmd, Cursor *c, Control *co) {
  switch (cmd) {
    case CMD_DOT_VOL:
      evalMixBalance(c, co);
      break;
  }
}

static void evalPlus(CmdPlus cmd, Cursor *c, Control *co) {
  switch (cmd) {
    case CMD_PLUS_ZONE:
      evalSplitZone(c, co);
      break;
  }
}

void eval(Cursor *c, Control *co) {
  Cmd cmd = c->val.n;
  if      (IS_CMD_DOT(cmd))   { evalDot(CMD_CHAR(cmd), c, co);  }
  else if (IS_CMD_COLON(cmd)) { return; }
  else if (IS_CMD_PLUS(cmd))  { evalPlus(CMD_CHAR(cmd), c, co); }
  else                        { evalPure(CMD_CHAR(cmd), c, co); }
}
