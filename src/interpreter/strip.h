#pragma once
#ifndef __STRIP_H__
#define __STRIP_H__ 1

typedef struct not_entry not_entry_t;

typedef struct not_strip /*Activation Records*/
{
    not_queue_t *variables;
    not_queue_t *inputs;

    struct not_strip *previous;
    not_mutex_t lock;
} not_strip_t;

not_strip_t *
not_strip_create(not_strip_t *previous);

not_entry_t *
not_strip_variable_push(not_strip_t *strip, not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value);

not_entry_t *
not_strip_variable_find(not_strip_t *strip, not_node_t *scope, not_node_t *key);

not_entry_t *
not_strip_input_push(not_strip_t *strip, not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value);

not_entry_t *
not_strip_input_find(not_strip_t *strip, not_node_t *scope, not_node_t *key);

int32_t
not_strip_variable_remove_by_scope(not_strip_t *strip, not_node_t *scope);

void not_strip_attach(not_strip_t *strip, not_strip_t *previous);

not_strip_t *not_strip_copy(not_strip_t *ar);

int32_t
not_strip_destroy(not_strip_t *strip);

int32_t
not_strip_destroy(not_strip_t *strip);

#endif