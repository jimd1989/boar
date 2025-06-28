#include "mixer.h"

void mixer(Mixer *m, int chan) {
 int i = 0;
 m->chan = chan;
 for (; i < MIXER_SIZE ; i++) { m->volumes[i] = 1.0f; }
}
