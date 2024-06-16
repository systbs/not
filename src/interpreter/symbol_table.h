#pragma once
#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__ 1

typedef struct not_entry not_entry_t;

typedef struct not_symbol_table
{
    not_entry_t *begin;
    not_mutex_t lock;
} not_symbol_table_t;

int32_t
not_symbol_table_init();

int32_t
not_symbol_table_destroy();

not_entry_t *
not_symbol_table_push(not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value);

not_entry_t *
not_symbol_table_find(not_node_t *scope, not_node_t *key);

#endif