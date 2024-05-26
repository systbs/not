#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
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
#include "../mutex.h"
#include "record.h"
#include "garbage.h"
#include "symbol_table.h"
#include "strip.h"

static int32_t
sy_execute_id_cmp(sy_node_t *n1, sy_node_t *n2)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

	return (strcmp(nb1->value, nb2->value) == 0);
}


int32_t 
sy_execute_entity(sy_node_t *scope, sy_node_t *node, sy_record_t *value, strip_t *strip)
{
    sy_node_entity_t *entity = (sy_node_entity_t *)node->value;

    sy_node_t *key_search = entity->key;
    if (entity->type)
    {
        key_search = entity->type;
    }

    sy_record_t *value_select = NULL;
    if (value->kind == RECORD_KIND_STRUCT)
    {
        value_select = sy_execute_attribute_from_struct(value, key_search);
        if (value_select == ERROR)
        {
            sy_error_runtime_by_node(value, "an error occurred while retriving the attribute from the struct\n\tMajor:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (value->kind == RECORD_KIND_TYPE)
    {
        value_select = sy_execute_attribute_from_type(value, key_search);
        if (value_select == ERROR)
        {
            sy_error_runtime_by_node(value, "an error occurred while retriving the attribute from the type\n\tMajor:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (value->kind == RECORD_KIND_OBJECT)
    {
        for (sy_record_object_t *object = value->value;object != NULL;object = object->next)
        {
            if (sy_execute_id_cmp(object->key, key_search) == 1)
            {
                value_select = object->value;
                break;
            }
        }
    }

    if (!value_select)
    {
        if (entity->value)
        {
            value_select = sy_execute_expression(entity->value, strip);
            if (value_select == ERROR)
            {
                sy_error_runtime_by_node(entity->value, "an error occurred during processing\n\tMajor:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
        }
    } 

    
}

int32_t 
sy_execute_set(sy_node_t *scope, sy_node_t *node, ey_node_t *value, strip_t *strip)
{
    sy_record_t *record_value = sy_execute_expression(value, strip);
    if (record_value == ERROR)
    {
        sy_error_runtime_by_node(value, "an error occurred during processing\n\tMajor:%s-%u", 
            __FILE__, __LINE__);
        return -1;
    }

    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        if (sy_execute_entity(scope, item, record_value, strip) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int32_t 
sy_execute_var(sy_node_t *scope, sy_node_t *node, strip_t *strip)
{
    sy_node_var_t *var1 = (sy_node_var_t *)node->value;
    if (var1->key->kind == NODE_KIND_SET)
	{
		if (sy_execute_set(scope, var1->key, var1->value, strip) < 0)
		{
			return -1;
		}
	}
    else
    {

    }
    return 0;
}

int32_t 
sy_execute_module(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_VAR)
        {
			if (sy_execute_var(node, item, NULL) < 0)
			{
				return -1;
			}
        }
    }

    return 0;
}

int32_t 
sy_execute_run(sy_node_t *root)
{
    if (sy_execute_module(root)  < 0)
    {
        return -1;
    }
    return 0;
}