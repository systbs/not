#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "memory.h"

void *
qalam_calloc(size_t num, size_t size) {
    void *ptr = calloc(num, size);
    if (!ptr) {
        printf("Unable to allocated a block of %zu bytes", size);
        return nullptr;
    }
    return ptr;
}

void *
qalam_realloc(void * ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (!ptr) {
        printf("Unable to realloced a block of %zu bytes", size);
        return nullptr;
    }
    return ptr;
}


void *
qalam_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        printf("Unable to malloced a block of %zu bytes", size);
        return nullptr;
    }
    return ptr;
}

void
qalam_free(void * ptr) {
    if (!ptr) {
        printf("Unable to free a block of %zu bytes", (size_t)ptr);
        return;
    }
	free(ptr);
}
