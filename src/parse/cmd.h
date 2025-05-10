#pragma once

#include <stdint.h>

#include "cursor.h"

/* All user commands are J-style: one char and (optionally one decorator).
 * Decorators are '.' or ':'. Ex: 'a', 'a.', and 'a:' */

/* A parsed Cmd is a 16 bit int, with bottom 7 bits being the ASCII char, and
 * top bits set depending upon the decorator. */
typedef int16_t Cmd;

#define CMD_PURE(X) ((X) | (1u << 8))
#define CMD_DOT(X) ((X) | (1u << 9))
#define CMD_COLON(X) ((X) | (1u << 10))
#define IS_CMD_PURE(X) (((X) >> 8) & 1)
#define IS_CMD_DOT(X) (((X) >> 9) & 1)
#define IS_CMD_COLON(X) (((X) >> 10) & 1)
#define CMD_CHAR(X) ((X) & 127)

/* Alphabets of valid commands are represented as set bits in 128 bit fields. 
 * These fields must be populated at start up time; don't forget to use
 * `enlistCmd` in the constructors in `cmd.c`. Technically a character can
 * be enlisted directly without these enums, but they can ensure complete
 * pattern matching at eval time. */
typedef enum {
  CMD_COMMENT   = 35,  /* # */
  CMD_ATTACK    = 97,  /* a */
  CMD_NOTE_ON   = 110, /* n */
  CMD_NOTE_OFF  = 111, /* o */
  CMD_WAVE      = 119  /* w */
} CmdPure;

typedef enum {
  CMD_DOT_UNKNOWN = 0
} CmdDot;

typedef enum {
  CMD_COLON_UNKNOWN = 0
} CmdColon;

typedef struct CmdAlphabet {
  uint32_t  pure[4];
  uint32_t  dot[4];
  uint32_t  colon[4];
} CmdAlphabet;

void cmdAlphabet(CmdAlphabet *);
void parseCmd(Cursor *, CmdAlphabet);
