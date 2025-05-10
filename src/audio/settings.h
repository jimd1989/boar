#pragma once

/* Global audio settings. The program has no reason to change these. */
#define AUDIO_CHANNELS 2
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_SAMPLE_SIZE_BYTES 2
#define AUDIO_CHUNK_SIZE 48
#define AUDIO_CHUNKS 50
#define AUDIO_BUFFER_SIZE (AUDIO_CHUNK_SIZE * AUDIO_CHUNKS)

/* AUDIO_CHUNK_SIZE × AUDIO_CHUNKS = 2,400
 * AUDIO_SAMPLE_RATE ÷ 2,400 = 20
 * AUDIO_BUFFER_SIZE = 1÷20 = 0.05 seconds */
