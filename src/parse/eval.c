#include "eval.h"
#include "eval_dot.h"
#include "eval_plus.h"
#include "eval_pure.h"
#include "parameter.h"

void eval(Cursor *c, Control *co) {
  Cmd cmd = c->val.n;
  if      (IS_CMD_DOT(cmd))   { evalDot(CMD_CHAR(cmd), c, co);  }
  else if (IS_CMD_COLON(cmd)) { return; }
  else if (IS_CMD_PLUS(cmd))  { evalPlus(CMD_CHAR(cmd), c, co); }
  else                        { evalPure(CMD_CHAR(cmd), c, co); }
}
