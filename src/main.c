/* Initializes the Audio struct and launches audio output in its own POSIX
 * thread. Then starts a REPL to accept user input. */

#include <err.h>
#include <pthread.h>

#include "audio-init.h"
#include "audio-output.h"
#include "errors.h"
#include "repl.h"

int main(int argc, char **argv) {
  pthread_t p = NULL;
  Audio a = {0};
  Repl r = {0};
  Error err = ERROR_OK;

  makeAudio(&a, argc, argv);
  err = pthread_create(&p, NULL, audioLoop, (void *)&a);
  if (err != ERROR_OK) {
    errx(err, "Error launching audio thread");
  }
  r.EchoNotes = a.Settings.EchoNotes;
  r.Audio = &a;
  repl(&r);
  err = pthread_join(p, NULL);
  if (err != ERROR_OK) {
    errx(err, "Error closing audio thread");
  }
  killAudio(&a);
  return 0;
}
