#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../../parser/syntax/syntax.h"
#include "../../error.h"
#include "../../mutex.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "../entry.h"
#include "execute.h"

static sy_record_t *
attribute_from_type(sy_node_t *node, sy_strip_t *strip, sy_node_t *left, sy_node_t *right, sy_node_t *applicant)
{
    sy_node_class_t *class1 = (sy_node_class_t *)left->value;
    for (sy_node_t *item = class1->block;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            sy_node_property_t *property = (sy_node_property_t *)item->value;
            if (sy_execute_id_cmp(property->key, right) == 1)
            {
                if ((property->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                sy_entry_t *entry = sy_symbol_table_find(left, property->key);
                if (entry == ERROR)
                {
                    return ERROR;
                }
                else
                if (entry != NULL)
                {
                    return entry->value;
                }

                sy_record_t *record_value = NULL;
                if (property->value)
                {
                    record_value = sy_execute_expression(property->value, strip, applicant, NULL);
                    if (record_value == ERROR)
                    {
                        return ERROR;
                    }

                    if (property->type)
                    {
                        sy_record_t *record_type = sy_execute_expression(property->type, strip, applicant, NULL);
                        if (record_type == ERROR)
                        {
                            if (record_value && (record_value != NAN))
                            {
                                if (record_value->link == 0)
                                {
                                    if (sy_record_destroy(record_value) < 0)
                                    {
                                        return ERROR;
                                    }
                                }
                            }
                            return ERROR;
                        }

                        if (record_type->kind != RECORD_KIND_TYPE)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                            sy_error_type_by_node(property->key, "'%s' unsupported type: '%s'", 
                                basic1->value, sy_record_type_as_string(record_type));
                            
                            if (record_type->link == 0)
                            {
                                if (sy_record_destroy(record_type) < 0)
                                {
                                    return ERROR;
                                }
                            }

                            if (record_value->link == 0)
                            {
                                if (sy_record_destroy(record_value) < 0)
                                {
                                    return ERROR;
                                }
                            }

                            return ERROR;
                        }

                        int32_t r1 = sy_execute_value_check_by_type(record_value, record_type, strip, applicant);
                        if (r1 < 0)
                        {
                            if (record_type->link == 0)
                            {
                                if (sy_record_destroy(record_type) < 0)
                                {
                                    return ERROR;
                                }
                            }

                            if (record_value->link == 0)
                            {
                                if (sy_record_destroy(record_value) < 0)
                                {
                                    return ERROR;
                                }
                            }
                        }
                        else
                        if (r1 == 0)
                        {
                            if ((property->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                                sy_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'", 
                                    basic1->value, sy_record_type_as_string(record_type), sy_record_type_as_string(record_value));

                                if (record_type->link == 0)
                                {
                                    if (sy_record_destroy(record_type) < 0)
                                    {
                                        return ERROR;
                                    }
                                }

                                if (record_value->link == 0)
                                {
                                    if (sy_record_destroy(record_value) < 0)
                                    {
                                        return ERROR;
                                    }
                                }

                                return ERROR;
                            }
                            else
                            {
                                if (record_value && (record_value != NAN))
                                {
                                    if (record_value->link == 1)
                                    {
                                        record_value = sy_record_copy(record_value);
                                    }
                                }

                                sy_record_t *record_value2 = sy_execute_value_casting_by_type(record_value, record_type, strip, applicant);
                                if (record_value2 == ERROR)
                                {
                                    if (record_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_type) < 0)
                                        {
                                            return ERROR;
                                        }
                                    }

                                    if (record_value->link == 0)
                                    {
                                        if (sy_record_destroy(record_value) < 0)
                                        {
                                            return ERROR;
                                        }
                                    }
                                    return ERROR;
                                }
                                else
                                if (record_value2 == NULL)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                                    sy_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'", 
                                        basic1->value, sy_record_type_as_string(record_type), sy_record_type_as_string(record_value));

                                    if (record_type->link == 0)
                                    {
                                        if (sy_record_destroy(record_type) < 0)
                                        {
                                            return ERROR;
                                        }
                                    }

                                    if (record_value->link == 0)
                                    {
                                        if (sy_record_destroy(record_value) < 0)
                                        {
                                            return ERROR;
                                        }
                                    }
                                    return ERROR;
                                }

                                record_value = record_value2;
                            }
                        }

                        if (record_type && (record_type != NAN))
                        {
                            if (record_type->link == 0)
                            {
                                if (sy_record_destroy(record_type) < 0)
                                {
                                    return ERROR;
                                }
                            }
                        }
                    }
                }

                if ((property->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (record_value->link == 1)
                    {
                        record_value = sy_record_copy(record_value);
                    }

                    record_value->link = 1;
                }

                entry = sy_symbol_table_push(left, item, property->key, record_value);
                if (entry == ERROR)
                {
                    if ((property->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        record_value->link = 0;

                        if (sy_record_destroy(record_value) < 0)
                        {
                            return ERROR;
                        }
                    }
                    return ERROR;
                }
                else
                if (entry == NULL)
                {
                    entry = sy_symbol_table_find(left, property->key);
                }

                return entry->value;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class2 = (sy_node_class_t *)item->value;
            if (sy_execute_id_cmp(class2->key, right) == 1)
            {
                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)class2->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                sy_strip_t *new_strip = sy_strip_copy(strip);
                if (new_strip)
                {
                    return ERROR;
                }

                return sy_record_make_type(item, new_strip); 
            }
        }
        else
        if (item->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
            if (sy_execute_id_cmp(fun1->key, right) == 1)
            {
                if ((fun1->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                sy_strip_t *new_strip = sy_strip_copy(strip);
                if (new_strip)
                {
                    return ERROR;
                }

                return sy_record_make_type(item, new_strip);
            }
        }
    }
    
    if (class1->heritages)
    {
        sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
        for (sy_node_t *item = block->items;item != NULL;item = item->next)
        {
            sy_node_heritage_t *heritage = (sy_node_heritage_t *)item->value;
            
            sy_record_t *resp = sy_execute_expression(heritage->type, strip, applicant, NULL);
            if (resp == ERROR)
            {
                return ERROR;
            }

            if (resp->kind != RECORD_KIND_TYPE)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
                sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                sy_error_type_by_node(node, "'%s' unexpected type as heritage '%s'", 
                    basic2->value, basic1->value);

                if (resp->link == 0)
                {
                    if (sy_record_destroy(resp) < 0)
                    {
                        return ERROR;
                    }
                }
                return ERROR;
            }

            sy_record_type_t *record_type = (sy_record_type_t *)resp->value;
            sy_node_t *type = record_type->type;

            if (type->kind == NODE_KIND_CLASS)
            {
                sy_record_t *result = attribute_from_type(
                    node, (sy_strip_t *)record_type->value, type, right, applicant);

                if (resp->link == 0)
                {
                    if (sy_record_destroy(resp) < 0)
                    {
                        return ERROR;
                    }
                }

                if (result == ERROR)
                {
                    return ERROR;
                }
                else
                if (result != NULL)
                {
                    return result;
                }
            }
            else
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
                sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                sy_error_type_by_node(node, "'%s' unexpected type as heritage '%s'", 
                    basic2->value, basic1->value);

                if (resp->link == 0)
                {
                    if (sy_record_destroy(resp) < 0)
                    {
                        return ERROR;
                    }
                }
                return ERROR;
            }
        }
    }

    return NULL;
}

static sy_record_t *
attribute_from_struct(sy_node_t *node, sy_strip_t *strip, sy_node_t *left, sy_node_t *right, sy_node_t *applicant)
{
    sy_node_class_t *class1 = (sy_node_class_t *)left->value;
    for (sy_node_t *item = class1->block;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            sy_node_property_t *property = (sy_node_property_t *)item->value;
            if (sy_execute_id_cmp(property->key, right) == 1)
            {
                if ((property->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                if ((property->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                sy_entry_t *entry = sy_strip_variable_find(strip, left, property->key);
                if (entry == ERROR)
                {
                    return ERROR;
                }
                else
                if (entry == NULL)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)property->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "in class '%s', property '%s' is not initialized", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                return entry->value;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class2 = (sy_node_class_t *)item->value;
            if (sy_execute_id_cmp(class2->key, right) == 1)
            {
                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)class2->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                sy_strip_t *new_strip = sy_strip_create(strip);
                if (new_strip == ERROR)
                {
                    return ERROR;
                }

                return sy_record_make_type(item, new_strip); 
            }
        }
        else
        if (item->kind == NODE_KIND_IF)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
            if (sy_execute_id_cmp(fun1->key, right) == 1)
            {
                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                    sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                    sy_error_type_by_node(node, "'%s' unexpected access to '%s'", 
                        basic2->value, basic1->value);
                    return ERROR;
                }

                sy_strip_t *new_strip = sy_strip_create(strip);
                if (new_strip == ERROR)
                {
                    return ERROR;
                }

                return sy_record_make_type(item, new_strip);
            }
        }
    }

    if (class1->heritages)
    {
        sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
        for (sy_node_t *item = block->items;item != NULL;item = item->next)
        {
            sy_node_heritage_t *heritage = (sy_node_heritage_t *)item->value;

            sy_entry_t *entry = sy_strip_variable_find(strip, left, heritage->key);
            if (entry == ERROR)
            {
                return ERROR;
            }
            else
            if (entry == NULL)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage->key->value;
                sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                sy_error_type_by_node(node, "in class '%s', heritage '%s' is not initialized", 
                    basic2->value, basic1->value);
                return ERROR;
            }

            sy_record_struct_t *record_struct = (sy_record_struct_t *)entry->value->value;
            sy_node_t *type = record_struct->type;

            sy_record_t *result = attribute_from_struct(
                node, (sy_strip_t *)record_struct->value, type, right, applicant);

            if (result == ERROR)
            {
                return ERROR;
            }
            else
            if (result != NULL)
            {
                return result;
            }
        }
    }

    return NULL;
}

sy_record_t *
sy_execute_attribute(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

    sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
    if (left == ERROR)
    {
        return ERROR;
    }

    if (left->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *record_type = (sy_record_type_t *)left->value;
        sy_node_t *type = record_type->type;
        if (type->kind == NODE_KIND_CLASS)
        {
            
            sy_record_t *result = attribute_from_type(
                node, (sy_strip_t *)record_type->value, type, binary->right, applicant);
            
            if (result == ERROR)
            {
                return ERROR;
            }
            else
            if (result == NULL)
            {
                sy_node_class_t *class1 = (sy_node_class_t *)type->value;
                sy_node_basic_t *basic1 = (sy_node_basic_t *)binary->right->value;
                sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
                sy_error_type_by_node(binary->right, "'%s' object has no attribute '%s'", 
                    basic2->value, basic1->value);

                if (left->link == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                return ERROR;
            }

            if (left->link == 0)
            {
                if (sy_record_destroy(left) < 0)
                {
                    return ERROR;
                }
            }

            return result;
        }
        else
        {
            if (type->kind == NODE_KIND_FUN)
            {
                sy_node_fun_t *fun1 = (sy_node_fun_t *)type->value;
                sy_node_basic_t *basic1 = (sy_node_basic_t *)binary->right->value;
                sy_node_basic_t *basic2 = (sy_node_basic_t *)fun1->key->value;
                sy_error_type_by_node(binary->right, "'%s' object has no attribute '%s'", 
                    basic2->value, basic1->value);
            }
            else
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)binary->right->value;
                sy_error_type_by_node(binary->right, "type object '%s' has no attribute '%s'", 
                    sy_node_kind_as_string(type), basic1->value);
            }

            if (left->link == 0)
            {
                if (sy_record_destroy(left) < 0)
                {
                    return ERROR;
                }
            }

            return ERROR;
        }
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *record_struct = (sy_record_struct_t *)left->value;
        sy_node_t *type = record_struct->type;

        sy_record_t *result = attribute_from_struct(
            node, (sy_strip_t *)record_struct->value, type, binary->right, applicant);
            
        if (result == ERROR)
        {
            return ERROR;
        }
        else
        if (result == NULL)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)type->value;
            sy_node_basic_t *basic1 = (sy_node_basic_t *)binary->right->value;
            sy_node_basic_t *basic2 = (sy_node_basic_t *)class1->key->value;
            sy_error_type_by_node(binary->right, "'%s' object has no attribute '%s'", 
                basic2->value, basic1->value);

            if (left->link == 0)
            {
                if (sy_record_destroy(left) < 0)
                {
                    return ERROR;
                }
            }

            return ERROR;
        }

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        return result;
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        for (sy_record_object_t *item = (sy_record_object_t *)left->value;item != NULL;item = item->next)
        {
            if (sy_execute_id_cmp(item->key, binary->right) == 1)
            {
                if (left->link == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }

                return item->value;
            }
        }

        sy_node_basic_t *basic1 = (sy_node_basic_t *)binary->right->value;
        sy_error_type_by_node(binary->right, "'%s' has no attribute '%s'", 
            sy_record_type_as_string(left), basic1->value);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        return ERROR;
    }
    else
    {
        sy_node_basic_t *basic1 = (sy_node_basic_t *)binary->right->value;
        sy_error_type_by_node(binary->right, "'%s' has no attribute '%s'", 
            sy_record_type_as_string(left), basic1->value);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        return ERROR;
    }
    
}