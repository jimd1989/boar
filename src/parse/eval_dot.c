#include "../control/control.h"
#include "cmd.h"
#include "cursor.h"
#include "eval_dot.h"
#include "parameter.h"

static void evalMixBalance(Cursor *, Control *);

static void evalMixBalance(Cursor *c, Control *co) {
  /* v. n … → set zone mixer vol for channels n … */
  int i = 0;
  for (; i < co->args.chan ; i++) {
    /* It's okay to provide less parameters than channels. */
    if (c->breakReason & (CURSOR_WAITING_PARAMS | CURSOR_PARAMETER_END)) {
      CURSOR_BOUND_PARSE(parseBoundFloat, c, 0.0f, 1.0f);
    }
  }
  /* It's not okay to provide more parameters than channels */
  if (!(c->breakReason & (CURSOR_CMD_END | CURSOR_LINE_END))) { 
    c->breakReason = CURSOR_ERROR;
  }
}

void evalDot(CmdDot cmd, Cursor *c, Control *co) {
  switch (cmd) {
    case CMD_DOT_VOL:
      evalMixBalance(c, co);
      break;
  }
}
