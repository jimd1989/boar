/* Defines constant maximum values of various variables, used to bound
 * certain operations */

#pragma once

/* The minimum bit depth allowed */
#define MIN_BITS 8

/* The maximum bit depth allowed */
#define MAX_BITS 32

/* The maximum bufsize allowed */
#define MAX_BUFSIZE 100000

/* The maximum number of channels allowed */
#define MAX_CHAN 128

/* The maximum sample rate allowed */
#define MAX_RATE 384000

/* The maximum polyphony allowed */
#define MAX_POLYPHONY 128

/* The maximum command line flag length (currently "polyphony") */
#define MAX_FLAG_LEN 10

/* The maximum amount of time, in seconds, an envelope stage runs for */
#define MAX_ENV_TIME 10.0f

/* The final index of a wavetable. */
#define MAX_WAVE_INDEX 2047

/* The smallest float value. Used to avoid zeroes. */
#define MIN_FLOAT 0.000001f

/* The maximum MIDI parameter value */
#define MAX_MIDI_VALUE 127
