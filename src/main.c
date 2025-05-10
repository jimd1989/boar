#include "parse/args.h"
#include "parse/cursor.h"
#include "parse/repl.h"

int main(int argc, char **argv) {
  Args a   = {0};
  Cursor c = {0};
  args(&a);
  c = parseArgs(argc, argv, &a);
  if (c.breakReason != CURSOR_LINE_END) { return 1; }
  /* Audio and synth init */
  repl();
  return 0;
}
