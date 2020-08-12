# pragma once

/* TYPE_FLAG_* is an individual bit that is set when parsing a string of user
 * input. This input can have multiple attributes, such as being negative and
 * a floating point number, so different combinations of engaged bits will
 * describe the full nature of the input's type. */

/* Engaged when a numerical value (0 ... 9) is encountered. */
#define TYPE_FLAG_NUMBER (unsigned int)(1 << 0)

/* Engaged when a decimal (.) is encountered. */
#define TYPE_FLAG_FLOATING (unsigned int)(1 << 1)

/* Engaged when a negative sign (-) is encountered. */
#define TYPE_FLAG_SIGNED (unsigned int)(1 << 2)

/* Engaged when an alphabetic character (A ... z) is encountered. */
#define TYPE_FLAG_TEXT (unsigned int)(1 << 3)

/* Engaged when any other character is encountered. */
#define TYPE_FLAG_MISC (unsigned int)(1 << 4)

/* TYPE_* is a composite value of TYPE_FLAG_* values. These are the values that
 * define the type signatures of boar's functions. */

/* No argument whatsoever. */
#define TYPE_NIL (unsigned int)0

/* An integer n, where n >= 0. */
#define TYPE_UINT TYPE_FLAG_NUMBER

/* A positive or negative integer. */
#define TYPE_INT (TYPE_FLAG_NUMBER | TYPE_FLAG_SIGNED)

/* A float n, where n >= 0.0. */
#define TYPE_UFLOAT (TYPE_FLAG_NUMBER | TYPE_FLAG_FLOATING)

/* A positive or negative float. */
#define TYPE_FLOAT (TYPE_FLAG_NUMBER | TYPE_FLAG_SIGNED | TYPE_FLAG_FLOATING)

/* A line of text made up exclusively of alphabetic characters and whitespace.
 * No digits or symbols are permitted. In almost all cases, only the first
 * character is relevant, making this more of a char type. */
#define TYPE_TEXT TYPE_FLAG_TEXT

/* A placeholder used to signify that a function and argument have yet to be
 * defined for a command character. This is given a high bit value to
 * segregate it from the bits used in parsing. */
#define TYPE_UNDEFINED (unsigned int)(1 << 31)

/* Signifies that any and all input is valid for a function. */
#define TYPE_ANY (unsigned int)(1 << 30)

/* A command with a "." adornment */
#define TYPE_PERIOD (unsigned int)(1 << 7)

/* A command with a ":" adornment */
#define TYPE_COLON (unsigned int)(1 << 8)

/* An array of expected argument types for commands with an unadorned
 * character such as (a), (d), etc */
static const unsigned int TYPE_SIGNATURES_PURE[58] = {
  TYPE_UFLOAT,    /* A */
  TYPE_UNDEFINED, /* B */
  TYPE_UNDEFINED, /* C */
  TYPE_UFLOAT,    /* D */
  TYPE_UNDEFINED, /* E */
  TYPE_UNDEFINED, /* F */
  TYPE_UNDEFINED, /* G */
  TYPE_UNDEFINED, /* H */
  TYPE_UNDEFINED, /* I */
  TYPE_UNDEFINED, /* J */
  TYPE_INT,       /* K */
  TYPE_UFLOAT,    /* L */
  TYPE_UNDEFINED, /* M */
  TYPE_UNDEFINED, /* N */
  TYPE_UNDEFINED, /* O */
  TYPE_UFLOAT,    /* P */
  TYPE_UNDEFINED, /* Q */
  TYPE_UFLOAT,    /* R */
  TYPE_UFLOAT,    /* S */
  TYPE_INT,       /* T */
  TYPE_UINT,      /* U */
  TYPE_UNDEFINED, /* V */
  TYPE_INT,       /* W */
  TYPE_UFLOAT,    /* X */
  TYPE_UNDEFINED, /* Y */
  TYPE_UNDEFINED, /* Z */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UFLOAT,    /* a */
  TYPE_UNDEFINED, /* b */
  TYPE_UNDEFINED, /* c */
  TYPE_UFLOAT,    /* d */
  TYPE_ANY,       /* e */
  TYPE_UNDEFINED, /* f */
  TYPE_UNDEFINED, /* g */
  TYPE_UNDEFINED, /* h */
  TYPE_UNDEFINED, /* i */
  TYPE_UNDEFINED, /* j */
  TYPE_INT,       /* k */
  TYPE_UFLOAT,    /* l */
  TYPE_UNDEFINED, /* m */
  TYPE_UINT,      /* n */
  TYPE_UINT,      /* o */
  TYPE_UFLOAT,    /* p */
  TYPE_NIL,       /* q */
  TYPE_UFLOAT,    /* r */
  TYPE_UFLOAT,    /* s */
  TYPE_INT,       /* t */
  TYPE_UFLOAT,    /* u */
  TYPE_UNDEFINED, /* v */
  TYPE_INT,       /* w */
  TYPE_UFLOAT,    /* x */
  TYPE_UNDEFINED, /* y */
  TYPE_UNDEFINED, /* z */
};

/* An array of expected argument types for commands with period-adorned
 * characters such as (a.), (d.), etc */
static const unsigned int TYPE_SIGNATURES_PERIOD[58] = {
  TYPE_INT,       /* A. */
  TYPE_UNDEFINED, /* B. */
  TYPE_UNDEFINED, /* C. */
  TYPE_INT,       /* D. */
  TYPE_UNDEFINED, /* E. */
  TYPE_UNDEFINED, /* F. */
  TYPE_UNDEFINED, /* G. */
  TYPE_UNDEFINED, /* H. */
  TYPE_UNDEFINED, /* I. */
  TYPE_UNDEFINED, /* J. */
  TYPE_UNDEFINED, /* K. */
  TYPE_UNDEFINED, /* L. */
  TYPE_UNDEFINED, /* M. */
  TYPE_UNDEFINED, /* N. */
  TYPE_UNDEFINED, /* O. */
  TYPE_UNDEFINED, /* P. */
  TYPE_UNDEFINED, /* Q. */
  TYPE_INT,       /* R. */
  TYPE_UNDEFINED, /* S. */
  TYPE_UNDEFINED, /* T. */
  TYPE_UNDEFINED, /* U. */
  TYPE_UNDEFINED, /* V. */
  TYPE_UNDEFINED, /* W. */
  TYPE_UNDEFINED, /* X. */
  TYPE_UNDEFINED, /* Y. */
  TYPE_UNDEFINED, /* Z. */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_INT,       /* a. */
  TYPE_UNDEFINED, /* b. */
  TYPE_UNDEFINED, /* c. */
  TYPE_INT,       /* d. */
  TYPE_UNDEFINED, /* e. */
  TYPE_UNDEFINED, /* f. */
  TYPE_UNDEFINED, /* g. */
  TYPE_UNDEFINED, /* h. */
  TYPE_UNDEFINED, /* i. */
  TYPE_UNDEFINED, /* j. */
  TYPE_UNDEFINED, /* k. */
  TYPE_UNDEFINED, /* l. */
  TYPE_UNDEFINED, /* m. */
  TYPE_UNDEFINED, /* n. */
  TYPE_UNDEFINED, /* o. */
  TYPE_UNDEFINED, /* p. */
  TYPE_UNDEFINED, /* q. */
  TYPE_INT,       /* r. */
  TYPE_UNDEFINED, /* s. */
  TYPE_UNDEFINED, /* t. */
  TYPE_INT,       /* u. */
  TYPE_UNDEFINED, /* v. */
  TYPE_UNDEFINED, /* w. */
  TYPE_UNDEFINED, /* x. */
  TYPE_UNDEFINED, /* y. */
  TYPE_UNDEFINED, /* z. */
};

/* An array of expected argument types for commands with colon-adorned
 * characters such as (a:), (d:), etc */
static const unsigned int TYPE_SIGNATURES_COLON[58] = {
  TYPE_UNDEFINED, /* A: */
  TYPE_UNDEFINED, /* B: */
  TYPE_UNDEFINED, /* C: */
  TYPE_UINT,      /* D: */
  TYPE_UNDEFINED, /* E: */
  TYPE_UNDEFINED, /* F: */
  TYPE_UNDEFINED, /* G: */
  TYPE_UNDEFINED, /* H: */
  TYPE_UNDEFINED, /* I: */
  TYPE_UNDEFINED, /* J: */
  TYPE_UNDEFINED, /* K: */
  TYPE_UNDEFINED, /* L: */
  TYPE_UNDEFINED, /* M: */
  TYPE_UNDEFINED, /* N: */
  TYPE_UNDEFINED, /* O: */
  TYPE_UNDEFINED, /* P: */
  TYPE_UNDEFINED, /* Q: */
  TYPE_UNDEFINED, /* R: */
  TYPE_UNDEFINED, /* S: */
  TYPE_UNDEFINED, /* T: */
  TYPE_UNDEFINED, /* U: */
  TYPE_UNDEFINED, /* V: */
  TYPE_UNDEFINED, /* W: */
  TYPE_UNDEFINED, /* X: */
  TYPE_UNDEFINED, /* Y: */
  TYPE_UNDEFINED, /* Z: */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* ignored */
  TYPE_UNDEFINED, /* a: */
  TYPE_UNDEFINED, /* b: */
  TYPE_UNDEFINED, /* c: */
  TYPE_UINT,      /* d: */
  TYPE_UNDEFINED, /* e: */
  TYPE_UNDEFINED, /* f: */
  TYPE_UNDEFINED, /* g: */
  TYPE_UNDEFINED, /* h: */
  TYPE_UNDEFINED, /* i: */
  TYPE_UNDEFINED, /* j: */
  TYPE_UNDEFINED, /* k: */
  TYPE_UNDEFINED, /* l: */
  TYPE_UNDEFINED, /* m: */
  TYPE_UNDEFINED, /* n: */
  TYPE_UNDEFINED, /* o: */
  TYPE_UNDEFINED, /* p: */
  TYPE_UNDEFINED, /* q: */
  TYPE_UNDEFINED, /* r: */
  TYPE_UNDEFINED, /* s: */
  TYPE_UNDEFINED, /* t: */
  TYPE_UNDEFINED, /* u: */
  TYPE_UNDEFINED, /* v: */
  TYPE_UNDEFINED, /* w: */
  TYPE_UNDEFINED, /* x: */
  TYPE_UNDEFINED, /* y: */
  TYPE_UNDEFINED, /* z: */
};
