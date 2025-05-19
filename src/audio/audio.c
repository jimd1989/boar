#include "audio.h"
#include "buffer.h"
#include "output_buffer.h"
#include "sndio.h"

void audio(Audio *a) {
  sio(&a->sio);
  audioBuffer(&a->buffer, a->sio.bufSizeFrames);
  sio_onmove(a->sio.port, &generateDsp, (void *)(&a->buffer));
  startSio(&a->sio);
}

void writeAudio(Audio *a) {
  fillOutputBuffer(&a->buffer.outputBuffer);
  //fillBuffer(&a->buffer);
  writeSio(&a->sio, a->buffer.outputBuffer.output);
}

void stopAudio(Audio *a) {
  stopSio(&a->sio);
  freeAudioBuffer(&a->buffer);
}
