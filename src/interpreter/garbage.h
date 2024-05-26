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

int32_t
sy_garbage_init();

int32_t
sy_garbage_destroy();

sy_garbage_entry_t *
sy_garbage_push(sy_record_t *value);

#endif