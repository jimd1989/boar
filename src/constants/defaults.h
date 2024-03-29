/* Defines a set of default numerical constants, which are considered sane
 * audio settings. Some of these are constant throughout the lifespan of the
 * program. Others are overwritten by user-specified flags. */

#pragma once

#include <math.h>
#include <stdbool.h>

/* Bit depth of an individual sample */
#define DEFAULT_BITS 16

/* Byte depth of an individual sample */
#define DEFAULT_BYTES (DEFAULT_BITS / 8)

/* Sample rate */
#define DEFAULT_RATE 48000

/* Number of times to poll for user input per second */
#define DEFAULT_RESOLUTION 375

/* Ideal audio output buffer size (in frames, not bytes). Even if audio output
 * does not respect this size, it will be used for the timing of user input,
 * ensuring a resolution of DEFAULT_RESOLUTION  */
#define DEFAULT_BUFSIZE (DEFAULT_RATE / DEFAULT_RESOLUTION)

/* An ideally-sized audio buffer would be responsive but performance intensive.
 * The sndio `appbufsz` parameter will be the minimal audio buffer size
 * multiplied by this constant (or overridden by command line flag). */
#define DEFAULT_BUF_BLOCKS 1

/* Number of channels. Hardcoded to stereo for now. */
#define DEFAULT_CHAN 2

/* Number of simultaneous voices */
#define DEFAULT_POLYPHONY 8

/* Length of wavetable (should be a multiple of UINT_MAX) */
#define DEFAULT_WAVELEN 2048

/* Number of possible MIDI notes */
#define DEFAULT_KEYS_NUM 128

/* Number of octaves, used to segregate band-limited wavetables */
#define DEFAULT_OCTAVES 12

/* Scaling factor to create a unique mapping of octave pitch to wavetable
 * index */
#define DEFAULT_OCTAVE_SCALING 4.5f

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
