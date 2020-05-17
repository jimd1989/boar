/* Defines return codes for errors */

#pragma once

typedef enum Error {
	/* Nothing wrong */
	ERROR_OK = 0,
	/* Error parsing commands line arguments */
	ERROR_ARG,
        /* Error allocating memory */
        ERROR_ALLOC,
        /* Error interfacting with sndio */
        ERROR_SIO,
        /* Error parsing user input */
        ERROR_INPUT,
        /* User input function is invalid */
        ERROR_FUNCTION,
        /* No action taken */
        ERROR_NOTHING,
        /* Wrong argument type passed to function */
        ERROR_TYPE,
        /* Signal to close the program */
        ERROR_EXIT
} Error;
