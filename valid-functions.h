/* Every user-input function is a single ASCII character. This is a master
 * array of which characters represent valid functions in the program, and
 * what type of input those functions expect as their argument. */

#pragma once

typedef enum ArgStatus {

/* Statuses for parsing string->Arg. */

    ARG_INVALID = 0,
    ARG_INT,
    ARG_FLOAT,
    ARG_STRING,
    ARG_NIL
} ArgStatus;

const ArgStatus VALID_FUNCTIONS[256] = {
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_NIL,       /* # */
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,    /* A */
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,    /* D */
    ARG_STRING,   /* E */
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,    /* L */
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,    /* P */
    ARG_INVALID,
    ARG_FLOAT,    /* R */
    ARG_FLOAT,    /* S */
    ARG_INT,      /* T */
    ARG_INVALID,
    ARG_INVALID,
    ARG_INT,      /* W */
    ARG_FLOAT,    /* X */
    ARG_INVALID, 
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,   /* a */
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,   /* d */
    ARG_STRING,  /* e */
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_FLOAT,   /* l */
    ARG_INVALID,
    ARG_INT,     /* n */
    ARG_INT,     /* o */
    ARG_FLOAT,   /* p */
    ARG_NIL,     /* q */
    ARG_FLOAT,   /* r */
    ARG_FLOAT,   /* s */
    ARG_INT,     /* t */
    ARG_INVALID,
    ARG_INT,     /* v */
    ARG_INT,     /* w */
    ARG_FLOAT,   /* x */
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
    ARG_INVALID,
};
