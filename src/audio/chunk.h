#pragma once

#include "settings.h"

#define AUDIO_CHUNK_SIZE 64 /* Should be power of two */
#define AUDIO_CHUNK_BITS 6  /* Used for bitwise division */
#define AUDIO_CHUNK_DIV(X) ((X) >> AUDIO_CHUNK_BITS)
#define AUDIO_CHUNK_MULT(X) ((X) << AUDIO_CHUNK_BITS)
#define AUDIO_CHUNK_MOD(X) ((X) & (AUDIO_CHUNK_SIZE - 1))
