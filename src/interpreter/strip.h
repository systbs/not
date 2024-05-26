#pragma once
#ifndef __STRIP_H__
#define __STRIP_H__ 1

typedef struct strip /*Activation Records*/ {
    sy_queue_t *variables;
    sy_queue_t *inputs;

    struct strip *previous;
    sy_mutex_t lock;
} strip_t;

typedef struct strip_entry {
    sy_node_t *scope;
    sy_node_t *block;
    sy_node_t *key;
    sy_record_t *value;
} sy_strip_entry_t;

strip_t *
sy_strip_create(strip_t *previous);

sy_strip_entry_t *
sy_strip_variable_push(strip_t *ar, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value);

sy_strip_entry_t *
sy_strip_variable_find(strip_t *ar, sy_node_t *key);

sy_strip_entry_t *
sy_strip_input_push(strip_t *ar, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value);

sy_strip_entry_t *
sy_strip_input_find(strip_t *ar, sy_node_t *key);

strip_t *
sy_strip_copy(strip_t *ar);

int32_t
sy_strip_destroy(strip_t *strip);

#endif