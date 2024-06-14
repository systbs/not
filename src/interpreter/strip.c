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
sy_strip_id_cmp(sy_node_t *n1, sy_node_t *n2)
{
    sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
    sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

    return (strcmp(nb1->value, nb2->value) == 0);
}

sy_strip_t *
sy_strip_create(sy_strip_t *previous)
{
    sy_strip_t *strip = (sy_strip_t *)sy_memory_calloc(1, sizeof(sy_strip_t));
    if (strip == ERROR)
    {
        sy_error_no_memory();
        return ERROR;
    }

    strip->inputs = NULL;
    strip->variables = NULL;
    strip->previous = previous;

    return strip;
}

static void
sy_strip_variable_link(sy_strip_t *strip, sy_entry_t *entry)
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
sy_strip_variable_unlink(sy_strip_t *strip, sy_entry_t *entry)
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
sy_strip_input_link(sy_strip_t *strip, sy_entry_t *entry)
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
sy_strip_input_unlink(sy_strip_t *strip, sy_entry_t *entry)
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

sy_entry_t *
sy_strip_variable_push(sy_strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value)
{
    for (sy_entry_t *item = strip->variables; item != NULL; item = item->next)
    {
        if (sy_strip_id_cmp(item->key, key) == 1)
        {
            return NULL;
        }
    }

    sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
    if (entry == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    sy_strip_variable_link(strip, entry);

    return entry;
}

sy_entry_t *
sy_strip_variable_find(sy_strip_t *strip, sy_node_t *scope, sy_node_t *key)
{
    for (sy_entry_t *entry = strip->variables; entry != NULL; entry = entry->next)
    {
        if ((entry->scope->id == scope->id) && sy_strip_id_cmp(entry->key, key) == 1)
        {
            if (entry->value)
            {
                sy_record_link_increase(entry->value);
            }
            return entry;
        }
    }

    if (strip->previous)
    {
        sy_entry_t *entry = sy_strip_variable_find(strip->previous, scope, key);
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

sy_entry_t *
sy_strip_input_find(sy_strip_t *strip, sy_node_t *scope, sy_node_t *key)
{
    for (sy_entry_t *entry = strip->inputs; entry != NULL; entry = entry->next)
    {
        if ((entry->scope->id == scope->id) && sy_strip_id_cmp(entry->key, key) == 1)
        {
            if (entry->value)
            {
                sy_record_link_increase(entry->value);
            }
            return entry;
        }
    }

    return NULL;
}

sy_entry_t *
sy_strip_input_push(sy_strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value)
{
    for (sy_entry_t *item = strip->inputs; item != NULL; item = item->next)
    {
        if (sy_strip_id_cmp(item->key, key) == 1)
        {
            return NULL;
        }
    }

    sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
    if (entry == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    sy_strip_input_link(strip, entry);

    return entry;
}

int32_t
sy_strip_variable_remove_by_scope(sy_strip_t *strip, sy_node_t *scope)
{
    for (sy_entry_t *entry = strip->variables, *b = NULL; entry != NULL; entry = b)
    {
        b = entry->next;
        if (entry->scope->id == scope->id)
        {
            sy_strip_variable_unlink(strip, entry);
            entry->value -= 1;
            sy_memory_free(entry);
        }
    }

    if (strip->previous)
    {
        if (sy_strip_variable_remove_by_scope(strip->previous, scope) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int32_t
sy_strip_destroy(sy_strip_t *strip)
{
    if (strip->previous)
    {
        if (sy_strip_destroy(strip->previous) < 0)
        {
            return 0;
        }

        strip->previous = NULL;
    }

    for (sy_entry_t *item = strip->variables, *next = NULL; item != NULL; item = next)
    {
        next = item->next;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_ENTITY)
            {
                sy_node_entity_t *entity1 = (sy_node_entity_t *)item->block->value;
                if ((entity1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item->block->value;
                if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PARAMETER)
            {
                sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item->block->value;
                if ((parameter1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else
            {
                if (sy_record_link_decrease(item->value) < 0)
                {
                    return -1;
                }
            }
        }

        sy_strip_variable_unlink(strip, item);
        sy_memory_free(item);
    }

    for (sy_entry_t *item = strip->inputs, *next = NULL; item != NULL; item = next)
    {
        next = item->next;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_ENTITY)
            {
                sy_node_entity_t *entity1 = (sy_node_entity_t *)item->block->value;
                if ((entity1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item->block->value;
                if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (item->block->kind == NODE_KIND_PARAMETER)
            {
                sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item->block->value;
                if ((parameter1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (sy_record_link_decrease(item->value) < 0)
                    {
                        return -1;
                    }
                }
            }
            else
            {
                if (sy_record_link_decrease(item->value) < 0)
                {
                    return -1;
                }
            }
        }

        sy_strip_input_unlink(strip, item);
        sy_memory_free(item);
    }

    sy_memory_free(strip);

    return 0;
}

sy_strip_t *
sy_strip_copy(sy_strip_t *strip)
{
    sy_strip_t *strip_previous = NULL;
    if (strip->previous)
    {
        strip_previous = sy_strip_copy(strip->previous);
        if (strip_previous == ERROR)
        {
            return ERROR;
        }
    }

    sy_strip_t *strip_copy = sy_strip_create(strip_previous);
    if (strip_copy == ERROR)
    {
        if (strip_previous)
        {
            if (sy_strip_destroy(strip_previous) < 0)
            {
                return ERROR;
            }
        }

        return ERROR;
    }

    for (sy_entry_t *item = strip->variables; item != NULL; item = item->next)
    {
        sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
        if (entry == NULL)
        {
            sy_error_no_memory();
            if (sy_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }

        sy_record_link_increase(item->value);

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = item->value;
        entry->block = item->block;

        sy_strip_variable_link(strip_copy, entry);
    }

    for (sy_entry_t *item = strip->inputs; item != NULL; item = item->next)
    {
        sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
        if (entry == NULL)
        {
            sy_error_no_memory();
            if (sy_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }

        sy_record_link_increase(item->value);

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = item->value;
        entry->block = item->block;

        sy_strip_input_link(strip_copy, entry);
    }
    return strip_copy;
}