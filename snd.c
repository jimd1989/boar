#include <err.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define AUDIO_BLOCK_SIZE_FRAMES 64
#define AUDIO_CHANS 2
#define AUDIO_BITS 16
#define AUDIO_BLOCK_SIZE_BYTES (AUDIO_BLOCK_SIZE_FRAMES * AUDIO_CHANS * (AUDIO_BITS / 8))
#define STDIN_BUFFER_SIZE 4096
#define FD_LIMIT 32

static int FD_POPULATION                                   = 0;
static int SIO_POPULATION                                  = 0;
static struct pollfd POLLFDS[FD_LIMIT]                     = {0};
static struct sio_hdl *SIO_HDLS[FD_LIMIT]                  = {0};
static uint8_t STDIN_BUFFER[STDIN_BUFFER_SIZE]             = {0};
static uint8_t AUDIO_OUTPUT_BUFFER[AUDIO_BLOCK_SIZE_BYTES] = {0};

void init_stdin(void) {
  int i     = 0;
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  for (i = 0 ; i < FD_LIMIT ; i++) { POLLFDS[i].fd = -1; }
  POLLFDS[0].fd     = STDIN_FILENO;
  POLLFDS[0].events = POLLIN;
  FD_POPULATION++;
}

void init_audio_output(char *name) {
  struct sio_hdl *sio = NULL;
  sio = sio_open(name, SIO_PLAY, true);
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return;
  }
  /* bounds check somewhere */
  FD_POPULATION += sio_pollfd(sio, POLLFDS + FD_POPULATION, POLLOUT);
  SIO_HDLS[SIO_POPULATION++] = sio;
  sio_start(sio);
  warnx("audio FD %d opened", POLLFDS[FD_POPULATION - 1].fd);
}

void io(void) {
  int i                       = 0;
  int mask                    = 0;
  struct pollfd *stdin_pollfd = &POLLFDS[0];
  struct pollfd hdl_pollfd[1] = {0};
  struct sio_hdl *sio         = NULL;
  warnx("waiting on any event from %d FDs", FD_POPULATION);
  poll(POLLFDS, FD_POPULATION, -1);
  if (stdin_pollfd->revents & POLLIN) {
    warnx("stdin ready");
    read(STDIN_FILENO, STDIN_BUFFER, STDIN_BUFFER_SIZE - 1);
    printf("%s\n", STDIN_BUFFER);
    /* (eval) callback eventually */
  }
  /* MIO HDL LOOP eventually */
  for (i = 0 ; i < SIO_POPULATION ; i++) {
    warnx("Checking audio device %d", i);
    sio  = SIO_HDLS[i];
    mask = sio_revents(sio, hdl_pollfd);
    warnx("Mask is %d", mask);
    if (mask & POLLOUT) {
      warnx("writing %d bytes of audio", AUDIO_BLOCK_SIZE_BYTES);
      sio_write(sio, AUDIO_OUTPUT_BUFFER, AUDIO_BLOCK_SIZE_BYTES);
    }
  }
}

int main(void) {
  init_stdin();
  init_audio_output("default");
  while (1) {
    io();
  }
  return 0;
}
