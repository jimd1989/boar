(import (chicken file posix) (chicken foreign) srfi-18)

#>
#include <err.h>
#include <poll.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define FD_LIMIT 32
static struct pollfd POLLFDS[FD_LIMIT]  = {0};
static void(*SIGNALS[FD_LIMIT])(void *) = {0};
static void *HDLS[FD_LIMIT]             = {0};
static int FD_POPULATION                = 0;

void init_stdin(void(*signal)(void *)) {
  if (FD_POPULATION >= FD_LIMIT) {
    warnx("fd limit %d exceeded", FD_POPULATION);
    return;
  }
  POLLFDS[FD_POPULATION].fd     = STDIN_FILENO;
  POLLFDS[FD_POPULATION].events = POLLIN;
  SIGNALS[FD_POPULATION]        = signal;
  HDLS[FD_POPULATION]           = NULL;
  FD_POPULATION++;
}

void init_audio_out(void(*signal)(void *), char *name) {
  /* still need parameters */
  struct sio_hdl *s = NULL;
  if (FD_POPULATION >= FD_LIMIT) {
    warnx("fd limit %d exceeded", FD_POPULATION);
    return;
  }
  s = sio_open(name, SIO_PLAY, true);
  if (s == NULL) {
    warnx("could not open audio output %s", name);
    return;
  } 
  sio_pollfd(s, &POLLFDS[FD_POPULATION], POLLIN);
  SIGNALS[FD_POPULATION] = signal;
  HDLS[FD_POPULATION]    = (void *)s;
  FD_POPULATION++;
}

void poll_io() {
  int i = 0;
  poll(POLLFDS, FD_POPULATION, 0);
  for (i = 0 ; i < FD_POPULATION ; i++) {
    if (POLLFDS[i].revents & POLLFDS[i].events) { 
      POLLFDS[i].revents = 0;
      SIGNALS[i](HDLS[i]);
    }
  }
}
<#

;(define-foreign-type sio (c-pointer (struct "sio_hdl")))
(define STDIN-COND (make-condition-variable))
(define-external (stdin_signal (c-pointer x)) void (condition-variable-broadcast! STDIN-COND))
(define init-stdin
  (foreign-safe-lambda void "init_stdin" (function void (c-pointer))))
(define poll-io (foreign-safe-lambda void "poll_io"))
(define (io)
  ;(print 'awaiting-io)
  (poll-io) 
  (io))
(define (stdin mutex condition)
  (mutex-lock! mutex)
  (print 'stdin-awakened)
  (print (eval (read)))
  (mutex-unlock! mutex condition)
  (stdin mutex condition))
(define (clock)
  (print 'tick)
  (thread-sleep! 1)
  (clock))
(define (new-io)
  (thread-wait-for-i/o! fileno/stdin #:input)
  (print (eval (read)))
  (new-io))
;(init-stdin (location stdin_signal))
(define clock-thread (make-thread (lambda () (clock))))
(define stdin-thread (make-thread (lambda () (new-io))))
;(define stdin-thread (make-thread (lambda () (stdin (make-mutex) STDIN-COND))))
;(define main-thread (make-thread (lambda () (io))))
(thread-start! clock-thread)
;(thread-start! main-thread)
(thread-start! stdin-thread)
(thread-join! stdin-thread)
