#include "audio.h"
#include "buffer.h"
#include "sndio.h"

void audio(Audio *a) {
  sio(&a->sio);
  audioBuffer(&a->buffer, a->sio.bufSizeFrames);
  startSio(&a->sio);
}

void writeAudio(Audio *a) {
  generateDsp(&a->buffer);
  fillBuffer(&a->buffer);
  a->buffer.framesWritten += writeSio(&a->sio, a->buffer.output);
}

void stopAudio(Audio *a) {
  stopSio(&a->sio);
  freeAudioBuffer(&a->buffer);
}
