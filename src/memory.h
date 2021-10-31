#pragma once

void *
qalam_calloc(size_t num, size_t size);

void *
qalam_realloc(void * ptr, size_t size);

void *
qalam_malloc(size_t size);

void
qalam_free(void * ptr);

#define qalam_sizeof(ptr) ((size_t) (malloc_usable_size(ptr)))

