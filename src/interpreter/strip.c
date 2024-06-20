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
#include "helper.h"
#include "strip.h"

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
        if (not_helper_id_cmp(item->key, key) == 0)
        {
            return NOT_PTR_NULL;
        }
    }

    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (entry == NOT_PTR_NULL)
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
        if (entry == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }
        else if (entry != NOT_PTR_NULL)
        {
            return entry;
        }
    }

    return NOT_PTR_NULL;
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

    return NOT_PTR_NULL;
}

not_entry_t *
not_strip_input_push(not_strip_t *strip, not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value)
{
    for (not_queue_entry_t *a1 = strip->inputs->begin; a1 != strip->inputs->end; a1 = a1->next)
    {
        not_entry_t *item = (not_entry_t *)a1->value;
        if (not_helper_id_cmp(item->key, key) == 0)
        {
            return NOT_PTR_NULL;
        }
    }

    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (entry == NOT_PTR_NULL)
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
    for (not_queue_entry_t *a = strip->variables->begin, *b = NOT_PTR_NULL; a != strip->variables->end; a = b)
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

    if (strip->previous)
    {
        if (not_strip_variable_remove_by_scope(strip->previous, scope) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int32_t
not_strip_destroy(not_strip_t *strip)
{
    if (strip->previous)
    {
        if (not_strip_destroy(strip->previous) < 0)
        {
            return 0;
        }

        strip->previous = NOT_PTR_NULL;
    }

    for (not_queue_entry_t *a = strip->variables->begin, *b = NOT_PTR_NULL; a != strip->variables->end; a = b)
    {
        b = a->next;
        not_entry_t *item = (not_entry_t *)a->value;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                not_node_var_t *var1 = (not_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_ENTITY)
            {
                not_node_entity_t *entity1 = (not_node_entity_t *)item->block->value;
                if ((entity1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PROPERTY)
            {
                not_node_property_t *property1 = (not_node_property_t *)item->block->value;
                if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PARAMETER)
            {
                not_node_parameter_t *parameter1 = (not_node_parameter_t *)item->block->value;
                if ((parameter1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else
            {
                if (not_record_link_decrease(item->value) < 0)
                {
                    return -1;
                }
            }
        }

        not_memory_free(item);
        not_queue_unlink(strip->variables, a);
        not_memory_free(a);
    }

    for (not_queue_entry_t *a = strip->inputs->begin, *b = NOT_PTR_NULL; a != strip->inputs->end; a = b)
    {
        b = a->next;
        not_entry_t *item = (not_entry_t *)a->value;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                not_node_var_t *var1 = (not_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_ENTITY)
            {
                not_node_entity_t *entity1 = (not_node_entity_t *)item->block->value;
                if ((entity1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PROPERTY)
            {
                not_node_property_t *property1 = (not_node_property_t *)item->block->value;
                if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PARAMETER)
            {
                not_node_parameter_t *parameter1 = (not_node_parameter_t *)item->block->value;
                if ((parameter1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (not_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else
            {
                if (not_record_link_decrease(item->value) < 0)
                {
                    return -1;
                }
            }
        }

        not_memory_free(item);
        not_queue_unlink(strip->inputs, a);
        not_memory_free(a);
    }

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
    if (strip == NOT_PTR_NULL)
    {
        return NOT_PTR_NULL;
    }

    not_strip_t *strip_previous = NOT_PTR_NULL;
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
        if (entry == NOT_PTR_NULL)
        {
            not_error_no_memory();
            if (not_strip_destroy(strip_copy) < 0)
            {
                return NOT_PTR_ERROR;
            }
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
            if (not_strip_destroy(strip_copy) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }
    }

    for (not_queue_entry_t *a = strip->inputs->begin; a != strip->inputs->end; a = a->next)
    {
        not_entry_t *item = (not_entry_t *)a->value;

        not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
        if (entry == NOT_PTR_NULL)
        {
            not_error_no_memory();
            if (not_strip_destroy(strip_copy) < 0)
            {
                return NOT_PTR_ERROR;
            }
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
            if (not_strip_destroy(strip_copy) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }
    }

    return strip_copy;
}