#include "audio.h"
#include "buffer.h"

void audio(Audio *a) {
  /* Eventually init sndio here */
  int sndio_bufsize = 480;
  audioBuffer(&a->buffer, sndio_bufsize);
}
