#pragma once
#ifndef __GARBAGE_H__
#define __GARBAGE_H__ 1

typedef struct sy_garbage_entry {
    sy_record_t *value;

    struct sy_garbage_entry *previous;
    struct sy_garbage_entry *next;
} sy_garbage_entry_t;

typedef struct sy_garbage {
    sy_garbage_entry_t *begin;
    sy_mutex_t lock;
} sy_garbage_t;

typedef struct {
    /* inputs */

    /* outputs */
    int32_t ret;
} sy_grabage_thread_data_t;

int32_t
sy_garbage_init();

int32_t
sy_garbage_destroy();

int32_t
sy_garbage_push(sy_record_t *value);

#if _WIN32
DWORD WINAPI 
sy_garbage_clean_by_thread(LPVOID arg);
#else
void * 
sy_garbage_clean_by_thread(void *arg);
#endif

#endif