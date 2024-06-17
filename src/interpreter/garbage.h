#pragma once
#ifndef __GARBAGE_H__
#define __GARBAGE_H__ 1

typedef struct not_garbage_entry
{
    not_record_t *value;

    struct not_garbage_entry *previous;
    struct not_garbage_entry *next;
} not_garbage_entry_t;

typedef struct not_garbage
{
    not_garbage_entry_t *begin;
    not_mutex_t lock;
} not_garbage_t;

typedef struct
{
    /* inputs */

    /* outputs */
    int32_t ret;
} not_grabage_thread_data_t;

int32_t
not_garbage_init();

int32_t
not_garbage_destroy();

int32_t
not_garbage_push(not_record_t *value);

#if defined(_WIN32) || defined(_WIN64)
DWORD WINAPI
not_garbage_clean_by_thread(LPVOID arg);
#else
void *
not_garbage_clean_by_thread(void *arg);
#endif

#endif