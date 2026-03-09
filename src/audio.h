#pragma once

#include <stdbool.h>
#include <sndio.h>
#include <stdint.h>

/* define-external */
void stdin_eval(char *);
void sio_0(int);
void sio_1(int);
void sio_2(int);
void sio_3(int);

/* foreign-lambda */
void stdin_init(void);
void fill_dsp(int, uint8_t *, int);
void poll_io(void (*f)(char *));
struct mio_hdl *midi_init(int, uint8_t *, void(*f)(int), char *, bool, bool);
int midi_write(int, uint8_t *, int);
struct sio_hdl *audio_init(int, char *, int, int, int, int, bool);
void audio_close(struct sio_hdl *);
