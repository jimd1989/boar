/* Initializes the Audio struct, then starts a REPL to accept user input. */

#include "audio-init.h"
#include "audio-output.h"
#include "repl.h"

int main(int argc, char **argv) {
  Audio a = {0};
  Repl r = {{0}};

  makeAudio(&a, argc, argv);
  r.Audio = &a;
  repl(&r);
  killAudio(&a);
  return 0;
}
