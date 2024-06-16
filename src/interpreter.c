#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "types/types.h"
#include "utils/path.h"
#include "token/position.h"
#include "container/queue.h"
#include "error.h"
#include "mutex.h"
#include "memory.h"
#include "interpreter.h"

not_interpreter_t *
not_interpreter_create()
{
    not_interpreter_t *it = (not_interpreter_t *)not_memory_calloc(1, sizeof(not_interpreter_t));
    if (!it)
    {
        not_error_no_memory();
        return ERROR;
    }

    it->expections = not_queue_create();
    if (it->expections == ERROR)
    {
        return ERROR;
    }

    return it;
}

void not_interpreter_destroy(not_interpreter_t *it)
{
    not_queue_destroy(it->expections);
    not_memory_free(it);
}
