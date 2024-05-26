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
#include "strip.h"
#include "garbage.h"

static int32_t
sy_strip_id_cmp(sy_node_t *n1, sy_node_t *n2)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

	return (strcmp(nb1->value, nb2->value) == 0);
}

strip_t *
sy_strip_create(strip_t *previous)
{
    strip_t *strip = (strip_t *)sy_memory_calloc(1, sizeof(strip_t));
    if (strip == ERROR)
    {
        sy_error_no_memory();
        return ERROR;
    }

    strip->inputs = sy_queue_create();
    if (strip->inputs == ERROR)
    {
        sy_memory_free(strip);
        return ERROR;
    }

    strip->variables = sy_queue_create();
    if (strip->variables == ERROR)
    {
        sy_memory_free(strip);
        return ERROR;
    }

    strip->previous = previous;

    if (sy_mutex_init(&strip->lock) < 0)
	{
        sy_memory_free(strip);
		sy_error_system("'%s' could not initialize the lock", "sy_strip.lock");
		return ERROR;
	}

    return strip;
}

sy_strip_entry_t *
sy_strip_variable_push(strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    sy_queue_entry_t *a1;
    for (a1 = strip->variables->begin;a1 != strip->variables->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;
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

    sy_strip_entry_t *entry = (sy_strip_entry_t *)sy_memory_calloc(1, sizeof(sy_strip_entry_t));
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

    if (ERROR == sy_queue_right_push(strip->variables, entry))
    {
        sy_memory_free(entry);
        if (sy_mutex_unlock(&strip->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_strip.lock");
            return ERROR;
        }
        return ERROR;
    }

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return entry;
}

sy_strip_entry_t *
sy_strip_variable_find(strip_t *strip, sy_node_t *key)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    sy_queue_entry_t *a1;
    for (a1 = strip->variables->begin;a1 != strip->variables->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;
        if (sy_strip_id_cmp(item->key, key) == 1)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return item;
        }
    }

    if (strip->previous)
    {
        sy_strip_entry_t *entry = sy_strip_variable_find(strip->previous, key);
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

sy_strip_entry_t *
sy_strip_input_find(strip_t *strip, sy_node_t *key)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    sy_queue_entry_t *a1;
    for (a1 = strip->inputs->begin;a1 != strip->inputs->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;
        if (sy_strip_id_cmp(item->key, key) == 1)
        {
            if (sy_mutex_unlock(&strip->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_strip.lock");
                return ERROR;
            }
            return item;
        }
    }

    if (strip->previous)
    {
        sy_strip_entry_t *entry = sy_strip_input_find(strip->previous, key);
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

sy_strip_entry_t *
sy_strip_input_push(strip_t *strip, sy_node_t *scope, sy_node_t *block, sy_node_t *key, sy_record_t *value)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    sy_queue_entry_t *a1;
    for (a1 = strip->inputs->begin;a1 != strip->inputs->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;
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

    sy_strip_entry_t *entry = (sy_strip_entry_t *)sy_memory_calloc(1, sizeof(sy_strip_entry_t));
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

    if (ERROR == sy_queue_right_push(strip->inputs, entry))
    {
        sy_memory_free(entry);
        if (sy_mutex_unlock(&strip->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_strip.lock");
            return ERROR;
        }
        return ERROR;
    }

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return entry;
}

int32_t
sy_strip_destroy(strip_t *strip)
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

    sy_queue_entry_t *a1;
    for (a1 = strip->variables->begin;a1 != strip->variables->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->reference = 0;
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
                    item->value->reference = 0;
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
                    item->value->reference = 0;
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
                    item->value->reference = 0;
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
                item->value->reference = 0;
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

        sy_memory_free(item);
    }

    sy_queue_destroy(strip->variables);

    for (a1 = strip->inputs->begin;a1 != strip->inputs->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;

        if (item->value)
        {
            if (item->block->kind == NODE_KIND_VAR)
            {
                sy_node_var_t *var1 = (sy_node_var_t *)item->block->value;
                if ((var1->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    item->value->reference = 0;
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
                    item->value->reference = 0;
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
                    item->value->reference = 0;
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
                    item->value->reference = 0;
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
                item->value->reference = 0;
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

        sy_memory_free(item);
    }

    sy_queue_destroy(strip->inputs);
    sy_memory_free(strip);

    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return -1;
    }

    return 0;
}

strip_t *
sy_strip_copy(strip_t *strip)
{
    if (sy_mutex_lock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_strip.lock");
        return ERROR;
    }

    strip_t *strip_previous = NULL;
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

    strip_t *strip_copy = sy_strip_create(strip_previous);
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

    sy_queue_entry_t *a1;
    for (a1 = strip->variables->begin;a1 != strip->variables->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;

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

        sy_strip_entry_t *entry = (sy_strip_entry_t *)sy_memory_calloc(1, sizeof(sy_strip_entry_t));
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

        record->reference = 1;

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = record;
        entry->block = item->block;

        if (ERROR == sy_queue_right_push(strip_copy->variables, entry))
        {
            sy_memory_free(entry);
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
    }

    for (a1 = strip->inputs->begin;a1 != strip->inputs->end;a1 = a1->next)
    {
        sy_strip_entry_t *item = (sy_strip_entry_t *)a1->value;

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

        sy_strip_entry_t *entry = (sy_strip_entry_t *)sy_memory_calloc(1, sizeof(sy_strip_entry_t));
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

        record->reference = 1;

        entry->scope = item->scope;
        entry->key = item->key;
        entry->value = record;
        entry->block = item->block;

        if (ERROR == sy_queue_right_push(strip_copy->inputs, entry))
        {
            sy_memory_free(entry);
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
    }

    if (sy_mutex_unlock(&strip->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_strip.lock");
        return ERROR;
    }

    return strip_copy;
}