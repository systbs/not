#pragma once
#ifndef __ENTRY_H__
#define __ENTRY_H__ 1

typedef struct not_entry
{
    not_node_t *scope;
    not_node_t *block;
    not_node_t *key;
    not_record_t *value;

    struct not_entry *next;
    struct not_entry *previous;
} not_entry_t;

#endif