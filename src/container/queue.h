#pragma once
#ifndef __QUEUE_H__
#define __QUEUE_H__ 1

typedef struct sy_queue_entry {
    struct sy_queue_entry *previous;
    struct sy_queue_entry *next;
    void *value;
} sy_queue_entry_t;

typedef struct sy_queue {
    sy_queue_entry_t *begin;
    sy_queue_entry_t *end;
} sy_queue_t;

sy_queue_t *
sy_queue_apply(sy_queue_t *queue);

sy_queue_t *
sy_queue_create();

int
sy_queue_is_empty(sy_queue_t *queue);

uint64_t
sy_queue_count(sy_queue_t *queue);

int32_t
sy_queue_query(sy_queue_t *queue, int (*f)(sy_queue_entry_t*));

void
sy_queue_destroy(sy_queue_t *queue);

void
sy_queue_link(sy_queue_t *queue, sy_queue_entry_t *current, sy_queue_entry_t *entry);

void
sy_queue_unlink(sy_queue_t *queue, sy_queue_entry_t *entry);

void
sy_queue_sort(sy_queue_t *queue, int (*f)(sy_queue_entry_t *, sy_queue_entry_t *));

void
sy_queue_clear(sy_queue_t *queue);

sy_queue_entry_t*
sy_queue_right_pop(sy_queue_t *queue);

sy_queue_entry_t *
sy_queue_right_push(sy_queue_t *queue, void *value);

sy_queue_entry_t*
sy_queue_left_pop(sy_queue_t *queue);

sy_queue_entry_t *
sy_queue_left_push(sy_queue_t *queue, void *value);

sy_queue_entry_t *
sy_queue_insert(sy_queue_t *queue, sy_queue_entry_t *index, void *value);

#endif