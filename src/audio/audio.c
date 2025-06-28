#include "../control/control.h"
#include "../parse/args.h"
#include "audio.h"
#include "buffer.h"
#include "output_buffer.h"
#include "sndio.h"

void audio(Audio *a, Control *c, Args *ar) {
  sio(&a->sio,ar);
  audioBuffer(&a->buffer, c, a->sio.bufSizeFrames, a->sio.chan);
  sio_onmove(a->sio.port, &generateDsp, (void *)(&a->buffer));
  startSio(&a->sio);
}

void writeAudio(Audio *a) {
  fillOutputBuffer(&a->buffer.o);
  writeSio(&a->sio, a->buffer.o.output);
}

void stopAudio(Audio *a) {
  stopSio(&a->sio);
  freeAudioBuffer(&a->buffer);
}
