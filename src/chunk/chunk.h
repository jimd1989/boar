#pragma once

/* Provides compile-time definition of DSP chunk sizes. Regardless of soundcard
 * buffer size, audio information will be generated according to these numbers.
 * Reducing frame-by-frame operations to chunk estimates offers performance
 * benefits. */

#define AUDIO_CHUNK_SIZE 64 /* Should be always be a power of two */
#define AUDIO_CHUNK_BITS 6  /* Used for bitwise division */
#define AUDIO_CHUNK_DIV(X) ((X) >> AUDIO_CHUNK_BITS)
#define AUDIO_CHUNK_MULT(X) ((X) << AUDIO_CHUNK_BITS)
#define AUDIO_CHUNK_MOD(X) ((X) & ((unsigned int)(AUDIO_CHUNK_SIZE - 1)))
