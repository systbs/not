#pragma once
#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__ 1

typedef struct sy_entry sy_entry_t;

typedef struct sy_symbol_table {
    sy_entry_t *begin;
    sy_mutex_t lock;
} sy_symbol_table_t;

int32_t
sy_symbol_table_init();

int32_t
sy_symbol_table_destroy();

sy_entry_t *
sy_symbol_table_push(sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value);

sy_entry_t *
sy_symbol_table_find(sy_node_t *scope, sy_node_t *key);

#endif