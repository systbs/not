#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"

void *
sy_memory_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void *
sy_memory_malloc(size_t size)
{
    return malloc(size);
}

void *
sy_memory_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void
sy_memory_free(void *ptr)
{
    free(ptr);
}