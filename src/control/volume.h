#pragma once

typedef struct Volume {
  int     chunks;
  int     idx;
  int     inc;
  float   val;
  float * curve;
} Volume;

void volume(Volume *, float *);
void setVol(Volume *, float);
void fillVolChunk(Volume *, float *);
