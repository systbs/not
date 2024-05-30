#pragma once
#ifndef __ENTRY_H__
#define __ENTRY_H__ 1

typedef struct sy_entry {
    sy_node_t *scope;
    sy_node_t *block;
    sy_node_t *key;
    sy_record_t *value;

    struct sy_entry *next;
    struct sy_entry *previous;
} sy_entry_t;

#endif