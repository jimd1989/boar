#include <stdint.h>
#include <string.h>

#include "slice.h"

void f32slice_copy_to_f32vector(float *slice, int written, 
                                int toWrite, float *dest) {
  float *src = &slice[written];
  int len    = toWrite * 4;
  memcpy((void *)dest, (void *)src, len);
}

void f32slice_copy_from_f32vector(float *slice, int written, 
                                int toWrite, float *src) {
  float *dest = &slice[written];
  int len     = toWrite * 4;
  memcpy((void *)dest, (void *)src, len);
}
