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

    if (sy_mutex_init(&strip->lock) < 0)
	{
        sy_memory_free(strip);
		sy_error_system("'%s' could not initialize the lock", "sy_strip.lock");
		return ERROR;
	}

    return strip;
}

static void
sy_strip_variable_link(sy_strip_t *strip, sy_entry_t *current, sy_entry_t *entry)
{
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
    
    if(strip->variables == current)
    {
        strip->variables = entry;
    }
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
sy_strip_input_link(sy_strip_t *strip, sy_entry_t *current, sy_entry_t *entry)
{
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
    
    if(strip->inputs == current)
    {
        strip->inputs = entry;
    }
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
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    for (sy_entry_t *item = strip->variables;item != NULL;item = item->next)
    {
        if (sy_strip_id_cmp(item->key, key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)key->value;
            sy_error_semantic_by_node(key, "'%s' already defined, previous in (%lld:%lld)\n\tMajor:%s-%u",
                basic1->value, item->key->position.line, item->key->position.column, __FILE__, __LINE__);

            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }

            return ERROR;
        }
    }

    sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
    if(entry == NULL)
    {
        sy_error_no_memory();
        if (sy_mutex_unlock(&strip->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_strip.lock");
            return ERROR;
        }
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    sy_strip_variable_link(strip, strip->variables, entry);

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return entry;
}

sy_entry_t *
sy_strip_variable_find(sy_strip_t *strip, sy_node_t *key)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    for (sy_entry_t *entry = strip->variables;entry != NULL;entry = entry->next)
    {
        if (sy_strip_id_cmp(entry->key, key) == 1)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return entry;
        }
    }

    if (strip->previous)
    {
        sy_entry_t *entry = sy_strip_variable_find(strip->previous, key);
        if (entry == ERROR)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }
        else
        if (entry != NULL)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return entry;
        }
    }

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return NULL;
}

sy_entry_t *
sy_strip_input_find(sy_strip_t *strip, sy_node_t *key)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    for (sy_entry_t *entry = strip->inputs;entry != NULL;entry = entry->next)
    {
        if (sy_strip_id_cmp(entry->key, key) == 1)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return entry;
        }
    }


    if (strip->previous)
    {
        sy_entry_t *entry = sy_strip_input_find(strip->previous, key);
        if (entry == ERROR)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }
        else
        if (entry != NULL)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return entry;
        }
    }

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return NULL;
}

sy_entry_t *
sy_strip_input_push(sy_strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    for (sy_entry_t *item = strip->inputs;item != NULL;item = item->next)
    {
        if (sy_strip_id_cmp(item->key, key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)key->value;
            sy_error_semantic_by_node(key, "'%s' already defined, previous in (%lld:%lld)\n\tMajor:%s-%u",
                basic1->value, item->key->position.line, item->key->position.column, __FILE__, __LINE__);

            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            
            return ERROR;
        }
    }

    sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
    if(entry == NULL)
    {
        sy_error_no_memory();
        if (sy_mutex_unlock(&strip->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_strip.lock");
            return ERROR;
        }
        return ERROR;
    }

    entry->scope = scope;
    entry->key = key;
    entry->value = value;
    entry->block = block;

    sy_strip_input_link(strip, strip->inputs, entry);

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return entry;
}

int32_t
sy_strip_destroy(sy_strip_t *strip)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return -1;
    }

    if (strip->previous)
    {
        if (sy_strip_destroy(strip->previous) < 0)
        {
            if (sy_mutex_lock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not lock", "sy_strip.lock");
                return -1;
            }
            return -1;
        }
    }

    for (sy_entry_t *item = strip->variables, *next = NULL;item != NULL;item = next)
    {
        next = item->next;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            if (item->block->kind == NODE_KIND_ENTITY)
            {
                sy_node_entity_t *entity1 = (sy_node_entity_t *)item->block->value;
                if ((entity1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            if (item->block->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item->block->value;
                if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            if (item->block->kind == NODE_KIND_PARAMETER)
            {
                sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item->block->value;
                if ((parameter1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            {
                item->value->link = 0;
                if (sy_record_destroy(item->value) < 0)
                {
                    if (sy_mutex_lock(&strip->lock) < 0)
                    {
                        sy_error_system("'%s' could not lock", "sy_strip.lock");
                        return -1;
                    }
                    return -1;
                }
            }
        }

        sy_strip_variable_unlink(strip, item);
        sy_memory_free(item);
    }

    for (sy_entry_t *item = strip->inputs, *next = NULL;item != NULL;item = next)
    {
        next = item->next;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            if (item->block->kind == NODE_KIND_ENTITY)
            {
                sy_node_entity_t *entity1 = (sy_node_entity_t *)item->block->value;
                if ((entity1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            if (item->block->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item->block->value;
                if ((property1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            if (item->block->kind == NODE_KIND_PARAMETER)
            {
                sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item->block->value;
                if ((parameter1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->link = 0;
                    if (sy_record_destroy(item->value) < 0)
                    {
                        if (sy_mutex_lock(&strip->lock) < 0)
                        {
                            sy_error_system("'%s' could not lock", "sy_strip.lock");
                            return -1;
                        }
                        return -1;
                    }
                }
            }
            else
            {
                item->value->link = 0;
                if (sy_record_destroy(item->value) < 0)
                {
                    if (sy_mutex_lock(&strip->lock) < 0)
                    {
                        sy_error_system("'%s' could not lock", "sy_strip.lock");
                        return -1;
                    }
                    return -1;
                }
            }
        }

        sy_strip_input_unlink(strip, item);
        sy_memory_free(item);
    }

    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return -1;
    }

    if (sy_mutex_destroy(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not destroy the lock", "sy_strip.lock");
        return -1;
    }

    sy_memory_free(strip);

    return 0;
}

sy_strip_t *
sy_strip_copy(sy_strip_t *strip)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    sy_strip_t *strip_previous = NULL;
    if (strip->previous)
    {
        strip_previous = sy_strip_copy(strip->previous);
        if (strip_previous == ERROR)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }
    }

    sy_strip_t *strip_copy = sy_strip_create(strip_previous);
    if (strip_copy == ERROR)
    {
        if (sy_mutex_unlock(&strip->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_strip.lock");
            return ERROR;
        }
        return ERROR;
    }

    if (sy_mutex_init(&strip_copy->lock) < 0)
	{
        if (sy_strip_destroy(strip_copy) < 0)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }
		sy_error_system("'%s' could not initialize the lock", "sy_strip.lock");
        if (sy_mutex_unlock(&strip->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_strip.lock");
            return ERROR;
        }
		return ERROR;
	}

    for (sy_entry_t *item = strip->variables;item != NULL;item = item->next)
    {
        sy_record_t *record = sy_record_copy(item->value);
        if (record == ERROR)
        {
            if (sy_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }

        sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
        if(entry == NULL)
        {
            sy_error_no_memory();
            if (sy_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }

        record->link = 1;

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = record;
        entry->block = item->block;

        sy_strip_variable_link(strip_copy, strip_copy->variables, entry);
    }

    for (sy_entry_t *item = strip->inputs;item != NULL;item = item->next)
    {
        sy_record_t *record = sy_record_copy(item->value);
        if (record == ERROR)
        {
            if (sy_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }

        sy_entry_t *entry = (sy_entry_t *)sy_memory_calloc(1, sizeof(sy_entry_t));
        if(entry == NULL)
        {
            sy_error_no_memory();
            if (sy_strip_destroy(strip_copy) < 0)
            {
                return ERROR;
            }
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return ERROR;
        }

        record->link = 1;

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = record;
        entry->block = item->block;

        sy_strip_input_link(strip_copy, strip_copy->inputs, entry);
    }

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return strip_copy;
}