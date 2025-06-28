#include <err.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "cursor.h"
#include "parameter.h"

void args(Args *a) {
  a->chan            = 2;
  a->maxEnvTime      = 10.0;
  a->audioOutputFile = NULL;
  a->infoOutputFile  = NULL;
}

Cursor parseArgs(int argc, char **argv, Args *a) {
  int i = 1; /* Ignore program name */
  Cursor c = cursor(argv[0]);
  for (; i < argc ; i++) {
    if (strcmp("-maxEnvTime", argv[i]) == 0) {
      if (i + 1 >= argc) {
        warnx("Expected argument for %s", argv[i]);
        c.breakReason = CURSOR_ERROR;
        return c;
      }
      c = cursor(argv[++i]);
      parseFloat(&c);
      if (c.breakReason == CURSOR_ERROR) {
        warnx("Could not parse arg %s", argv[i]);
        return c;
      }
      a->maxEnvTime = c.val.f;
    }
    else if (strcmp("-chan", argv[i]) == 0) {
      if (i + 1 >= argc) {
        warnx("Expected argument for %s", argv[i]);
        c.breakReason = CURSOR_ERROR;
        return c;
      }
      c = cursor(argv[++i]);
      parseInt(&c);
      if (c.breakReason == CURSOR_ERROR) {
        warnx("Could not parse arg %s", argv[i]);
        return c;
      }
      a->chan = c.val.n;
    }
    else if (strcmp("-audioOutputFile", argv[i]) == 0) {
      if (i + 1 >= argc) {
        warnx("Expected argument for %s", argv[i]);
        c.breakReason = CURSOR_ERROR;
        return c;
      }
      c = cursor(argv[++i]);
      parseString(&c);
      if (c.breakReason == CURSOR_ERROR) {
        warnx("Could not parse arg %s", argv[i]);
        return c;
      }
      a->audioOutputFile = c.val.s;
    }
    else if (strcmp("-infoOutputFile", argv[i]) == 0) {
      if (i + 1 >= argc) {
        warnx("Expected argument for %s", argv[i]);
        c.breakReason = CURSOR_ERROR;
        return c;
      }
      c = cursor(argv[++i]);
      parseString(&c);
      if (c.breakReason == CURSOR_ERROR) {
        warnx("Could not parse arg %s", argv[i]);
        return c;
      }
      a->audioOutputFile = c.val.s;
    }
    else {
      warnx("Invalid argument %s", argv[i]);
      c.breakReason = CURSOR_ERROR;
      return c;
    }
  }
  c.breakReason = CURSOR_LINE_END;
  return c;
}
