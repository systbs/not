#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>
#include <jansson.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../parser/syntax/syntax.h"
#include "../error.h"
#include "../memory.h"
#include "../mutex.h"
#include "record.h"
#include "entry.h"
#include "symbol_table.h"

not_symbol_table_t base_symbol_table;

static not_symbol_table_t *
not_symbol_table_get()
{
    return &base_symbol_table;
}

int32_t
not_symbol_table_init()
{
    not_symbol_table_t *st = not_symbol_table_get();
    if (not_mutex_init(&st->lock) < 0)
    {
        not_error_system("'%s' could not initialize the lock", "not_symbol_table.lock");
        return -1;
    }

    st->begin = NOT_PTR_NULL;

    return 0;
}

int32_t
not_symbol_table_destroy()
{
    not_symbol_table_t *st = not_symbol_table_get();
    if (not_mutex_destroy(&st->lock) < 0)
    {
        not_error_system("'%s' could not destroy the lock", "not_symbol_table.lock");
        return -1;
    }
    return 0;
}

static int32_t
not_symbol_table_id_cmp(not_node_t *n1, not_node_t *n2)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
    not_node_basic_t *nb2 = (not_node_basic_t *)n2->value;

    return (strcmp(nb1->value, nb2->value) == 0);
}

static void
not_symbol_table_link(not_entry_t *entry)
{
    not_symbol_table_t *st = not_symbol_table_get();

    entry->next = st->begin;
    if (st->begin)
    {
        st->begin->previous = entry;
    }
    entry->previous = NOT_PTR_NULL;
    st->begin = entry;
}

static void
not_symbol_table_unlink(not_entry_t *entry)
{
    not_symbol_table_t *st = not_symbol_table_get();

    if (st->begin == entry)
    {
        st->begin = entry->next;
    }

    if (entry->next)
    {
        entry->next->previous = entry->previous;
    }

    if (entry->previous)
    {
        entry->previous->next = entry->next;
    }

    entry->previous = entry->next = NOT_PTR_NULL;
}

not_entry_t *
not_symbol_table_push(not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value)
{
    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (!entry)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;
    entry->next = entry->previous = entry;

    not_symbol_table_t *st = not_symbol_table_get();

    if (not_mutex_lock(&st->lock) < 0)
    {
        not_memory_free(entry);
        not_error_system("'%s' could not lock", "not_tymbol_table.lock");
        return NOT_PTR_ERROR;
    }

    not_symbol_table_link(entry);

    if (not_mutex_unlock(&st->lock) < 0)
    {
        not_memory_free(entry);
        not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
        return NOT_PTR_ERROR;
    }

    return entry;
}

not_entry_t *
not_symbol_table_find(not_node_t *scope, not_node_t *key)
{
    not_symbol_table_t *st = not_symbol_table_get();

    if (not_mutex_lock(&st->lock) < 0)
    {
        not_error_system("'%s' could not lock", "not_tymbol_table.lock");
        return NOT_PTR_ERROR;
    }

    not_entry_t *a1;
    for (a1 = st->begin; a1 != NOT_PTR_NULL; a1 = a1->next)
    {
        if ((a1->scope->id == scope->id) && (not_symbol_table_id_cmp(a1->key, key) == 1))
        {
            if (not_mutex_unlock(&st->lock) < 0)
            {
                not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
                return NOT_PTR_ERROR;
            }
            if (a1->value)
            {
                not_record_link_increase(a1->value);
            }
            return a1;
        }
    }

    if (not_mutex_unlock(&st->lock) < 0)
    {
        not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
        return NOT_PTR_ERROR;
    }

    return NOT_PTR_NULL;
}
