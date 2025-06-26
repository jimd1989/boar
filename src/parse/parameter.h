#pragma once

#include "cursor.h"

/* Parameters are space-delimited arguments to a command. They are lazily parsed
 * and evaluated, meaning a command can be overloaded to contain different
 * parameter types and counts. The types are:
 * int            → A number without a decimal.
 * float          → A number with a decimal.
 * nullable float → A number with a decimal or blank "_", represented 
 *                  internally by NaN. Used to deliberately omit certain values. 
 *                  Due to NaN, it must be a float, but can always be cast back 
 *                  to int once the null case is handled.
 * string         → A piece of text. Null terminator "\0" is inserted directly
 *                  into parse buffer where param delimiter is. This should be
 *                  enough for most idempotent commands, but copy if long-lived
 *                  data is needed. */
void parseInt(Cursor *);
void parseBoundInt(Cursor *, int, int);
void parseFloat(Cursor *);
void parseBoundFloat(Cursor *, float, float);
void parseNullableBoundFloat(Cursor *, float, float);
void parseString(Cursor *);
