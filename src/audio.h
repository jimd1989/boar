#pragma once

#include <stdbool.h>
#include <sndio.h>
#include <stdint.h>

/* define-external */
void stdin_eval(char *);
void mio_0_callback(int);
void mio_1_callback(int);
void mio_2_callback(int);
void mio_3_callback(int);
void sio_0_callback(int);
void sio_1_callback(int);
void sio_2_callback(int);
void sio_3_callback(int);

/* foreign-lambda */
void stdin_init(void);
void audio_write(int, uint8_t *, int);
void poll_io(void);
struct mio_hdl *midi_init(int, uint8_t *, char *, bool, bool);
int midi_write(int, uint8_t *, int);
struct sio_hdl *audio_init(int, char *, int, int, int, int, bool);
void audio_close(struct sio_hdl *);
