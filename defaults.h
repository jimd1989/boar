/* Defines a set of default numerical constants, which are considered sane
 * audio settings. Some of these are constant throughout the lifespan of the
 * program. Others are overwritten by user-specified flags. */

#pragma once

#include <math.h>
#include <stdbool.h>

/* Bit depth of an individual sample */
#define DEFAULT_BITS 16

/* Audio output buffer size (in frames) */
#define DEFAULT_BUFSIZE 960

/* Number of channels */
#define DEFAULT_CHAN 1

/* Sample rate */
#define DEFAULT_RATE 48000

/* Number of simultaneous voices */
#define DEFAULT_POLYPHONY 8

/* Length of wavetable (should be a multiple of UINT_MAX) */
#define DEFAULT_WAVELEN 4096

/* Number of possible MIDI notes */
#define DEFAULT_KEYS_NUM 128

/* A number greater than the number of possible MIDI notes, for signaling
 * that no note is being played */
#define DEFAULT_NO_KEY (DEFAULT_KEYS_NUM + 1)

/* Lowest frequency of all 128 notes (C1) */
#define DEFAULT_LOWEST_FREQUENCY 8.1757989156f

/* ASCII number for "0" */
#define DEFAULT_ASCII_ZERO 48

/* ASCII number for "9" */
#define DEFAULT_ASCII_NINE 57

/* ASCII number for "A" */
#define DEFAULT_ASCII_A 65

/* ASCII number for "Z" */
#define DEFAULT_ASCII_Z 90

/* ASCII number for "a" */
#define DEFAULT_ASCII_A_LOWER 97

/* ASCII number for "z" */
#define DEFAULT_ASCII_Z_LOWER 122

/* Maximum user line input */
#define DEFAULT_LINESIZE 4096

/* Where echo (e) commands are written to */
#define DEFAULT_ECHO_FILE stdout
