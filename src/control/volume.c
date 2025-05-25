#include "volume.h"

void volume(Volume *v, float *curve, int len, int chunkSize) {
  v->chunks    = 0;
  v->chunkSize = chunkSize;
  v->idx       = 0;
  v->inc       = 0;
  v->curveLen  = len;
  v->val       = 0.0f;
  v->curve     = curve;
}

void setVol(Volume *v, float f) {
  /* Immediately set v->val to new value, but calculate interpolation positions
   * and speed. The first block of a changed volume will be filled via
   * interpolated reads, while subsequent ones can just take v->val.
   * v->inc is truncated and won't actually fill the gap between oldIdx and
   * newIdx over the span of v->chunkSize, so there will still be a slight jump
   * to v->val. This should not be audible. */
  int oldIdx = v->val * (v->curveLen - 1);
  int newIdx = f * (v->curveLen - 1);
  v->idx     = oldIdx;
  v->inc     = (newIdx - oldIdx) / v->chunkSize;
  v->val     = v->curve[newIdx];
  v->chunks  = 1; /* Size this based on delta? */
}

void fillVol(Volume *v, float *buf) {
  int i = 0;
  if (v->chunks > 0) {
    for (; i < v->chunkSize ; i++) {
      buf[i]  = v->curve[v->idx];
      v->idx += v->inc;
    }
    v->chunks--;
  } else {
    for (; i < v->chunkSize ; i++) {
      buf[i] = v->val;
    }
  }
}
