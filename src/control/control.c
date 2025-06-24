#include "../keyboard/keyboard.h" 
#include "control.h"
#include "volume.h"

void control(Control *c) {
  volume(&c->vol, c->curves.cubic); 
  curves(&c->curves);
  keyboard(&c->keyboard);
}
