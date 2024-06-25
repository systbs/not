#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>
#include <ffi.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../mutex.h"
#include "../memory.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "../repository.h"
#include "../interpreter.h"
#include "../thread.h"
#include "symbol_table.h"
#include "strip.h"
#include "entry.h"
#include "helper.h"
#include "execute.h"
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

    st->queue_variables = not_queue_create();
    if (st->queue_variables == NOT_PTR_ERROR)
    {
        if (not_mutex_destroy(&st->lock) < 0)
        {
            not_error_system("'%s' could not destroy the lock", "not_symbol_table.lock");
            return -1;
        }
        return -1;
    }

    return 0;
}

int32_t
not_symbol_table_destroy()
{
    not_symbol_table_t *st = not_symbol_table_get();

    if (not_mutex_lock(&st->lock) < 0)
    {
        not_error_system("'%s' could not lock", "not_tymbol_table.lock");
        return -1;
    }

    for (not_queue_entry_t *n = st->queue_variables->begin, *b; n != st->queue_variables->end; n = b)
    {
        b = n->next;
        not_entry_t *entry = (not_entry_t *)n->value;

        not_record_link_decrease(entry->value);

        not_memory_free(entry);
        not_queue_unlink(st->queue_variables, n);
        not_memory_free(n);
    }

    not_queue_destroy(st->queue_variables);

    if (not_mutex_unlock(&st->lock) < 0)
    {
        not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
        return -1;
    }

    if (not_mutex_destroy(&st->lock) < 0)
    {
        not_error_system("'%s' could not destroy the lock", "not_symbol_table.lock");
        return -1;
    }

    return 0;
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

    if (NOT_PTR_ERROR == not_queue_right_push(st->queue_variables, entry))
    {
        not_memory_free(entry);
        if (not_mutex_unlock(&st->lock) < 0)
        {
            not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
            return NOT_PTR_ERROR;
        }
        return NOT_PTR_ERROR;
    }

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

    for (not_queue_entry_t *a1 = st->queue_variables->begin; a1 != st->queue_variables->end; a1 = a1->next)
    {
        not_entry_t *entry = (not_entry_t *)a1->value;
        if ((entry->scope->id == scope->id) && (not_helper_id_cmp(entry->key, key) == 0))
        {
            if (not_mutex_unlock(&st->lock) < 0)
            {
                not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
                return NOT_PTR_ERROR;
            }
            if (entry->value)
            {
                not_record_link_increase(entry->value);
            }
            return entry;
        }
    }

    if (not_mutex_unlock(&st->lock) < 0)
    {
        not_error_system("'%s' could not unlock", "not_tymbol_table.lock");
        return NOT_PTR_ERROR;
    }

    return NULL;
}
