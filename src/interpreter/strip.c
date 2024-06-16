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
#include "strip.h"
#include "garbage.h"

static int32_t
not_strip_id_cmp(not_node_t *n1, not_node_t *n2)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
    not_node_basic_t *nb2 = (not_node_basic_t *)n2->value;

    return (strcmp(nb1->value, nb2->value) == 0);
}

not_strip_t *
not_strip_create(not_strip_t *previous)
{
    not_strip_t *strip = (not_strip_t *)not_memory_calloc(1, sizeof(not_strip_t));
    if (strip == ERROR)
    {
        not_error_no_memory();
        return ERROR;
    }

    strip->inputs = NULL;
    strip->variables = NULL;
    strip->previous = previous;

    return strip;
}

static void
not_strip_variable_link(not_strip_t *strip, not_entry_t *entry)
{
    entry->next = strip->variables;
    if (strip->variables)
    {
        strip->variables->previous = entry;
    }
    entry->previous = NULL;
    strip->variables = entry;
}

static void
not_strip_variable_unlink(not_strip_t *strip, not_entry_t *entry)
{
    if (entry == strip->variables)
    {
        strip->variables = entry->next;
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

static void
not_strip_input_link(not_strip_t *strip, not_entry_t *entry)
{
    entry->next = strip->inputs;
    if (strip->inputs)
    {
        strip->inputs->previous = entry;
    }
    entry->previous = NULL;
    strip->inputs = entry;
}

static void
not_strip_input_unlink(not_strip_t *strip, not_entry_t *entry)
{
    if (entry == strip->inputs)
    {
        strip->inputs = entry->next;
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

not_entry_t *
not_strip_variable_push(not_strip_t *strip, not_node_t *scope, not_node_t *block, not_node_t *key, not_record_t *value)
{
    for (not_entry_t *item = strip->variables; item != NULL; item = item->next)
    {
        if (not_strip_id_cmp(item->key, key) == 1)
        {
            return NULL;
        }
    }

    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (entry == NULL)
    {
        not_error_no_memory();
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    not_strip_variable_link(strip, entry);

    return entry;
}

not_entry_t *
not_strip_variable_find(not_strip_t *strip, not_node_t *scope, not_node_t *key)
{
    for (not_entry_t *entry = strip->variables; entry != NULL; entry = entry->next)
    {
        if ((entry->scope->id == scope->id) && not_strip_id_cmp(entry->key, key) == 1)
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
        if (entry == ERROR)
        {
            return ERROR;
        }
        else if (entry != NULL)
        {
            return entry;
        }
    }

    return NULL;
}

not_entry_t *
not_strip_input_find(not_strip_t *strip, not_node_t *scope, not_node_t *key)
{
    for (not_entry_t *entry = strip->inputs; entry != NULL; entry = entry->next)
    {
        if ((entry->scope->id == scope->id) && not_strip_id_cmp(entry->key, key) == 1)
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
    for (not_entry_t *item = strip->inputs; item != NULL; item = item->next)
    {
        if (not_strip_id_cmp(item->key, key) == 1)
        {
            return NULL;
        }
    }

    not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
    if (entry == NULL)
    {
        not_error_no_memory();
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    not_strip_input_link(strip, entry);

    return entry;
}

int32_t
not_strip_variable_remove_by_scope(not_strip_t *strip, not_node_t *scope)
{
    for (not_entry_t *entry = strip->variables, *b = NULL; entry != NULL; entry = b)
    {
        b = entry->next;
        if (entry->scope->id == scope->id)
        {
            not_strip_variable_unlink(strip, entry);
            entry->value -= 1;
            not_memory_free(entry);
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

        strip->previous = NULL;
    }

    for (not_entry_t *item = strip->variables, *next = NULL; item != NULL; item = next)
    {
        next = item->next;

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

        not_strip_variable_unlink(strip, item);
        not_memory_free(item);
    }

    for (not_entry_t *item = strip->inputs, *next = NULL; item != NULL; item = next)
    {
        next = item->next;

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

        not_strip_input_unlink(strip, item);
        not_memory_free(item);
    }

    not_memory_free(strip);

    return 0;
}

not_strip_t *
not_strip_copy(not_strip_t *strip)
{
    not_strip_t *strip_previous = NULL;
    if (strip->previous)
    {
        strip_previous = not_strip_copy(strip->previous);
        if (strip_previous == ERROR)
        {
            return ERROR;
        }
    }

    not_strip_t *strip_copy = not_strip_create(strip_previous);
    if (strip_copy == ERROR)
    {
        if (strip_previous)
        {
            if (not_strip_destroy(strip_previous) < 0)
            {
                return ERROR;
            }
        }

        return ERROR;
    }

    for (not_entry_t *item = strip->variables; item != NULL; item = item->next)
    {
        not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
        if (entry == NULL)
        {
            not_error_no_memory();
            if (not_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }

        not_record_link_increase(item->value);

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = item->value;
        entry->block = item->block;

        not_strip_variable_link(strip_copy, entry);
    }

    for (not_entry_t *item = strip->inputs; item != NULL; item = item->next)
    {
        not_entry_t *entry = (not_entry_t *)not_memory_calloc(1, sizeof(not_entry_t));
        if (entry == NULL)
        {
            not_error_no_memory();
            if (not_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }

        not_record_link_increase(item->value);

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = item->value;
        entry->block = item->block;

        not_strip_input_link(strip_copy, entry);
    }
    return strip_copy;
}