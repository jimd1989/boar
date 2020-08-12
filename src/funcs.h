# pragma once

#include "defaults.h"
#include "types.h"

/* This file contains enum values for every function defined in boar. */

/* (a) sets attack */
#define FUNC_ATTACK ((unsigned int)'a' - DEFAULT_ASCII_A)

/* (A) sets modulator attack */
#define FUNC_MOD_ATTACK ((unsigned int)'A' - DEFAULT_ASCII_A)

/* (a.) sets attack waveform */
#define FUNC_ATTACK_WAVE (((unsigned int)'a' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (A.) sets modulator attack waveform*/
#define FUNC_MOD_ATTACK_WAVE (((unsigned int)'A' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (d) sets decay */
#define FUNC_DECAY ((unsigned int)'d' - DEFAULT_ASCII_A)

/* (D) sets modulator decay */
#define FUNC_MOD_DECAY ((unsigned int)'D' - DEFAULT_ASCII_A)

/* (d.) sets decay waveform */
#define FUNC_DECAY_WAVE (((unsigned int)'d' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (D.) sets modulator decay waveform*/
#define FUNC_MOD_DECAY_WAVE (((unsigned int)'D' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (d:) sets envelope loop */
#define FUNC_ENV_LOOP (((unsigned int)'d' - DEFAULT_ASCII_A) | TYPE_COLON)

/* (D:) sets modulator envelope loop */
#define FUNC_MOD_ENV_LOOP (((unsigned int)'D' - DEFAULT_ASCII_A) | TYPE_COLON)

/* (e) echoes input */
#define FUNC_ECHO ((unsigned int)'e' - DEFAULT_ASCII_A)

/* (k) sets key follow */
#define FUNC_KEY_FOLLOW ((unsigned int)'k' - DEFAULT_ASCII_A)

/* (K) sets modulator key follow */
#define FUNC_MOD_KEY_FOLLOW ((unsigned int)'K' - DEFAULT_ASCII_A)

/* (l) sets amplitude */
#define FUNC_AMPLITUDE ((unsigned int)'l' - DEFAULT_ASCII_A)

/* (L) sets modulator amplitude */
#define FUNC_MOD_AMPLITUDE ((unsigned int)'L' - DEFAULT_ASCII_A)

/* (n) turns a note on */
#define FUNC_NOTE_ON ((unsigned int)'n' - DEFAULT_ASCII_A)

/* (o) turns a note off */
#define FUNC_NOTE_OFF ((unsigned int)'o' - DEFAULT_ASCII_A)

/* (p) sets pitch ratio */
#define FUNC_PITCH ((unsigned int)'p' - DEFAULT_ASCII_A)

/* (P) sets modulator pitch ratio */
#define FUNC_MOD_PITCH ((unsigned int)'P' - DEFAULT_ASCII_A)

/* (q) quits all boars in the pipe */
#define FUNC_QUIT ((unsigned int)'q' - DEFAULT_ASCII_A)

/* (r) sets release */
#define FUNC_RELEASE ((unsigned int)'r' - DEFAULT_ASCII_A)

/* (R) sets modulator release */
#define FUNC_MOD_RELEASE ((unsigned int)'R' - DEFAULT_ASCII_A)

/* (r.) sets release waveform */
#define FUNC_RELEASE_WAVE (((unsigned int)'r' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (R.) sets release decay waveform*/
#define FUNC_MOD_RELEASE_WAVE (((unsigned int)'R' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (s) sets sustain */
#define FUNC_SUSTAIN ((unsigned int)'s' - DEFAULT_ASCII_A)

/* (S) sets modulator sustain */
#define FUNC_MOD_SUSTAIN ((unsigned int)'S' - DEFAULT_ASCII_A)

/* (t) sets touch sensitivity */
#define FUNC_TOUCH ((unsigned int)'t' - DEFAULT_ASCII_A)

/* (T) sets modulator touch sensitivity */
#define FUNC_MOD_TOUCH ((unsigned int)'T' - DEFAULT_ASCII_A)

/* (u) tunes note */
#define FUNC_TUNE ((unsigned int)'u' - DEFAULT_ASCII_A)

/* (U) selects note to tune */
#define FUNC_TUNE_NOTE ((unsigned int)'U' - DEFAULT_ASCII_A)

/* (u.) selects detune source */
#define FUNC_TUNE_TARGET (((unsigned int)'u' - DEFAULT_ASCII_A) | TYPE_PERIOD)

/* (w) selects wave */
#define FUNC_WAVE ((unsigned int)'w' - DEFAULT_ASCII_A)

/* (W) selects modulator wave */
#define FUNC_MOD_WAVE ((unsigned int)'W' - DEFAULT_ASCII_A)

/* (x) sets a fixed frequency */
#define FUNC_FIXED ((unsigned int)'x' - DEFAULT_ASCII_A)

/* (X) sets modulator to a fixed frequency */
#define FUNC_MOD_FIXED ((unsigned int)'X' - DEFAULT_ASCII_A)
