#pragma once
#ifndef __MEMORY_H__
#define __MEMORY_H__

void *
not_memory_calloc(size_t num, size_t size);

void *
not_memory_malloc(size_t size);

void *
not_memory_realloc(void *ptr, size_t size);

void not_memory_free(void *ptr);

#endif