#pragma once

#include <stdint.h>

#include "cursor.h"

/* All user commands are J-style: one char and (optionally one decorator).
 * Decorators are '.', ':', or '+'. Ex: 'a', 'a.', 'a:', and 'a+'.
 * No hard rules; must allow space for all commands, but some guidelines:
 * - Undecorated commands favored for "real time" events like notes, bend, etc.
 * - '+' decorator intended for settings tweaks. 
 * - Capital letters considered reserved for modulation targets. Wary of mod
 *   matrix becoming a DSL unto itself, and alphabet clashes would make this
 *   worse. */

/* A parsed Cmd is a 16 bit int, with bottom 7 bits being the ASCII char, and
 * top bits set depending upon the decorator. */
typedef int16_t Cmd;

#define CMD_PURE(X) ((X) | (1u << 8))
#define CMD_DOT(X) ((X) | (1u << 9))
#define CMD_COLON(X) ((X) | (1u << 10))
#define CMD_PLUS(X) ((X) | (1u << 11))
#define IS_CMD_PURE(X) (((X) >> 8) & 1)
#define IS_CMD_DOT(X) (((X) >> 9) & 1)
#define IS_CMD_COLON(X) (((X) >> 10) & 1)
#define IS_CMD_PLUS(X) (((X) >> 11) & 1)
#define CMD_CHAR(X) ((X) & 127)

/* Alphabets of valid commands are represented as set bits in 128 bit fields. 
 * These fields must be populated at start up time; don't forget to use
 * `enlistCmd` in the constructors in `cmd.c`. Technically a character can
 * be enlisted directly without these enums, but they can ensure complete
 * pattern matching at eval time. */
typedef enum {
  CMD_COMMENT    = 35,  /* # */
  CMD_ATTACK     = 97,  /* a */
  CMD_BEND       = 98,  /* b */
  CMD_NOTE_ON    = 110, /* n */
  CMD_NOTE_OFF   = 111, /* o */
  CMD_AFTERTOUCH = 116, /* t */
  CMD_VOL        = 118, /* v */
  CMD_WAVE       = 119, /* w */
  CMD_ZONE       = 122  /* z */
} CmdPure;

typedef enum {
  CMD_DOT_VOL = 118, /* v. */
} CmdDot;

typedef enum {
  CMD_COLON_UNKNOWN = 0
} CmdColon;

typedef enum {
  CMD_PLUS_ZONE = 122 /* z+ */
} CmdPlus;

typedef struct CmdAlphabet {
  uint32_t  pure[4];
  uint32_t  dot[4];
  uint32_t  colon[4];
  uint32_t  plus[4];
} CmdAlphabet;

void cmdAlphabet(CmdAlphabet *);
void parseCmd(Cursor *, CmdAlphabet);
