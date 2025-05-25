#pragma once

typedef struct Volume {
  int     chunks;
  int     chunkSize;
  int     idx;
  int     inc;
  int     curveLen;
  float   val;
  float * curve;
} Volume;

void volume(Volume *, float *, int, int);
void setVol(Volume *, float);
void fillVol(Volume *, float *);
