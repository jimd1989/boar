#include <math.h>

#include "../control/control.h"
#include "cmd.h"
#include "cursor.h"
#include "eval_plus.h"
#include "parameter.h"

#define IS_FLOAT_NULL isnan

static void evalSplitZone(Cursor *, Control *);

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

void evalPlus(CmdPlus cmd, Cursor *c, Control *co) {
  switch (cmd) {
    case CMD_PLUS_ZONE:
      evalSplitZone(c, co);
      break;
  }
}
