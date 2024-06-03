#pragma once
#ifndef __STRIP_H__
#define __STRIP_H__ 1

typedef struct sy_entry sy_entry_t;

typedef struct sy_strip /*Activation Records*/ {
    sy_entry_t *variables;
    sy_entry_t *inputs;

    struct sy_strip *previous;
    sy_mutex_t lock;
} sy_strip_t;

sy_strip_t *
sy_strip_create(sy_strip_t *previous);

sy_entry_t *
sy_strip_variable_push(sy_strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value);

sy_entry_t *
sy_strip_variable_find(sy_strip_t *strip, sy_node_t *key);

sy_entry_t *
sy_strip_variable_find_by_scope(sy_strip_t *strip, sy_node_t *scope, sy_node_t *key);

sy_entry_t *
sy_strip_input_push(sy_strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value);

sy_entry_t *
sy_strip_input_find(sy_strip_t *strip, sy_node_t *key);

sy_strip_t *
sy_strip_copy(sy_strip_t *ar);

int32_t
sy_strip_destroy_all(sy_strip_t *strip);

int32_t
sy_strip_destroy(sy_strip_t *strip);

#endif