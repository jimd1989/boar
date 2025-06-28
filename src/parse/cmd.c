#include <err.h>
#include <stdbool.h>
#include <stdint.h>

#include "cmd.h"
#include "cursor.h"

static void enlistCmd(int, uint32_t *);
static void pureCmdAlphabet(uint32_t *);
static void dotCmdAlphabet(uint32_t *);
static void plusCmdAlphabet(uint32_t *);
static bool cmdExists(int, uint32_t[4]);

static void enlistCmd(int cmd, uint32_t *alphabet) {
  int quadrant = CMD_CHAR(cmd) >> 5;
  int bit      = cmd & 31;
  alphabet[quadrant] |= (1u << bit);
}

static bool cmdExists(int cmd, uint32_t alphabet[4]) {
  int quadrant = CMD_CHAR(cmd) >> 5;
  int bit      = cmd & 31;
  return (alphabet[quadrant] & (1u << bit)) != 0;
}

static void pureCmdAlphabet(uint32_t *alphabet) {
  enlistCmd(CMD_ATTACK, alphabet);
  enlistCmd(CMD_COMMENT, alphabet);
  enlistCmd(CMD_NOTE_ON, alphabet);
  enlistCmd(CMD_NOTE_OFF, alphabet);
  enlistCmd(CMD_VOL, alphabet);
  enlistCmd(CMD_WAVE, alphabet);
  enlistCmd(CMD_ZONE, alphabet);
}

static void dotCmdAlphabet(uint32_t *alphabet) {
  enlistCmd(CMD_DOT_VOL, alphabet);
}

static void plusCmdAlphabet(uint32_t *alphabet) {
  enlistCmd(CMD_PLUS_ZONE, alphabet);
}

void cmdAlphabet(CmdAlphabet *ca) {
  pureCmdAlphabet(ca->pure);
  dotCmdAlphabet(ca->dot);
  plusCmdAlphabet(ca->plus);
}

void parseCmd(Cursor *c, CmdAlphabet a) {
  char head      = CURSOR_HEAD(c);
  char decorator = '\0';
  c->pos++;
  decorator = CURSOR_HEAD(c);
  c->breakReason = CURSOR_WAITING_PARAMS;
  if (decorator == '.') {
    if (cmdExists(head, a.dot)) {
      c->val.n = CMD_DOT(head); 
      c->pos++; 
    } else {
      warnx("cmd not found %c.", head);
      c->breakReason = CURSOR_ERROR;
    }
  } else if (decorator == ':') {
    if (cmdExists(head, a.colon)) {
      c->val.n = CMD_COLON(head); 
      c->pos++; 
    } else {
      warnx("cmd not found %c:", head);
      c->breakReason = CURSOR_ERROR;
    }
  } else if (decorator == '+') {
    if (cmdExists(head, a.plus)) {
      c->val.n = CMD_PLUS(head); 
      c->pos++; 
    } else {
      warnx("cmd not found %c:", head);
      c->breakReason = CURSOR_ERROR;
    }
  } else { 
    if (cmdExists(head, a.pure)) {
      c->val.n = CMD_PURE(head); 
    } else {
      warnx("cmd not found %c", head);
      c->breakReason = CURSOR_ERROR;
    }
  } 
}
