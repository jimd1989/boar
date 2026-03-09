#pragma once

#include <stdbool.h>
#include <sndio.h>
#include <stdint.h>

void stdin_init(void);
void fill_dsp(int, uint8_t *, int);
void poll_io(void (*f)(char *));
struct mio_hdl *midi_init(int, uint8_t *, void(*f)(int), char *, bool, bool);
int midi_write(int, uint8_t *, int);
struct sio_hdl *audio_init(int, void(*f)(int), char *, int, int, int, int, bool);
void audio_close(struct sio_hdl *);
