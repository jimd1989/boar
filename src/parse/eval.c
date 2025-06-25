#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <err.h> /* temp */

#include "../control/control.h"
#include "../control/volume.h"
#include "../voices/voice.h"
#include "cmd.h"
#include "cursor.h"
#include "eval.h"
#include "parameter.h"

static void evalPure(CmdPure, Cursor *, Control *);
static void evalPlus(CmdPlus, Cursor *, Control *);
static void evalVol(Cursor *, Control *);
static void evalAttack(Cursor *);
static void evalComment(Cursor *);
static void evalNoteOff(Cursor *, Control *);
static void evalNoteOn(Cursor *, Control *);
static void evalWave(Cursor *);
static void evalZone(Cursor *, Control *);

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
  int8_t note = 0;
  int8_t vel  = 0;
  CURSOR_BOUND_PARSE(parseBoundInt, c, 0, 127);
  note        = c->val.n;
  if (c->breakReason == CURSOR_PARAMETER_END) {
    /* optional velocity argument */
    CURSOR_BOUND_PARSE(parseBoundInt, c, 0, 127);
    vel = c->val.n;
  }
  noteOff(co, note, vel);
}

static void evalNoteOn(Cursor *c, Control *co) {
  int8_t note = 0;
  int8_t vel  = 127;
  CURSOR_BOUND_PARSE(parseBoundInt, c, 0, 127);
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

static void evalZone(Cursor *c, Control *co) {
  CURSOR_BOUND_PARSE(parseNullableBoundFloat, c, 1.0f, (float)VOICES_SIZE);
  if (isnan(c->val.f)) {
    warnx("Bottom-end zoning %p", (void *)co);
  } else {
    warnx("Even-split zoning %p", (void *)co);
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

static void evalPlus(CmdPlus cmd, Cursor *c, Control *co) {
  switch (cmd) {
    case CMD_PLUS_ZONE:
      evalZone(c, co);
      break;
  }
}

void eval(Cursor *c, Control *co) {
  Cmd cmd = c->val.n;
  if      (IS_CMD_DOT(cmd))   { return; }
  else if (IS_CMD_COLON(cmd)) { return; }
  else if (IS_CMD_PLUS(cmd))  { evalPlus(CMD_CHAR(cmd), c, co); }
  else                        { evalPure(CMD_CHAR(cmd), c, co); }
}
