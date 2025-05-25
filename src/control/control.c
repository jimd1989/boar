#include "../curves/curves.h"
#include "control.h"
#include "volume.h"

void control(Control *c) {
  volume(&c->vol, c->curves.cubic, CURVES_LEN, 64); 
  curves(&c->curves);
}
