#pragma once
#ifndef __QUEUE_H__
#define __QUEUE_H__ 1

typedef struct not_queue_entry
{
    struct not_queue_entry *previous;
    struct not_queue_entry *next;
    void *value;
} not_queue_entry_t;

typedef struct not_queue
{
    not_queue_entry_t *begin;
    not_queue_entry_t *end;
} not_queue_t;

not_queue_t *
not_queue_apply(not_queue_t *queue);

not_queue_t *
not_queue_create();

int not_queue_is_empty(not_queue_t *queue);

uint64_t
not_queue_count(not_queue_t *queue);

int32_t
not_queue_query(not_queue_t *queue, int (*f)(not_queue_entry_t *));

void not_queue_destroy(not_queue_t *queue);

void not_queue_link(not_queue_t *queue, not_queue_entry_t *current, not_queue_entry_t *entry);

void not_queue_unlink(not_queue_t *queue, not_queue_entry_t *entry);

void not_queue_sort(not_queue_t *queue, int (*f)(not_queue_entry_t *, not_queue_entry_t *));

void not_queue_clear(not_queue_t *queue);

not_queue_entry_t *
not_queue_right_pop(not_queue_t *queue);

not_queue_entry_t *
not_queue_right_push(not_queue_t *queue, void *value);

not_queue_entry_t *
not_queue_left_pop(not_queue_t *queue);

not_queue_entry_t *
not_queue_left_push(not_queue_t *queue, void *value);

not_queue_entry_t *
not_queue_insert(not_queue_t *queue, not_queue_entry_t *index, void *value);

#endif