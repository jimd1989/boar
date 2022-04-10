# pragma once

#include "defaults.h"
#include "types.h"

/* This file contains enum values for every function defined in boar. */

/* Assigns a function name and type to a character pair. */
#define FUNC_DEF(X, TYPE) (((unsigned int)(X) - DEFAULT_ASCII_A) | (TYPE))

/* (a) sets attack */
#define FUNC_ATTACK FUNC_DEF('a', TYPE_NORMAL)

/* (A) sets modulator attack */
#define FUNC_MOD_ATTACK FUNC_DEF('A', TYPE_NORMAL)

/* (a.) sets attack waveform */
#define FUNC_ATTACK_WAVE FUNC_DEF('a', TYPE_PERIOD)

/* (A.) sets modulator attack waveform */
#define FUNC_MOD_ATTACK_WAVE FUNC_DEF('A', TYPE_PERIOD)

/* (b) sets left/right channel balance*/
#define FUNC_CHAN_BALANCE FUNC_DEF('b', TYPE_NORMAL)

/* (d) sets decay */
#define FUNC_DECAY FUNC_DEF('d', TYPE_NORMAL)

/* (D) sets modulator decay */
#define FUNC_MOD_DECAY FUNC_DEF('D', TYPE_NORMAL)

/* (d.) sets decay waveform */
#define FUNC_DECAY_WAVE FUNC_DEF('d', TYPE_PERIOD)

/* (D.) sets modulator decay waveform */
#define FUNC_MOD_DECAY_WAVE FUNC_DEF('D', TYPE_PERIOD)

/* (d:) sets envelope loop */
#define FUNC_ENV_LOOP FUNC_DEF('d', TYPE_COLON)

/* (D:) sets modulator envelope loop */
#define FUNC_MOD_ENV_LOOP FUNC_DEF('D', TYPE_COLON)

/* (k) sets key follow */
#define FUNC_KEY_FOLLOW FUNC_DEF('k', TYPE_NORMAL)

/* (K) sets modulator key follow */
#define FUNC_MOD_KEY_FOLLOW FUNC_DEF('K', TYPE_NORMAL)

/* (l) sets amplitude */
#define FUNC_AMPLITUDE FUNC_DEF('l', TYPE_NORMAL)

/* (L) sets modulator amplitude */
#define FUNC_MOD_AMPLITUDE FUNC_DEF('L', TYPE_NORMAL)

/* (n) turns a note on */
#define FUNC_NOTE_ON FUNC_DEF('n', TYPE_NORMAL)

/* (o) turns a note off */
#define FUNC_NOTE_OFF FUNC_DEF('o', TYPE_NORMAL)

/* (p) sets pitch ratio */
#define FUNC_PITCH FUNC_DEF('p', TYPE_NORMAL)

/* (P) sets modulator pitch ratio */
#define FUNC_MOD_PITCH FUNC_DEF('P', TYPE_NORMAL)

/* (q) quits all boars in the pipe */
#define FUNC_QUIT FUNC_DEF('q', TYPE_NORMAL)

/* (r) sets release */
#define FUNC_RELEASE FUNC_DEF('r', TYPE_NORMAL)

/* (R) sets modulator release */
#define FUNC_MOD_RELEASE FUNC_DEF('R', TYPE_NORMAL)

/* (r.) sets release waveform */
#define FUNC_RELEASE_WAVE FUNC_DEF('r', TYPE_PERIOD)

/* (R.) sets release decay waveform*/
#define FUNC_MOD_RELEASE_WAVE FUNC_DEF('R', TYPE_PERIOD)

/* (s) sets sustain */
#define FUNC_SUSTAIN FUNC_DEF('s', TYPE_NORMAL)

/* (S) sets modulator sustain */
#define FUNC_MOD_SUSTAIN FUNC_DEF('S', TYPE_NORMAL)

/* (s.) sets carrier envelope depth */
#define FUNC_ENV_DEPTH FUNC_DEF('s', TYPE_PERIOD)

/* (S.) sets modulator envelope depth */
#define FUNC_MOD_ENV_DEPTH FUNC_DEF('S', TYPE_PERIOD)

/* (t) sets touch sensitivity */
#define FUNC_TOUCH FUNC_DEF('t', TYPE_NORMAL)

/* (T) sets modulator touch sensitivity */
#define FUNC_MOD_TOUCH FUNC_DEF('T', TYPE_NORMAL)

/* (u) tunes note */
#define FUNC_TUNE FUNC_DEF('u', TYPE_NORMAL)

/* (U) selects note to tune */
#define FUNC_TUNE_NOTE FUNC_DEF('U', TYPE_NORMAL)

/* (u.) selects detune source */
#define FUNC_TUNE_TARGET FUNC_DEF('u', TYPE_PERIOD)

/* (w) selects wave */
#define FUNC_WAVE FUNC_DEF('w', TYPE_NORMAL)

/* (W) selects modulator wave */
#define FUNC_MOD_WAVE FUNC_DEF('W', TYPE_NORMAL)

/* (x) sets a fixed frequency */
#define FUNC_FIXED FUNC_DEF('x', TYPE_NORMAL)

/* (X) sets modulator to a fixed frequency */
#define FUNC_MOD_FIXED FUNC_DEF('X', TYPE_NORMAL)
