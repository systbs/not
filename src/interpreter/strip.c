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

not_strip_t *
not_strip_create(not_strip_t *previous)
{
    not_strip_t *strip = (not_strip_t *)not_memory_calloc(1, sizeof(not_strip_t));
    if (strip == NOT_PTR_ERROR)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    strip->inputs = not_queue_create();
    if (strip->inputs == NOT_PTR_ERROR)
    {
        not_memory_free(strip);
        return NOT_PTR_ERROR;
    }

    strip->variables = not_queue_create();
    if (strip->variables == NOT_PTR_ERROR)
    {
        not_memory_free(strip);
        not_queue_destroy(strip->inputs);
        return NOT_PTR_ERROR;
    }

    strip->previous = previous;

    return strip;
}

not_entry_t *
not_strip_variable_push(not_strip_t *strip, not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value)
{
    for (not_queue_entry_t *a1 = strip->variables->begin; a1 != strip->variables->end; a1 = a1->next)
    {
        not_entry_t *item = (not_entry_t *)a1->value;
        if ((item->scope->id == scope->id) && (not_helper_id_cmp(item->key, key) == 0))
        {
            return NULL;
        }
    }

    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (entry == NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    if (NOT_PTR_ERROR == not_queue_right_push(strip->variables, entry))
    {
        not_memory_free(entry);
        return NOT_PTR_ERROR;
    }

    return entry;
}

not_entry_t *
not_strip_variable_find(not_strip_t *strip, not_node_t *scope, not_node_t *key)
{
    for (not_queue_entry_t *a1 = strip->variables->begin; a1 != strip->variables->end; a1 = a1->next)
    {
        not_entry_t *entry = (not_entry_t *)a1->value;
        if ((entry->scope->id == scope->id) && not_helper_id_cmp(entry->key, key) == 0)
        {
            if (entry->value)
            {
                not_record_link_increase(entry->value);
            }
            return entry;
        }
    }

    if (strip->previous)
    {
        not_entry_t *entry = not_strip_variable_find(strip->previous, scope, key);
        if (entry != NULL)
        {
            return entry;
        }
    }

    return NULL;
}

not_entry_t *
not_strip_input_find(not_strip_t *strip, not_node_t *scope, not_node_t *key)
{
    for (not_queue_entry_t *a1 = strip->inputs->begin; a1 != strip->inputs->end; a1 = a1->next)
    {
        not_entry_t *entry = (not_entry_t *)a1->value;
        if ((entry->scope->id == scope->id) && not_helper_id_cmp(entry->key, key) == 0)
        {
            if (entry->value)
            {
                not_record_link_increase(entry->value);
            }
            return entry;
        }
    }

    return NULL;
}

not_entry_t *
not_strip_input_push(not_strip_t *strip, not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value)
{
    for (not_queue_entry_t *a1 = strip->inputs->begin; a1 != strip->inputs->end; a1 = a1->next)
    {
        not_entry_t *item = (not_entry_t *)a1->value;
        if (not_helper_id_cmp(item->key, key) == 0)
        {
            return NULL;
        }
    }

    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (entry == NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    if (NOT_PTR_ERROR == not_queue_right_push(strip->inputs, entry))
    {
        not_memory_free(entry);
        return NOT_PTR_ERROR;
    }

    return entry;
}

int32_t
not_strip_variable_remove_by_scope(not_strip_t *strip, not_node_t *scope)
{
    if (strip->previous)
    {
        if (not_strip_variable_remove_by_scope(strip->previous, scope) < 0)
        {
            return -1;
        }
    }

    for (not_queue_entry_t *a = strip->variables->begin, *b = NULL; a != strip->variables->end; a = b)
    {
        b = a->next;

        not_entry_t *entry = (not_entry_t *)a->value;
        if (entry->scope->id == scope->id)
        {
            if (not_record_link_decrease(entry->value) < 0)
            {
                return -1;
            }
            not_memory_free(entry);
            not_queue_unlink(strip->variables, a);
            not_memory_free(a);
        }
    }

    return 0;
}

int32_t
not_strip_destroy(not_strip_t *strip)
{
    if (!!strip->previous)
    {
        if (not_strip_destroy(strip->previous) < 0)
        {
            return 0;
        }

        strip->previous = NULL;
    }

    for (not_queue_entry_t *a = strip->variables->begin, *b = NULL; a != strip->variables->end; a = b)
    {
        b = a->next;
        not_entry_t *item = (not_entry_t *)a->value;

        if (not_record_link_decrease(item->value) < 0)
        {
            return -1;
        }

        not_memory_free(item);
        not_queue_unlink(strip->variables, a);
        not_memory_free(a);
    }

    not_queue_destroy(strip->variables);

    for (not_queue_entry_t *a = strip->inputs->begin, *b = NULL; a != strip->inputs->end; a = b)
    {
        b = a->next;
        not_entry_t *item = (not_entry_t *)a->value;

        if (not_record_link_decrease(item->value) < 0)
        {
            return -1;
        }

        not_memory_free(item);
        not_queue_unlink(strip->inputs, a);
        not_memory_free(a);
    }

    not_queue_destroy(strip->inputs);
    not_memory_free(strip);

    return 0;
}

void not_strip_attach(not_strip_t *strip, not_strip_t *previous)
{
    not_strip_t *strip_itr = strip;
    while (strip_itr && strip_itr->previous)
    {
        strip_itr = strip_itr->previous;
    }
    if (strip_itr)
    {
        strip_itr->previous = previous;
    }
}

not_strip_t *
not_strip_copy(not_strip_t *strip)
{
    not_strip_t *strip_previous = NULL;
    if (strip->previous)
    {
        strip_previous = not_strip_copy(strip->previous);
        if (strip_previous == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }
    }

    not_strip_t *strip_copy = not_strip_create(strip_previous);
    if (strip_copy == NOT_PTR_ERROR)
    {
        if (strip_previous)
        {
            if (not_strip_destroy(strip_previous) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }

        return NOT_PTR_ERROR;
    }

    for (not_queue_entry_t *a = strip->variables->begin; a != strip->variables->end; a = a->next)
    {
        not_entry_t *item = (not_entry_t *)a->value;

        not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
        if (entry == NULL)
        {
            not_error_no_memory();
            not_strip_destroy(strip_copy);
            return NOT_PTR_ERROR;
        }

        not_record_link_increase(item->value);

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = item->value;
        entry->block = item->block;

        if (NOT_PTR_ERROR == not_queue_right_push(strip_copy->variables, entry))
        {
            not_memory_free(entry);
            not_strip_destroy(strip_copy);
            return NOT_PTR_ERROR;
        }
    }

    /*
    for (not_queue_entry_t *a = strip->inputs->begin; a != strip->inputs->end; a = a->next)
    {
        not_entry_t *item = (not_entry_t *)a->value;

        not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
        if (entry == NULL)
        {
            not_error_no_memory();
            not_strip_destroy(strip_copy);
            return NOT_PTR_ERROR;
        }

        not_record_link_increase(item->value);

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = item->value;
        entry->block = item->block;

        if (NOT_PTR_ERROR == not_queue_right_push(strip_copy->inputs, entry))
        {
            not_memory_free(entry);
            not_strip_destroy(strip_copy);
            return NOT_PTR_ERROR;
        }
    }
    */

    return strip_copy;
}