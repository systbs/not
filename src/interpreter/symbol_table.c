#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>

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

sy_symbol_table_t base_symbol_table;

static sy_symbol_table_t *
sy_symbol_table_get()
{
    return &base_symbol_table;
}

int32_t
sy_symbol_table_init()
{
    sy_symbol_table_t *st = sy_symbol_table_get();
	if (sy_mutex_init(&st->lock) < 0)
	{
		sy_error_system("'%s' could not initialize the lock", "sy_symbol_table.lock");
		return -1;
	}

    st->begin = NULL;

	return 0;
}

int32_t
sy_symbol_table_destroy()
{
    sy_symbol_table_t *st = sy_symbol_table_get();
    if (sy_mutex_destroy(&st->lock) < 0)
    {
        sy_error_system("'%s' could not destroy the lock", "sy_symbol_table.lock");
        return -1;
    }
    return 0;
}

static int32_t
sy_symbol_table_id_cmp(sy_node_t *n1, sy_node_t *n2)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

	return (strcmp(nb1->value, nb2->value) == 0);
}

static void
sy_symbol_table_link(sy_entry_t *current, sy_entry_t *entry)
{
    sy_symbol_table_t *st = sy_symbol_table_get();

    entry->next = current;
    if (current)
    {
        entry->previous = current->previous;
        current->previous = entry;
        if (current->previous)
        {
            current->previous->next = entry;
        }
    }
    
    if(st->begin == current)
    {
        st->begin = entry;
    }
}

static void
sy_symbol_table_unlink(sy_entry_t *entry)
{
    sy_symbol_table_t *st = sy_symbol_table_get();

    if (entry == st->begin)
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
}

sy_entry_t *
sy_symbol_table_push(sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value)
{
    sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
    if(!entry)
    {
        sy_error_no_memory();
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;
    entry->next = entry->previous = entry;

    sy_symbol_table_t *st = sy_symbol_table_get();

    if (sy_mutex_lock(&st->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not lock", "sy_tymbol_table.lock");
        return ERROR;
    }

    sy_symbol_table_link(st->begin, entry);

    if (sy_mutex_unlock(&st->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not unlock", "sy_tymbol_table.lock");
        return ERROR;
    }

    return entry;
}

sy_entry_t *
sy_symbol_table_find(sy_node_t *scope, sy_node_t *key)
{
    sy_symbol_table_t *st = sy_symbol_table_get();

    if (sy_mutex_lock(&st->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_tymbol_table.lock");
        return ERROR;
    }

    sy_entry_t *a1;
    for (a1 = st->begin;a1 != NULL;a1 = a1->next)
    {
        if ((a1->scope->id == scope->id) && (sy_symbol_table_id_cmp(a1->key, key) == 1))
        {
            if (sy_mutex_unlock(&st->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_tymbol_table.lock");
                return ERROR;
            }
            return a1;
        }
    }

    if (sy_mutex_unlock(&st->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_tymbol_table.lock");
        return ERROR;
    }

    return NULL;
}
