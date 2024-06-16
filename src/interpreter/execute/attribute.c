#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>

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

not_record_t *
not_execute_attribute_from_type(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant)
{
    not_node_class_t *class1 = (not_node_class_t *)left->value;
    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            not_node_property_t *property = (not_node_property_t *)item->value;
            if (not_execute_id_cmp(property->key, right) == 1)
            {
                if ((property->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                not_entry_t *entry = not_symbol_table_find(left, property->key);
                if (entry == ERROR)
                {
                    return ERROR;
                }
                else if (entry != NULL)
                {
                    return entry->value;
                }

                not_record_t *record_value = NULL;
                if (property->value)
                {
                    record_value = not_execute_expression(property->value, strip, applicant, NULL);
                    if (record_value == ERROR)
                    {
                        return ERROR;
                    }

                    if (property->type)
                    {
                        not_record_t *record_type = not_execute_expression(property->type, strip, applicant, NULL);
                        if (record_type == ERROR)
                        {
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }

                        if (record_type->kind != RECORD_KIND_TYPE)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                            not_error_type_by_node(property->key, "'%s' unsupported type: '%s'",
                                                   basic1->value, not_record_type_as_string(record_type));

                            if (not_record_link_decrease(record_type) < 0)
                            {
                                return ERROR;
                            }
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return ERROR;
                            }

                            return ERROR;
                        }

                        int32_t r1 = not_execute_value_check_by_type(node, record_value, record_type, strip, applicant);
                        if (r1 < 0)
                        {
                            if (not_record_link_decrease(record_type) < 0)
                            {
                                return ERROR;
                            }
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }
                        else if (r1 == 0)
                        {
                            if ((property->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                                not_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'",
                                                       basic1->value, not_record_type_as_string(record_type), not_record_type_as_string(record_value));

                                if (not_record_link_decrease(record_type) < 0)
                                {
                                    return ERROR;
                                }
                                if (not_record_link_decrease(record_value) < 0)
                                {
                                    return ERROR;
                                }

                                return ERROR;
                            }
                            else
                            {
                                record_value = not_record_copy(record_value);

                                not_record_t *record_value2 = not_execute_value_casting_by_type(node, record_value, record_type, strip, applicant);
                                if (record_value2 == ERROR)
                                {
                                    if (not_record_link_decrease(record_type) < 0)
                                    {
                                        return ERROR;
                                    }
                                    if (not_record_link_decrease(record_value) < 0)
                                    {
                                        return ERROR;
                                    }

                                    return ERROR;
                                }
                                else if (record_value2 == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                                    not_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'",
                                                           basic1->value, not_record_type_as_string(record_type), not_record_type_as_string(record_value));

                                    if (not_record_link_decrease(record_type) < 0)
                                    {
                                        return ERROR;
                                    }
                                    if (not_record_link_decrease(record_value) < 0)
                                    {
                                        return ERROR;
                                    }

                                    return ERROR;
                                }

                                record_value = record_value2;
                            }
                        }

                        if (not_record_link_decrease(record_type) < 0)
                        {
                            return ERROR;
                        }
                    }
                }

                if ((property->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    record_value = not_record_copy(record_value);
                }

                entry = not_symbol_table_push(left, item, property->key, record_value);
                if (entry == ERROR)
                {
                    if ((property->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (not_record_link_decrease(record_value) < 0)
                        {
                            return ERROR;
                        }
                    }
                    return ERROR;
                }
                else if (entry == NULL)
                {
                    entry = not_symbol_table_find(left, property->key);
                }

                return entry->value;
            }
        }
        else if (item->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class2 = (not_node_class_t *)item->value;
            if (not_execute_id_cmp(class2->key, right) == 1)
            {
                if ((class2->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)class2->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_new = not_strip_create(copy_strip);
                if (strip_new == ERROR)
                {
                    if (not_strip_destroy(copy_strip) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                return not_record_make_type(item, strip_new);
            }
        }
        else if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_execute_id_cmp(fun1->key, right) == 1)
            {
                if ((fun1->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_new = not_strip_create(copy_strip);
                if (strip_new == ERROR)
                {
                    if (not_strip_destroy(copy_strip) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                return not_record_make_type(item, strip_new);
            }
        }
    }

    if (class1->heritages)
    {
        not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
        for (not_node_t *item = block->items; item != NULL; item = item->next)
        {
            not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;

            not_record_t *resp = not_execute_expression(heritage->type, strip, applicant, NULL);
            if (resp == ERROR)
            {
                return ERROR;
            }

            if (resp->kind != RECORD_KIND_TYPE)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(node, "'%s' unexpected type as heritage '%s'",
                                       basic2->value, basic1->value);

                if (not_record_link_decrease(resp) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }

            not_record_type_t *record_type = (not_record_type_t *)resp->value;
            not_node_t *type = record_type->type;

            if (type->kind == NODE_KIND_CLASS)
            {
                not_record_t *result = not_execute_attribute_from_type(
                    node, (not_strip_t *)record_type->value, type, right, applicant);

                if (not_record_link_decrease(resp) < 0)
                {
                    return ERROR;
                }

                if (result == ERROR)
                {
                    return ERROR;
                }
                else if (result != NULL)
                {
                    return result;
                }
            }
            else
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(node, "'%s' unexpected type as heritage '%s'",
                                       basic2->value, basic1->value);

                if (not_record_link_decrease(resp) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }
        }
    }

    return NULL;
}

not_record_t *
not_execute_attribute_from_struct(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant)
{
    not_node_class_t *class1 = (not_node_class_t *)left->value;
    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            not_node_property_t *property = (not_node_property_t *)item->value;
            if (not_execute_id_cmp(property->key, right) == 1)
            {
                if ((property->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                if ((property->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                not_entry_t *entry = not_strip_variable_find(strip, left, property->key);
                if (entry == ERROR)
                {
                    return ERROR;
                }
                else if (entry == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "in class '%s', property '%s' is not initialized",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                return entry->value;
            }
        }
        else if (item->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class2 = (not_node_class_t *)item->value;
            if (not_execute_id_cmp(class2->key, right) == 1)
            {
                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)class2->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_new = not_strip_create(copy_strip);
                if (strip_new == ERROR)
                {
                    if (not_strip_destroy(copy_strip) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                return not_record_make_type(item, strip_new);
            }
        }
        else if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_execute_id_cmp(fun1->key, right) == 1)
            {
                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_new = not_strip_create(copy_strip);
                if (strip_new == ERROR)
                {
                    if (not_strip_destroy(copy_strip) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                return not_record_make_type(item, strip_new);
            }
        }
    }

    if (class1->heritages)
    {
        not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
        for (not_node_t *item = block->items; item != NULL; item = item->next)
        {
            not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;

            not_entry_t *entry = not_strip_variable_find(strip, left, heritage->key);
            if (entry == ERROR)
            {
                return ERROR;
            }
            else if (entry == NULL)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(node, "in class '%s', heritage '%s' is not initialized",
                                       basic2->value, basic1->value);
                return ERROR;
            }

            not_record_struct_t *record_struct = (not_record_struct_t *)entry->value->value;
            not_node_t *type = record_struct->type;

            not_record_t *result = not_execute_attribute_from_struct(node, (not_strip_t *)record_struct->value, type, right, applicant);

            if (result == ERROR)
            {
                if (not_record_link_decrease(entry->value) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            else if (result != NULL)
            {
                if (not_record_link_decrease(entry->value) < 0)
                {
                    return ERROR;
                }
                return result;
            }
        }
    }

    return NULL;
}

not_record_t *
not_execute_attribute(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_binary_t *binary = (not_node_binary_t *)node->value;

    not_record_t *left = not_execute_expression(binary->left, strip, applicant, origin);
    if (left == ERROR)
    {
        return ERROR;
    }

    if (left->kind == RECORD_KIND_TYPE)
    {
        not_record_type_t *record_type = (not_record_type_t *)left->value;
        not_node_t *type = record_type->type;
        not_strip_t *strip_new = (not_strip_t *)record_type->value;
        if (type->kind == NODE_KIND_CLASS)
        {
            not_record_t *result = not_execute_attribute_from_type(node, strip_new, type, binary->right, applicant);

            if (result == ERROR)
            {
                return ERROR;
            }
            else if (result == NULL)
            {
                not_node_class_t *class1 = (not_node_class_t *)type->value;
                not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(binary->right, "'%s' object has no attribute '%s'",
                                       basic2->value, basic1->value);

                if (not_record_link_decrease(left) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }

            if (not_record_link_decrease(left) < 0)
            {
                return ERROR;
            }

            return result;
        }
        else
        {
            if (type->kind == NODE_KIND_FUN)
            {
                not_node_fun_t *fun1 = (not_node_fun_t *)type->value;
                not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)fun1->key->value;
                not_error_type_by_node(binary->right, "'%s' object has no attribute '%s'",
                                       basic2->value, basic1->value);
            }
            else
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
                not_error_type_by_node(binary->right, "type object '%s' has no attribute '%s'",
                                       not_node_kind_as_string(type), basic1->value);
            }

            if (not_record_link_decrease(left) < 0)
            {
                return ERROR;
            }

            return ERROR;
        }
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        not_record_struct_t *record_struct = (not_record_struct_t *)left->value;
        not_node_t *type = record_struct->type;
        not_strip_t *strip_new = (not_strip_t *)record_struct->value;

        not_record_t *result = not_execute_attribute_from_struct(node, strip_new, type, binary->right, applicant);

        if (result == ERROR)
        {
            return ERROR;
        }
        else if (result == NULL)
        {
            not_node_class_t *class1 = (not_node_class_t *)type->value;
            not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
            not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
            not_error_type_by_node(binary->right, "'%s' object has no attribute '%s'",
                                   basic2->value, basic1->value);

            if (not_record_link_decrease(left) < 0)
            {
                return ERROR;
            }

            return ERROR;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        return result;
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        for (not_record_object_t *item = (not_record_object_t *)left->value; item != NULL; item = item->next)
        {
            if (not_execute_id_strcmp(binary->right, item->key) == 1)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return ERROR;
                }

                return item->value;
            }
        }

        not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
        not_error_type_by_node(node, "'%s' has no attribute '%s'",
                               not_record_type_as_string(left), basic1->value);

        if (not_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        return ERROR;
    }
    else
    {
        not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
        not_error_type_by_node(binary->right, "'%s' has no attribute '%s'",
                               not_record_type_as_string(left), basic1->value);

        if (not_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        return ERROR;
    }
}