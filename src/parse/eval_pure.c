#include "../control/control.h"
#include "cmd.h"
#include "cursor.h"
#include "eval_pure.h"
#include "parameter.h"

static void evalVol(Cursor *, Control *);
static void evalToggleZone(Cursor *, Control *);
static void evalComment(Cursor *);
static void evalNoteOff(Cursor *, Control *);
static void evalNoteOn(Cursor *, Control *);

static void evalVol(Cursor *c, Control *co) {
  /* v n → Set master volume to n */
  CURSOR_BOUND_PARSE(parseBoundFloat, c, 0.0f, 1.0f);
  setVol(&co->vol, c->val.f);
}

static void evalComment(Cursor *c) {
  /* # … → Ignore everything afterwards */
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

static void evalToggleZone(Cursor *c, Control *co) {
  /* z n → Set active zone to n */
  CURSOR_BOUND_PARSE(parseBoundInt, c, 1, VOICES_SIZE);
  co->voiceZones.currentZone = c->val.n - 1; 
}

void evalPure(CmdPure cmd, Cursor *c, Control *co) {
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
      break;
    case CMD_WAVE:
      break;
    case CMD_COMMENT:
      evalComment(c);
      break;
  }
}
