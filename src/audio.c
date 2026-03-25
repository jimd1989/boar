#include <err.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <sndio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "audio.h"

#define AUDIO_RATE 48000
#define AUDIO_CHANS 2
#define AUDIO_BITS 16
#define STDIN_BUFFER_BLOCK_SIZE 4096
#define MIDI_BUFFER_SIZE 1024
#define STDIN_IDX 0
#define SNDIO_OUT_IDX 1
#define TEXT_FD_LIMIT 1
#define AUDIO_FD_LIMIT 4
#define MIDI_FD_LIMIT 4
#define FD_LIMIT (TEXT_FD_LIMIT + AUDIO_FD_LIMIT + MIDI_FD_LIMIT)

typedef struct StdinBuffer {
  int     sizeBlocks;
  int     readPos;
  char    block[STDIN_BUFFER_BLOCK_SIZE];
  char  * data;
} StdinBuffer;

typedef struct AudioBuffer {
  int               fdIdx;
  int               dspSizeBytes;
  int               writeSizeBytes;
  int               dspPos;
  int               writePos;
  struct sio_hdl  * sio;
  struct sio_par    parameters;
  uint8_t         * dspData;
  uint8_t         * writeData;
  void              (*schemeCallback)(int);
} AudioBuffer;

typedef struct MidiBuffer {
  int               fdIdx;
  struct mio_hdl  * mio;
  void              (*schemeCallback)(int);
  uint8_t         * midiData;
} MidiBuffer;

static struct pollfd POLLFDS[FD_LIMIT]           = {0};
static StdinBuffer STDIN_BUFFER                  = {0};
static AudioBuffer AUDIO_BUFFERS[AUDIO_FD_LIMIT] = {0};
static MidiBuffer MIDI_BUFFERS[MIDI_FD_LIMIT]    = {0};

void stdin_init(void) {
  int blockSize = 2;
  int flags     = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  POLLFDS[STDIN_IDX].fd     = STDIN_FILENO;
  POLLFDS[STDIN_IDX].events = POLLIN;
  STDIN_BUFFER.sizeBlocks   = blockSize;
  STDIN_BUFFER.data         = malloc(blockSize * STDIN_BUFFER_BLOCK_SIZE);
}

void stdin_read() {
  int bytesRead    = 0;
  int currentBlock = 0;
  StdinBuffer *sb  = &STDIN_BUFFER;
  char *newBuffer  = NULL;
  sb->readPos      = 0;
  do {
    currentBlock++;
    if (currentBlock > sb->sizeBlocks) {
      newBuffer       = malloc(sb->sizeBlocks * 2 * STDIN_BUFFER_BLOCK_SIZE);
      memcpy(newBuffer, sb->data, sb->sizeBlocks * STDIN_BUFFER_BLOCK_SIZE);
      free(sb->data);
      sb->data        = newBuffer;
      sb->sizeBlocks *= 2;
    }
    bytesRead    = read(STDIN_FILENO, &sb->data[sb->readPos], 
                        STDIN_BUFFER_BLOCK_SIZE - 1);
    if (bytesRead > 0) { 
      sb->readPos += bytesRead;
    }
  } while (bytesRead > 0);
  sb->data[sb->readPos] = '\0';
  if (sb->readPos > 1) {
    stdin_eval(STDIN_BUFFER.data);
  }
}

struct mio_hdl * midi_init(int idx, uint8_t *buffer, char *name, bool in, 
                           bool out) {
  int mode              = (in ? MIO_IN : 0) | (out ? MIO_OUT : 0);
  struct mio_hdl *mio   = NULL;
  MidiBuffer *mb        = NULL;
  void (*callback)(int) = idx == 1 ? mio_1_callback :
                          idx == 2 ? mio_2_callback :
                          idx == 3 ? mio_3_callback :
                          /* else */ mio_0_callback ;
  if (idx < 0 || idx >= MIDI_FD_LIMIT) {
    warnx("%d midi devices available, requested #%d", MIDI_FD_LIMIT, idx + 1);
  }
  mb  = &MIDI_BUFFERS[idx];
  mio = mio_open(name, mode, true);
  if (mio == NULL) {
    warnx("could not open midi device %s", name);
  }
  mb->mio            = mio;
  mb->fdIdx          = TEXT_FD_LIMIT + AUDIO_FD_LIMIT + idx;
  mb->schemeCallback = callback;
  mb->midiData       = buffer;
  mio_pollfd(mb->mio, &POLLFDS[mb->fdIdx], POLLIN | POLLOUT);
  return mb->mio;
}

int midi_write(int idx, uint8_t *buffer, int bytes) {
  int mask = 0;
  int bytesWritten = 0;
  int bytesToWrite = 0;
  struct pollfd pfd[1] = {0};
  MidiBuffer *mb = &MIDI_BUFFERS[idx];
  while (bytesWritten < bytes) {
    bytesToWrite = bytes > MIDI_BUFFER_SIZE ? MIDI_BUFFER_SIZE : bytes;
    mio_pollfd(mb->mio, pfd, POLLOUT);
    poll(pfd, 1, -1);
    mask = mio_revents(mb->mio, pfd);
    if (mask & POLLOUT) {
      bytesWritten += mio_write(mb->mio, buffer, bytesToWrite); 
    }
  }
  return bytesWritten;
}

void audio_callback(void *arg, int deltaFrames) {
  AudioBuffer *ob = (AudioBuffer *)arg;
  int chans       = ob->parameters.pchan;
  int byteDepth   = ob->parameters.bits >> 3;
  int deltaBytes  = deltaFrames * chans * byteDepth;
  ob->schemeCallback(deltaBytes);
}

void fill_silence(AudioBuffer *ob) {
  /* Meant for pre-filling the buffer. For whatever reason it does not
     seem to trigger callbacks, so init ob->writePos at 0 for maximum
     distance from ob->dspPos. */
  int i            = 0;
  int n            = ob->dspSizeBytes / ob->writeSizeBytes;
  int bytesWritten = 0;
  struct pollfd pfd[1] = {0};
  for (i = 0 ; i <= n ; i++) {
    sio_pollfd(ob->sio, pfd, POLLOUT);
    poll(pfd, 1, -1);
    if (sio_revents(ob->sio, pfd) & POLLOUT) {
      memset(ob->writeData, 0, ob->writeSizeBytes);
      bytesWritten += sio_write(ob->sio, ob->writeData, ob->writeSizeBytes);
    }
  }
  ob->writePos = (ob->writePos + bytesWritten) % ob->dspSizeBytes;
}

struct sio_hdl * audio_init(int idx, char *name, int rate, int outCh, int inCh, 
                            int bits, bool readWrite) {
  int bytes             = 0;
  struct sio_hdl *sio   = NULL;
  struct sio_par par    = {0};
  AudioBuffer *ab       = NULL;
  void (*callback)(int) = idx == 1 ? sio_1_callback :
                          idx == 2 ? sio_2_callback :
                          idx == 3 ? sio_3_callback :
                          /* else */ sio_0_callback ;
  if (idx < 0 || idx >= AUDIO_FD_LIMIT) {
    warnx("%d audio devices available, requested #%d", AUDIO_FD_LIMIT, idx + 1);
  }
  ab = &AUDIO_BUFFERS[idx];
  if (readWrite) {
    sio = sio_open(name, SIO_REC | SIO_PLAY, true);
  } else {
    sio = sio_open(name, SIO_PLAY, true);
  }
  if (sio == NULL) { 
    warnx("could not open audio output %s", name); 
    return NULL;
  }
  sio_initpar(&par);
  par.bits     = bits;
  par.appbufsz = 1; /* soundcard will overwrite with min size */
  par.rate     = rate;
  par.pchan    = outCh;
  par.rchan    = inCh;
  par.le       = 1;
  par.sig      = 1;
  sio_setpar(sio, &par);
  sio_getpar(sio, &par);
  bytes = par.bits >> 3;
  ab->fdIdx          = TEXT_FD_LIMIT + idx;
  ab->sio            = sio;
  ab->parameters     = par;
  ab->dspSizeBytes   = par.pchan * par.bufsz * bytes;
  ab->writeSizeBytes = par.pchan * par.appbufsz * bytes;
  ab->dspData        = malloc(ab->dspSizeBytes);
  ab->writeData      = malloc(ab->writeSizeBytes);
  ab->schemeCallback = callback;
  sio_onmove(sio, &audio_callback, (void *)ab);
  sio_start(sio);
  warnx("%dch %dHz %d frame buffer", par.pchan, par.rate, par.appbufsz);
  fill_silence(ab);
  return ab->sio;
}

void audio_close(struct sio_hdl *sio) {
  sio_stop(sio);
  sio_close(sio);
  sio = NULL;
}

void fill_dsp(int idx, uint8_t *data, int sizeBytes) {
  AudioBuffer *ob = &AUDIO_BUFFERS[idx];
  memcpy(&ob->dspData[ob->dspPos], data, sizeBytes);
  ob->dspPos = (ob->dspPos + sizeBytes) % ob->dspSizeBytes;
  //warnx("Δ %d → %d", sizeBytes, ob->dspPos);
}

static void audio_write(AudioBuffer *ob) {
  int i            = 0;
  int write_ix     = ob->writePos;
  int bytesWritten = 0;
  for (i = 0 ; i < ob->writeSizeBytes ; i++) {
    ob->writeData[i] = ob->dspData[write_ix];
    write_ix = (write_ix + 1) % ob->dspSizeBytes;
  }
  bytesWritten = sio_write(ob->sio, ob->writeData, ob->writeSizeBytes);
  ob->writePos = (ob->writePos + bytesWritten) % ob->dspSizeBytes;
}

void poll_io() {
  int i           = 0;
  int mask        = 0;
  int bytesRead   = 0;
  MidiBuffer *mb  = NULL;
  AudioBuffer *ob = NULL;
  /* seemingly must run every time for audio, but not for MIDI. */
  for (i = 0 ; i < AUDIO_FD_LIMIT ; i++) {
    ob = &AUDIO_BUFFERS[i];
    if (ob->sio != NULL) {
      sio_pollfd(ob->sio, &POLLFDS[ob->fdIdx], POLLIN | POLLOUT);
    }
  }
  poll(POLLFDS, FD_LIMIT, -1);
  if (POLLFDS[STDIN_IDX].revents & POLLIN) {
    stdin_read();
  }
  for (i = 0 ; i < MIDI_FD_LIMIT ; i++) {
    mb = &MIDI_BUFFERS[i];
    if (mb->mio != NULL) {
      mask = mio_revents(mb->mio, &POLLFDS[mb->fdIdx]);
      if (mask & POLLIN) {
        bytesRead = mio_read(mb->mio, mb->midiData, MIDI_BUFFER_SIZE);
        mb->schemeCallback(bytesRead);
      }
      if (mask & POLLOUT) {
        /* This probably is not right. Leaving branch here, but writes are
           likely a separate, direct call. */
        warnx("the MIDI POLLOUT was actually triggered");
        mio_write(mb->mio, mb->midiData, MIDI_BUFFER_SIZE);
      }
    }
  }
  for (i = 0 ; i < AUDIO_FD_LIMIT ; i++) {
    ob = &AUDIO_BUFFERS[i];
    if (ob->sio != NULL) {
      mask = sio_revents(ob->sio, &POLLFDS[ob->fdIdx]);
      if (mask & POLLIN) {
        sio_read(ob->sio, ob->writeData, ob->writeSizeBytes);
      }
      if (mask & POLLOUT) {
        audio_write(ob);
      }
    }
  }
}
