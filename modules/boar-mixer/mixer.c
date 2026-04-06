#include <math.h>
#include <stdint.h>
#include <string.h>

#include "mixer.h"

void mixer_zero(int outputCh, int bufLen, float *audio) {
  memset((void *)audio, 0, sizeof(float) * bufLen * outputCh);
}

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
 * Parameters within an input channel are interleaved; the entire parameter fade
 * across each channel is not. Input channel parameters are:
 *   - 1 local volume
 *   - N master channel sends (balances)
 * 
 * For master params and {a b} mono input channels, with stereo output:
 *
 * [  v0  l0  r0  v1  l1  r1  v2  l2  r2 …
 *   va0 la0 ra0 va1 la1 ra1 va2 la2 ra2 …
 *   vb0 lb0 rb0 vb1 lb1 rb1 vb2 lb2 rb2 … ]
 *
 * Separate functions exist for faded parameters and static parameters, to avoid
 * mixing against these buffers where possible. */
int mix_f32_fade(int fadeLen, int paramLen, float *params, int inputCh, 
                 int outputCh, int bufLen, float *audio) {
  int paramCount        = 1 + outputCh;
  int paramChLen        = paramCount * paramLen;
  int masterParamOffset = paramChLen;
  int masterBufLen      = bufLen * outputCh; /* zero this section of buffer */
  int fadeIdx           = 0;
  int chIdx             = 0;
  int paramChIdx        = 0;
  int paramIdx          = 0;
  int masterBufIdx      = 0;
  int localBufIdx       = 0;
  int balIdx            = 0;
  float fadePhase       = 0.0f;
  float fadeInc         = 0.0f;
  float chDiv           = 1.0f / (float)inputCh;
  float vol             = 0.0f;
  float bal             = 0.0f;
  float localSample     = 0.0f;
  if (bufLen < fadeLen) {
    fadeLen = bufLen;
  }
  fadeInc = (float)paramLen / (float)fadeLen;
  for (chIdx = 0 ; chIdx < inputCh ; chIdx++) {
    /* Must check the start or end of volume fade curve to see if channel is on. 
     * Skip otherwise. */
    paramChIdx = masterParamOffset + (chIdx * paramChLen);
    if (params[paramChIdx                            ] > 0.0f ||
        params[paramChIdx + (paramChLen - paramCount)] > 0.0f  ) {
      for (
           /* init */
           fadeIdx       = 0                               ,
           localBufIdx   = masterBufLen + (chIdx * bufLen) ,
           masterBufIdx  = 0                               ;
           /* cond */
           fadeIdx       < fadeLen                         ;
           /* iterate */
           fadeIdx++                                       ,
           fadePhase    += fadeInc                          ) {
        paramIdx    = paramChIdx + ((int)fadePhase * paramCount); /* no lerp */
        vol         = params[paramIdx] * chDiv;
        localSample = audio[localBufIdx++];
        for (balIdx = 1 ; balIdx < paramCount ; balIdx++) {
          bal                    = params[paramIdx + balIdx];
          audio[masterBufIdx++] += vol * bal * localSample;
        }
      } 
    }
  }
  return fadeLen;
}

/* Mixes with interleaved, but non-faded volume/balance params: ie a one
 * dimensional array of:
 *
 * [ v0 l0 r0 va1 la1 ra1 vb2 lb2 rb2 … ] 
 *
 * Starts at `offset` for each input channel, which could be `fadeLen` if a 
 * fade has taken place, or 0 if no parameters were recently changed. */
void mix_f32(int offset, float *staticParams, int inputCh, int outputCh,
             int bufLen, float *audio) {
  int i                 = 0;
  int paramCount        = 1 + outputCh;
  int masterParamOffset = paramCount;
  int masterBufLen      = bufLen * outputCh; /* zero this section of buffer */
  int masterBufOffset   = offset * outputCh;
  int localBufLen       = bufLen - offset;
  int chIdx             = 0;
  int paramIdx          = 0;
  int masterBufIdx      = 0;
  int localBufIdx       = 0;
  int balIdx            = 0;
  float chDiv           = 1.0f / (float)inputCh;
  float vol             = 0.0f;
  float bal             = 0.0f;
  float localSample     = 0.0f;
  for (chIdx = 0 ; chIdx < inputCh ; chIdx++) {
    paramIdx = masterParamOffset + (chIdx * paramCount);
    vol      = staticParams[paramIdx] * chDiv;
    if (vol > 0.0f) {
      localBufIdx = masterBufLen + offset + (chIdx * bufLen);
      for (masterBufIdx = masterBufOffset, i = 0 ; i < localBufLen ; i++) {
        localSample = audio[localBufIdx++];
        for (balIdx = 1 ; balIdx < paramCount ; balIdx++) {
          bal                   = staticParams[paramIdx + balIdx];
          audio[masterBufIdx++] += vol * bal * localSample;
        }
      }
    }
  }
}

int mix_s16_fade(int fadeLen, int paramLen, float *params, int outputCh, 
                 int bufLen, float *audio, uint8_t *output) {
  int paramCount   = 1 + outputCh;
  int masterBufLen = bufLen * outputCh; /* zero this section of buffer */
  int fadeIdx      = 0;
  int paramIdx     = 0;
  int masterBufIdx = 0;
  int outputIdx    = 0;
  int balIdx       = 0;
  float fadePhase  = 0.0f;
  float fadeInc    = 0.0f;
  float vol        = 0.0f;
  float bal        = 0.0f;
  float sample     = 0.0f;
  int16_t s        = 0;
  if (bufLen < fadeLen) {
    fadeLen = bufLen;
  }
  fadeInc = (float)paramLen / (float)fadeLen;
  for (fadeIdx = 0 ; fadeIdx < fadeLen ; fadeIdx++, fadePhase += fadeInc) {
    paramIdx = paramCount * (int)fadePhase; /* no lerp */
    vol      = params[paramIdx];
    for (balIdx = 1 ; balIdx < paramCount ; balIdx++) {
      bal                 = params[paramIdx + balIdx];
      sample              = vol * bal * audio[masterBufIdx++] * 32767.0f;
      sample              = fmaxf(-32768.0f, fminf(32767.0f, sample));
      s                   = (int16_t)sample; /* no dither yet */
      output[outputIdx++] = s & 255;
      output[outputIdx++] = s >> 8;
    }
  }
  return fadeLen;
}

void mix_s16(int offset, float *staticParams, int outputCh, int bufLen, 
             float *audio, uint8_t *output) {
  int paramCount      = 1 + outputCh;
  int masterBufLen    = bufLen * outputCh; /* zero this section of buffer */
  int masterBufOffset = offset * outputCh;
  int masterBufIdx    = masterBufOffset;
  int outputIdx       = masterBufOffset * sizeof(int16_t);
  int balIdx          = 0;
  float vol           = staticParams[0];
  float bal           = 0.0f;
  float sample        = 0.0f;
  int16_t s           = 0;
  while (masterBufIdx < masterBufLen) {
    for (balIdx = 1 ; balIdx < paramCount ; balIdx++) {
      bal                 = staticParams[balIdx];
      sample              = vol * bal * audio[masterBufIdx++] * 32767.0f;
      sample              = fmaxf(-32768.0f, fminf(32767.0f, sample));
      s                   = (int16_t)sample; /* no dither yet */
      output[outputIdx++] = s & 255;
      output[outputIdx++] = s >> 8;
    }
  }
}
