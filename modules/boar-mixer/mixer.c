#include <stdint.h>
#include <string.h>

#include "mixer.h"

/* Mixer uses contiguous float buffers for audio and parameter fading.
 * Audio is polyphonic master output, interleaved, then monophonic input
 * channels, contiguous. 
 *
 * For {a b c d} mono input channels, with stereo output:
 *
 * [ l0 r0 l1 r1 l2 r2 …
 *   a0    a1    a2    …
 *   b0    b1    b2    …
 *   c0    c1    c2    …
 *   d0    d1    d2    … ]
 *
 * Input parameters and master parameters are faded on separate buffers.
 * Parameters within an input channel are interleaved; the entire parameter fade
 * across each channel is not. Input channel parameters are:
 *   - 1 local volume
 *   - N master channel sends (balances)
 * 
 * For {a b} mono input channels, with stereo output:
 *
 * [ va0 la0 ra0 va1 la1 ra1 va2 la2 ra2 …
 *   vb0 lb0 rb0 vb1 lb1 rb1 vb2 lb2 rb2 … ]
 *
 * Separate functions exist for faded parameters and static parameters, to avoid
 * mixing against these buffers where possible. */
int mix_f32_fade(int fadeLen, int paramLen, float *params, int inputCh, 
                 int outputCh, int bufLen, float *audio) {
  int paramCount    = 1 + outputCh;
  int paramChLen    = paramCount * paramLen;
  int masterBufLen  = bufLen * outputCh; /* zero this section of buffer */
  int chIdx         = 0;
  int paramChIdx    = 0;
  int paramIdx      = 0;
  int masterBufIdx  = 0;
  int localBufIdx   = 0;
  int balIdx        = 0;
  float fadePhase   = 0.0f;
  float fadeInc     = (float)paramLen / (float)fadeLen;
  float vol         = 0.0f;
  float chDiv       = 1.0f / (float)inputCh;
  float bal         = 0.0f;
  float localSample = 0.0f;
  if (bufLen < fadeLen) {
    fadeLen = bufLen;
    fadeInc = (float)paramLen / (float)fadeLen;
  }
  for (chIdx = 0 ; chIdx < inputCh ; chIdx++) {
    /* Must check the start or end of fade curve to see if channel is on. 
     * Skip otherwise. */
    if ((params[chIdx        * paramChLen     ] > 0.0f) ||
        (params[((chIdx + 1) * paramChLen) - 1] > 0.0f)  ) {
      for (
           /* init */
           fadePhase     = 0.0f                            ,
           localBufIdx   = masterBufLen + (chIdx * bufLen) ,
           paramChIdx    = chIdx * paramChLen              ,
           masterBufIdx  = 0                               ;
           /* cond */
           fadePhase     < fadeLen                         ;
           /* iterate */
           fadePhase    += fadeInc                          ) {
        paramIdx    = paramChIdx + ((int)fadePhase * paramCount); /* no lerp */
        vol         = params[paramIdx];
        localSample = audio[localBufIdx++];
        for (balIdx = 1 ; balIdx <= outputCh ; balIdx++) {
          bal                    = params[paramIdx + balIdx];
          audio[masterBufIdx++] += chDiv * vol * bal * localSample;
        }
      } 
    }
  }
  return fadeLen;
}

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
  int balIdx           = 0;
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
        for (balIdx = 0 ; balIdx < outputCh ; balIdx++) {
          balance = balances[balIdx + ((1 + inputChannelIdx) * outputCh)];
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
  float vol        = volumes[0];
  float sampleF    = 0.0f;
  int16_t sample   = 0;
  mix_f32(inputCh, outputCh, bufLen, volumes, balances, audio);
  for (masterIdx = 0 ; masterIdx < masterBufLen ; masterIdx++) {
    sampleF             = audio[masterIdx];
    sampleF            *= vol * balances[masterIdx % outputCh];
    sample              = sampleF * 32767.0f; /* eventually dither */
    output[outputIdx++] = sample & 255;
    output[outputIdx++] = sample >> 8;
  }
}
