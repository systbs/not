#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "types/types.h"
#include "utils/path.h"
#include "token/position.h"
#include "error.h"
#include "mutex.h"
#include "memory.h"
#include "interpreter.h"

sy_interpreter_t *
sy_interpreter_create()
{
    sy_interpreter_t *it = (sy_interpreter_t *)sy_memory_calloc(1, sizeof(sy_interpreter_t));
    if (!it)
    {
        sy_error_no_memory();
        return ERROR;
    }

    return it;
}

int32_t
sy_interpreter_destroy(sy_interpreter_t *it)
{
    sy_memory_free(it);
    return 0;
}

