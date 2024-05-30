#pragma once
#ifndef __MEMORY_H__
#define __MEMORY_H__

void *
sy_memory_calloc(size_t num, size_t size);

void *
sy_memory_malloc(size_t size);

void *
sy_memory_realloc(void *ptr, size_t size);

void
sy_memory_free(void *ptr);

#endif