#pragma once

#include "../parse/args.h"
#include <sndio.h>
#include <stdint.h>

typedef struct sio_hdl *SioHdl;

typedef struct Sio {
  int     chan;
  int     bufSizeFrames;
  int     bufSizeBytes;
  int     nfds;
  SioHdl  port;
} Sio;

void sio(Sio *, Args *);
void startSio(Sio *);
int writeSio(Sio *, uint8_t *);
void stopSio(Sio *);
