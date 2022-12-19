#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "error.h"

error_t *
error_create(position_t position, char *message)
{
    error_t *error;
    error = (error_t *)malloc(sizeof(error_t));
    if(!error)
    {
        fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(error_t));
		return NULL;
    }
    error->message = message;
    error->position = position;
    return error;
}

