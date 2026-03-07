#include <stdint.h>
#include <string.h>

#include "mixer.h"

/* All mixer audio is a contiguous stretch of floats, with the master
 * multi-channel mix reserved at the front of the buffer, and each monophonic
 * mixer input found after the offset `masterBufLen`. Each monophonic input
 * channel's sample is mixed to master `outputCh` times. */
static void mix_f32(int inputCh, int outputCh, int bufLen, float *volumes,
             float *balances, float *audio) {
  int masterBufLen     = outputCh * bufLen;
  int totalIdx         = masterBufLen;
  int inputChannelIdx  = 0;
  int localBufIdx      = 0;
  int masterAudioIdx   = 0;
  int balanceIdx       = 0;
  float chanDiv        = 1.0f / (float)inputCh;
  float vol            = 0.0f;
  float masterBalance  = 0.0f;
  float balance        = 0.0f;
  /* zero out master buffer for a new mix */
  memset((void *)audio, 0, masterBufLen * sizeof(float));
  /* mixing all monophonic input channels to master */
  for (inputChannelIdx = 0 ; inputChannelIdx < inputCh ; inputChannelIdx++) {
    masterAudioIdx = 0;
    vol            = volumes[inputChannelIdx + 1];
    if (vol > 0.0f) {
      /* mixing a single monophonic input channel to master */
      for (localBufIdx = 0 ; localBufIdx < bufLen ; localBufIdx++) {
        /* mixing a single input channel sample to master `outputCh` times */
        for (balanceIdx = 0 ; balanceIdx < outputCh ; balanceIdx++) {
          balance = balances[balanceIdx + ((1 + inputChannelIdx) * outputCh)];
          audio[masterAudioIdx++] += chanDiv * vol * balance * audio[totalIdx];
        }
        totalIdx++;
      }
    } else {
      /* skip entire input channel */
      totalIdx += bufLen;
    }
  }
}

void mix_s16(int inputCh, int outputCh, int bufLen, float *volumes,
             float *balances, float *audio, uint8_t *output) {
  int masterBufLen = bufLen * outputCh;
  int masterIdx    = 0;
  int outputIdx    = 0;
  int16_t sample   = 0;
  mix_f32(inputCh, outputCh, bufLen, volumes, balances, audio);
  for (masterIdx = 0 ; masterIdx < masterBufLen ; masterIdx++) {
    sample              = audio[masterIdx] * 32767.0f; /* eventually dither */
    output[outputIdx++] = sample & 255;
    output[outputIdx++] = sample >> 8;
  }
}
