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
#include "../../memory.h"
#include "../record.h"

#include "../symbol_table.h"
#include "../strip.h"
#include "../entry.h"
#include "../helper.h"
#include "execute.h"

not_record_t *
not_attribute_from_type(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant)
{
    not_node_class_t *class1 = (not_node_class_t *)left->value;
    for (not_node_t *item = class1->block; item != NOT_PTR_NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            not_node_property_t *property = (not_node_property_t *)item->value;
            if (not_helper_id_cmp(property->key, right) == 0)
            {
                if ((property->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                not_entry_t *entry = not_symbol_table_find(left, property->key);
                if (entry == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }
                else if (entry != NOT_PTR_NULL)
                {
                    return entry->value;
                }

                not_record_t *record_value = NOT_PTR_NULL;
                if (property->value)
                {
                    record_value = not_expression(property->value, strip, applicant, NOT_PTR_NULL);
                    if (record_value == NOT_PTR_ERROR)
                    {
                        return NOT_PTR_ERROR;
                    }

                    if (property->type)
                    {
                        not_record_t *record_type = not_expression(property->type, strip, applicant, NOT_PTR_NULL);
                        if (record_type == NOT_PTR_ERROR)
                        {
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return NOT_PTR_ERROR;
                            }
                            return NOT_PTR_ERROR;
                        }

                        if (record_type->kind != RECORD_KIND_TYPE)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                            not_error_type_by_node(property->key, "'%s' unsupported type: '%s'",
                                                   basic1->value, not_record_type_as_string(record_type));

                            if (not_record_link_decrease(record_type) < 0)
                            {
                                return NOT_PTR_ERROR;
                            }
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return NOT_PTR_ERROR;
                            }

                            return NOT_PTR_ERROR;
                        }

                        int32_t r1 = not_execute_value_check_by_type(node, record_value, record_type, strip, applicant);
                        if (r1 < 0)
                        {
                            if (not_record_link_decrease(record_type) < 0)
                            {
                                return NOT_PTR_ERROR;
                            }
                            if (not_record_link_decrease(record_value) < 0)
                            {
                                return NOT_PTR_ERROR;
                            }
                            return NOT_PTR_ERROR;
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
                                    return NOT_PTR_ERROR;
                                }
                                if (not_record_link_decrease(record_value) < 0)
                                {
                                    return NOT_PTR_ERROR;
                                }

                                return NOT_PTR_ERROR;
                            }
                            else
                            {
                                record_value = not_record_copy(record_value);

                                not_record_t *record_value2 = not_execute_value_casting_by_type(node, record_value, record_type, strip, applicant);
                                if (record_value2 == NOT_PTR_ERROR)
                                {
                                    if (not_record_link_decrease(record_type) < 0)
                                    {
                                        return NOT_PTR_ERROR;
                                    }
                                    if (not_record_link_decrease(record_value) < 0)
                                    {
                                        return NOT_PTR_ERROR;
                                    }

                                    return NOT_PTR_ERROR;
                                }
                                else if (record_value2 == NOT_PTR_NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                                    not_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'",
                                                           basic1->value, not_record_type_as_string(record_type), not_record_type_as_string(record_value));

                                    if (not_record_link_decrease(record_type) < 0)
                                    {
                                        return NOT_PTR_ERROR;
                                    }
                                    if (not_record_link_decrease(record_value) < 0)
                                    {
                                        return NOT_PTR_ERROR;
                                    }

                                    return NOT_PTR_ERROR;
                                }

                                record_value = record_value2;
                            }
                        }

                        if (not_record_link_decrease(record_type) < 0)
                        {
                            return NOT_PTR_ERROR;
                        }
                    }
                }

                if ((property->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    record_value = not_record_copy(record_value);
                }

                entry = not_symbol_table_push(left, item, property->key, record_value);
                if (entry == NOT_PTR_ERROR)
                {
                    if ((property->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (not_record_link_decrease(record_value) < 0)
                        {
                            return NOT_PTR_ERROR;
                        }
                    }
                    return NOT_PTR_ERROR;
                }
                else if (entry == NOT_PTR_NULL)
                {
                    entry = not_symbol_table_find(left, property->key);
                }

                return entry->value;
            }
        }
        else if (item->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class2 = (not_node_class_t *)item->value;
            if (not_helper_id_cmp(class2->key, right) == 0)
            {
                if ((class2->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)class2->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }

                return not_record_make_type(item, copy_strip);
            }
        }
        else if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_helper_id_cmp(fun1->key, right) == 0)
            {
                if ((fun1->flag & (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT)) != (SYNTAX_MODIFIER_STATIC | SYNTAX_MODIFIER_EXPORT))
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }

                return not_record_make_type(item, copy_strip);
            }
        }
    }

    if (class1->heritages)
    {
        not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;

            not_record_t *resp = not_expression(heritage->type, strip, applicant, NOT_PTR_NULL);
            if (resp == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }

            if (resp->kind != RECORD_KIND_TYPE)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(node, "'%s' unexpected type as heritage '%s'",
                                       basic2->value, basic1->value);

                if (not_record_link_decrease(resp) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }

            not_record_type_t *record_type = (not_record_type_t *)resp->value;
            not_node_t *type = record_type->type;

            if (type->kind == NODE_KIND_CLASS)
            {
                not_record_t *result = not_attribute_from_type(
                    node, (not_strip_t *)record_type->value, type, right, applicant);

                if (not_record_link_decrease(resp) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                if (result == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }
                else if (result != NOT_PTR_NULL)
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
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
        }
    }

    return NOT_PTR_NULL;
}

not_record_t *
not_attribute_from_struct(not_node_t *node, not_strip_t *strip, not_node_t *left, not_node_t *right, not_node_t *applicant)
{
    not_node_class_t *class1 = (not_node_class_t *)left->value;
    for (not_node_t *item = class1->block; item != NOT_PTR_NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            not_node_property_t *property = (not_node_property_t *)item->value;
            if (not_helper_id_cmp(property->key, right) == 0)
            {
                if ((property->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                if ((property->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                not_entry_t *entry = not_strip_variable_find(strip, left, property->key);
                if (entry == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }
                else if (entry == NOT_PTR_NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "in class '%s', property '%s' is not initialized",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                return entry->value;
            }
        }
        else if (item->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class2 = (not_node_class_t *)item->value;
            if (not_helper_id_cmp(class2->key, right) == 0)
            {
                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)class2->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }

                return not_record_make_type(item, copy_strip);
            }
        }
        else if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_helper_id_cmp(fun1->key, right) == 0)
            {
                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                    not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                    not_error_type_by_node(node, "'%s' unexpected access to '%s'",
                                           basic2->value, basic1->value);
                    return NOT_PTR_ERROR;
                }

                not_strip_t *copy_strip = not_strip_copy(strip);
                if (copy_strip == NOT_PTR_ERROR)
                {
                    return NOT_PTR_ERROR;
                }

                return not_record_make_type(item, copy_strip);
            }
        }
    }

    if (class1->heritages)
    {
        not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;

            not_entry_t *entry = not_strip_variable_find(strip, left, heritage->key);
            if (entry == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
            else if (entry == NOT_PTR_NULL)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(node, "in class '%s', heritage '%s' is not initialized",
                                       basic2->value, basic1->value);
                return NOT_PTR_ERROR;
            }

            not_record_struct_t *record_struct = (not_record_struct_t *)entry->value->value;
            not_node_t *type = record_struct->type;

            not_record_t *result = not_attribute_from_struct(node, (not_strip_t *)record_struct->value, type, right, applicant);

            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(entry->value) < 0)
                {
                    return NOT_PTR_ERROR;
                }
                return NOT_PTR_ERROR;
            }
            else if (result != NOT_PTR_NULL)
            {
                if (not_record_link_decrease(entry->value) < 0)
                {
                    return NOT_PTR_ERROR;
                }
                return result;
            }
        }
    }

    return NOT_PTR_NULL;
}

not_record_t *
not_attribute_object_builtin_remove(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 1)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Remove", 1, cnt1);
            return NOT_PTR_ERROR;
        }
    }
    else
    {
        not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Remove", 1, 0);
        return NOT_PTR_ERROR;
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    not_node_block_t *block = (not_node_block_t *)arguments->value;
    not_record_t *record_arg[1];

    size_t array_length = sizeof(record_arg) / sizeof(record_arg[0]);
    for (size_t i = 0; i < array_length; i++)
    {
        record_arg[i] = NOT_PTR_NULL;
    }

    size_t parameter_index = 0;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_argument_t *argument = (not_node_argument_t *)item->value;
        if (argument->value)
        {
            if (not_helper_id_strcmp(argument->key, "key") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                if (arg->kind != RECORD_KIND_STRING)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)argument->key->value;
                    not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                           basic->value, not_record_type_as_string(arg), "string");
                }

                record_arg[0] = arg;
            }
        }
        else
        {
            not_record_t *arg = not_expression(argument->key, strip, applicant, NOT_PTR_NULL);
            if (arg == NOT_PTR_ERROR)
            {
                goto region_cleanup;
            }

            if (parameter_index == 0)
            {
                if (arg->kind != RECORD_KIND_STRING)
                {
                    not_error_type_by_node(argument->key, "mismatch: '%s' and '%s'",
                                           not_record_type_as_string(arg), "string");
                }
            }

            record_arg[parameter_index] = arg;

            parameter_index += 1;
        }
    }

    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i] == NOT_PTR_NULL)
        {
            if (i == 0)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Remove", "key");
            }
            goto region_cleanup;
        }
    }

    char *key = (char *)record_arg[0]->value;
    for (not_record_object_t *object = (not_record_object_t *)source->value, *previous = NOT_PTR_NULL; object != NOT_PTR_NULL; object = object->next)
    {
        if (strcmp(object->key, key) == 0)
        {
            if (object->next)
            {
                if (previous)
                {
                    previous->next = object->next;
                }
                else
                {
                    source->value = object->next;
                }
            }
            else
            {
                if (previous)
                {
                    previous->next = NOT_PTR_NULL;
                }
                else
                {
                    source->value = NOT_PTR_NULL;
                }
            }

            not_memory_free(object->key);
            if (not_record_link_decrease(object->value) < 0)
            {
                goto region_cleanup;
            }
            not_memory_free(object);
            break;
        }
        previous = object;
    }

    not_record_link_increase(source);
    return_value = source;

region_cleanup:
    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i])
        {
            if (not_record_link_decrease(record_arg[i]) < 0)
            {
                if (return_value != NOT_PTR_ERROR)
                {
                    not_record_link_decrease(return_value);
                }
                return NOT_PTR_ERROR;
            }
        }
    }

    return return_value;
}

not_record_t *
not_attribute_object_builtin_add(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 2)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Add", 2, cnt1);
            return NOT_PTR_ERROR;
        }
    }
    else
    {
        not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Add", 2, 0);
        return NOT_PTR_ERROR;
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    not_node_block_t *block = (not_node_block_t *)arguments->value;
    not_record_t *record_arg[2];

    size_t array_length = sizeof(record_arg) / sizeof(record_arg[0]);
    for (size_t i = 0; i < array_length; i++)
    {
        record_arg[i] = NOT_PTR_NULL;
    }

    size_t parameter_index = 0;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_argument_t *argument = (not_node_argument_t *)item->value;
        if (argument->value)
        {
            if (not_helper_id_strcmp(argument->key, "key") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                if (arg->kind != RECORD_KIND_STRING)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)argument->key->value;
                    not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                           basic->value, not_record_type_as_string(arg), "string");
                }

                record_arg[0] = arg;
            }
            else if (not_helper_id_strcmp(argument->key, "value") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                record_arg[1] = arg;
            }
        }
        else
        {
            not_record_t *arg = not_expression(argument->key, strip, applicant, NOT_PTR_NULL);
            if (arg == NOT_PTR_ERROR)
            {
                goto region_cleanup;
            }

            if (parameter_index == 0)
            {
                if (arg->kind != RECORD_KIND_STRING)
                {
                    not_error_type_by_node(argument->key, "mismatch: '%s' and '%s'",
                                           not_record_type_as_string(arg), "string");
                }
            }

            record_arg[parameter_index] = arg;

            parameter_index += 1;
        }
    }

    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i] == NOT_PTR_NULL)
        {
            if (i == 0)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Add", "key");
            }
            else if (i == 1)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Add", "value");
            }
            goto region_cleanup;
        }
    }

    int found = 0;
    char *key = (char *)record_arg[0]->value;
    not_record_object_t *last_object = NOT_PTR_NULL;
    for (not_record_object_t *object = (not_record_object_t *)source->value; object != NOT_PTR_NULL; object = object->next)
    {
        last_object = object;
        if (strcmp(object->key, key) == 0)
        {
            if (not_record_link_decrease(object->value) < 0)
            {
                goto region_cleanup;
            }
            not_record_link_increase(record_arg[1]);
            object->value = record_arg[1];
            found = 1;
            break;
        }
    }

    if (found == 0)
    {
        not_record_object_t *new_object = not_record_make_object(key, record_arg[1], NOT_PTR_NULL);
        if (new_object == NOT_PTR_ERROR)
        {
            goto region_cleanup;
        }
        not_record_link_increase(record_arg[1]);

        if (last_object)
        {
            last_object->next = new_object;
        }
        else
        {
            source->value = new_object;
        }
    }

    not_record_link_increase(source);
    return_value = source;

region_cleanup:
    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i])
        {
            if (not_record_link_decrease(record_arg[i]) < 0)
            {
                if (return_value != NOT_PTR_ERROR)
                {
                    not_record_link_decrease(return_value);
                }
                return NOT_PTR_ERROR;
            }
        }
    }

    return return_value;
}

not_record_t *
not_attribute_tuple_builtin_remove(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 1)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Remove", 1, cnt1);
            return NOT_PTR_ERROR;
        }
    }
    else
    {
        not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Remove", 1, 0);
        return NOT_PTR_ERROR;
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    not_node_block_t *block = (not_node_block_t *)arguments->value;
    not_record_t *record_arg[1];

    size_t array_length = sizeof(record_arg) / sizeof(record_arg[0]);
    for (size_t i = 0; i < array_length; i++)
    {
        record_arg[i] = NOT_PTR_NULL;
    }

    size_t parameter_index = 0;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_argument_t *argument = (not_node_argument_t *)item->value;
        if (argument->value)
        {
            if (not_helper_id_strcmp(argument->key, "index") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                if (arg->kind != RECORD_KIND_INT)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)argument->key->value;
                    not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                           basic->value, not_record_type_as_string(arg), "int");
                }

                record_arg[0] = arg;
            }
        }
        else
        {
            not_record_t *arg = not_expression(argument->key, strip, applicant, NOT_PTR_NULL);
            if (arg == NOT_PTR_ERROR)
            {
                goto region_cleanup;
            }

            if (parameter_index == 0)
            {
                if (arg->kind != RECORD_KIND_INT)
                {
                    not_error_type_by_node(argument->key, "mismatch: '%s' and '%s'",
                                           not_record_type_as_string(arg), "int");
                }
            }

            record_arg[parameter_index] = arg;

            parameter_index += 1;
        }
    }

    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i] == NOT_PTR_NULL)
        {
            if (i == 0)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Remove", "index");
            }
            goto region_cleanup;
        }
    }

    mpz_t length;
    mpz_init_set_si(length, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        mpz_add_ui(length, length, 1);
    }

    mpz_t term;
    mpz_init_set(term, *(mpz_t *)record_arg[0]->value);

    while (mpz_cmp_si(term, 0) < 0)
    {
        mpz_add(term, length, term);
    }

    mpz_clear(length);

    mpz_t index;
    mpz_init_set_si(index, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value, *previous = NOT_PTR_NULL; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        if (mpz_cmp(index, term) == 0)
        {
            if (tuple->next)
            {
                if (previous)
                {
                    previous->next = tuple->next;
                }
                else
                {
                    source->value = tuple->next;
                }
            }
            else
            {
                if (previous)
                {
                    previous->next = NOT_PTR_NULL;
                }
                else
                {
                    source->value = NOT_PTR_NULL;
                }
            }

            if (not_record_link_decrease(tuple->value) < 0)
            {
                mpz_clear(term);
                mpz_clear(index);
                goto region_cleanup;
            }
            not_memory_free(tuple);
            break;
        }
        previous = tuple;
        mpz_add_ui(index, index, 1);
    }

    mpz_clear(term);
    mpz_clear(index);

    not_record_link_increase(source);
    return_value = source;

region_cleanup:
    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i])
        {
            if (not_record_link_decrease(record_arg[i]) < 0)
            {
                if (return_value != NOT_PTR_ERROR)
                {
                    not_record_link_decrease(return_value);
                }
                return NOT_PTR_ERROR;
            }
        }
    }

    return return_value;
}

not_record_t *
not_attribute_tuple_builtin_set(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 2)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Set", 2, cnt1);
            return NOT_PTR_ERROR;
        }
    }
    else
    {
        not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Set", 2, 0);
        return NOT_PTR_ERROR;
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    not_node_block_t *block = (not_node_block_t *)arguments->value;
    not_record_t *record_arg[2];

    size_t array_length = sizeof(record_arg) / sizeof(record_arg[0]);
    for (size_t i = 0; i < array_length; i++)
    {
        record_arg[i] = NOT_PTR_NULL;
    }

    size_t parameter_index = 0;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_argument_t *argument = (not_node_argument_t *)item->value;
        if (argument->value)
        {
            if (not_helper_id_strcmp(argument->key, "index") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                if (arg->kind != RECORD_KIND_INT)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)argument->key->value;
                    not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                           basic->value, not_record_type_as_string(arg), "int");
                }

                record_arg[0] = arg;
            }
            else if (not_helper_id_strcmp(argument->key, "value") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                record_arg[1] = arg;
            }
        }
        else
        {
            not_record_t *arg = not_expression(argument->key, strip, applicant, NOT_PTR_NULL);
            if (arg == NOT_PTR_ERROR)
            {
                goto region_cleanup;
            }

            if (parameter_index == 0)
            {
                if (arg->kind != RECORD_KIND_INT)
                {
                    not_error_type_by_node(argument->key, "mismatch: '%s' and '%s'",
                                           not_record_type_as_string(arg), "int");
                }
            }

            record_arg[parameter_index] = arg;

            parameter_index += 1;
        }
    }

    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i] == NOT_PTR_NULL)
        {
            if (i == 0)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Set", "index");
            }
            else if (i == 1)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Set", "value");
            }
            goto region_cleanup;
        }
    }

    mpz_t length;
    mpz_init_set_si(length, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        mpz_add_ui(length, length, 1);
    }

    mpz_t term;
    mpz_init_set(term, *(mpz_t *)record_arg[0]->value);

    while (mpz_cmp_si(term, 0) < 0)
    {
        mpz_add(term, length, term);
    }

    mpz_clear(length);

    mpz_t index;
    mpz_init_set_si(index, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        if (mpz_cmp(index, term) < 0)
        {
            if (tuple->next == NOT_PTR_NULL)
            {
                not_record_t *arg = not_record_make_undefined();
                if (arg == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    goto region_cleanup;
                }

                not_record_tuple_t *new_tuple = not_record_make_tuple(arg, NOT_PTR_NULL);
                if (new_tuple == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    not_record_link_decrease(arg);
                    goto region_cleanup;
                }

                tuple->next = new_tuple;
            }
            mpz_add_ui(index, index, 1);
            continue;
        }
        else if (mpz_cmp(index, term) == 0)
        {
            if (not_record_link_decrease(tuple->value) < 0)
            {
                mpz_clear(term);
                mpz_clear(index);
                goto region_cleanup;
            }

            not_record_link_increase(record_arg[1]);
            tuple->value = record_arg[1];
            break;
        }
    }

    mpz_clear(term);
    mpz_clear(index);

    not_record_link_increase(source);
    return_value = source;

region_cleanup:
    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i])
        {
            if (not_record_link_decrease(record_arg[i]) < 0)
            {
                if (return_value != NOT_PTR_ERROR)
                {
                    not_record_link_decrease(return_value);
                }
                return NOT_PTR_ERROR;
            }
        }
    }

    return return_value;
}

not_record_t *
not_attribute_tuple_builtin_insert(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 2)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Insert", 2, cnt1);
            return NOT_PTR_ERROR;
        }
    }
    else
    {
        not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Insert", 2, 0);
        return NOT_PTR_ERROR;
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    not_node_block_t *block = (not_node_block_t *)arguments->value;
    not_record_t *record_arg[2];

    size_t array_length = sizeof(record_arg) / sizeof(record_arg[0]);
    for (size_t i = 0; i < array_length; i++)
    {
        record_arg[i] = NOT_PTR_NULL;
    }

    size_t parameter_index = 0;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_argument_t *argument = (not_node_argument_t *)item->value;
        if (argument->value)
        {
            if (not_helper_id_strcmp(argument->key, "index") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                if (arg->kind != RECORD_KIND_INT)
                {
                    not_node_basic_t *basic = (not_node_basic_t *)argument->key->value;
                    not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                           basic->value, not_record_type_as_string(arg), "int");
                }

                record_arg[0] = arg;
            }
            else if (not_helper_id_strcmp(argument->key, "value") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                record_arg[1] = arg;
            }
        }
        else
        {
            not_record_t *arg = not_expression(argument->key, strip, applicant, NOT_PTR_NULL);
            if (arg == NOT_PTR_ERROR)
            {
                goto region_cleanup;
            }

            if (parameter_index == 0)
            {
                if (arg->kind != RECORD_KIND_INT)
                {
                    not_error_type_by_node(argument->key, "mismatch: '%s' and '%s'",
                                           not_record_type_as_string(arg), "int");
                }
            }

            record_arg[parameter_index] = arg;

            parameter_index += 1;
        }
    }

    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i] == NOT_PTR_NULL)
        {
            if (i == 0)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Insert", "index");
            }
            else if (i == 1)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Insert", "value");
            }
            goto region_cleanup;
        }
    }

    mpz_t length;
    mpz_init_set_si(length, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        mpz_add_ui(length, length, 1);
    }

    mpz_t term;
    mpz_init_set(term, *(mpz_t *)record_arg[0]->value);

    while (mpz_cmp_si(term, 0) < 0)
    {
        mpz_add(term, length, term);
    }

    mpz_clear(length);

    int appended = 0;
    mpz_t index;
    mpz_init_set_si(index, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value, *previous = NOT_PTR_NULL; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        if (mpz_cmp(index, term) < 0)
        {
            if (tuple->next == NOT_PTR_NULL)
            {
                not_record_t *arg = not_record_make_undefined();
                if (arg == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    goto region_cleanup;
                }

                not_record_tuple_t *new_tuple = not_record_make_tuple(arg, NOT_PTR_NULL);
                if (new_tuple == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    not_record_link_decrease(arg);
                    goto region_cleanup;
                }

                appended = 1;
                tuple->next = new_tuple;
            }
            mpz_add_ui(index, index, 1);

            previous = tuple;
            continue;
        }
        else if (mpz_cmp(index, term) == 0)
        {
            if (appended == 1)
            {
                if (not_record_link_decrease(tuple->value) < 0)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    goto region_cleanup;
                }

                not_record_link_increase(record_arg[1]);
                tuple->value = record_arg[1];
            }
            else
            {
                not_record_link_increase(record_arg[1]);
                not_record_tuple_t *new_tuple = not_record_make_tuple(record_arg[1], NOT_PTR_NULL);
                if (new_tuple == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    not_record_link_decrease(record_arg[1]);
                    goto region_cleanup;
                }

                if (previous)
                {
                    new_tuple->next = tuple;
                    previous->next = new_tuple;
                }
                else
                {
                    new_tuple->next = tuple;
                    source->value = new_tuple;
                }
            }
            break;
        }
    }

    mpz_clear(term);
    mpz_clear(index);

    not_record_link_increase(source);
    return_value = source;

region_cleanup:
    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i])
        {
            if (not_record_link_decrease(record_arg[i]) < 0)
            {
                if (return_value != NOT_PTR_ERROR)
                {
                    not_record_link_decrease(return_value);
                }
                return NOT_PTR_ERROR;
            }
        }
    }

    return return_value;
}

not_record_t *
not_attribute_tuple_builtin_append(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 1)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Append", 1, cnt1);
            return NOT_PTR_ERROR;
        }
    }
    else
    {
        not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Append", 1, 0);
        return NOT_PTR_ERROR;
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    not_node_block_t *block = (not_node_block_t *)arguments->value;
    not_record_t *record_arg[1];

    size_t array_length = sizeof(record_arg) / sizeof(record_arg[0]);
    for (size_t i = 0; i < array_length; i++)
    {
        record_arg[i] = NOT_PTR_NULL;
    }

    size_t parameter_index = 0;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        not_node_argument_t *argument = (not_node_argument_t *)item->value;
        if (argument->value)
        {
            if (not_helper_id_strcmp(argument->key, "value") == 0)
            {
                not_record_t *arg = not_expression(argument->value, strip, applicant, NOT_PTR_NULL);
                if (arg == NOT_PTR_ERROR)
                {
                    goto region_cleanup;
                }

                record_arg[0] = arg;
            }
        }
        else
        {
            not_record_t *arg = not_expression(argument->key, strip, applicant, NOT_PTR_NULL);
            if (arg == NOT_PTR_ERROR)
            {
                goto region_cleanup;
            }

            if (parameter_index == 0)
            {
                if (arg->kind != RECORD_KIND_INT)
                {
                    not_error_type_by_node(argument->key, "mismatch: '%s' and '%s'",
                                           not_record_type_as_string(arg), "int");
                }
            }

            record_arg[parameter_index] = arg;

            parameter_index += 1;
        }
    }

    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i] == NOT_PTR_NULL)
        {
            if (i == 0)
            {
                not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "Append", "value");
            }
            goto region_cleanup;
        }
    }

    mpz_t length;
    mpz_init_set_si(length, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        mpz_add_ui(length, length, 1);
    }

    mpz_t term;
    mpz_init_set(term, length);

    mpz_clear(length);

    int appended = 0;
    mpz_t index;
    mpz_init_set_si(index, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value, *previous = NOT_PTR_NULL; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        if (mpz_cmp(index, term) < 0)
        {
            if (tuple->next == NOT_PTR_NULL)
            {
                not_record_t *arg = not_record_make_undefined();
                if (arg == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    goto region_cleanup;
                }

                not_record_tuple_t *new_tuple = not_record_make_tuple(arg, NOT_PTR_NULL);
                if (new_tuple == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    not_record_link_decrease(arg);
                    goto region_cleanup;
                }

                appended = 1;
                tuple->next = new_tuple;
            }
            mpz_add_ui(index, index, 1);

            previous = tuple;
            continue;
        }
        else if (mpz_cmp(index, term) == 0)
        {
            if (appended == 1)
            {
                if (not_record_link_decrease(tuple->value) < 0)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    goto region_cleanup;
                }

                not_record_link_increase(record_arg[0]);
                tuple->value = record_arg[0];
            }
            else
            {
                not_record_link_increase(record_arg[0]);
                not_record_tuple_t *new_tuple = not_record_make_tuple(record_arg[0], NOT_PTR_NULL);
                if (new_tuple == NOT_PTR_ERROR)
                {
                    mpz_clear(term);
                    mpz_clear(index);
                    not_record_link_decrease(record_arg[0]);
                    goto region_cleanup;
                }

                if (previous)
                {
                    new_tuple->next = tuple;
                    previous->next = new_tuple;
                }
                else
                {
                    new_tuple->next = tuple;
                    source->value = new_tuple;
                }
            }
            break;
        }
    }

    mpz_clear(term);
    mpz_clear(index);

    not_record_link_increase(source);
    return_value = source;

region_cleanup:
    for (size_t i = 0; i < array_length; i++)
    {
        if (record_arg[i])
        {
            if (not_record_link_decrease(record_arg[i]) < 0)
            {
                if (return_value != NOT_PTR_ERROR)
                {
                    not_record_link_decrease(return_value);
                }
                return NOT_PTR_ERROR;
            }
        }
    }

    return return_value;
}

not_record_t *
not_attribute_tuple_builtin_count(not_node_t *base, not_record_t *source, not_node_t *arguments, not_strip_t *strip, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;

        uint64_t cnt1 = 0;
        for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
        {
            cnt1 += 1;
        }

        if (cnt1 > 0)
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "Count", 0, cnt1);
            return NOT_PTR_ERROR;
        }
    }

    not_record_t *return_value = NOT_PTR_ERROR;

    mpz_t length;
    mpz_init_set_si(length, 0);
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)source->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        mpz_add_ui(length, length, 1);
    }

    return_value = not_record_make_int_from_z(length);

    mpz_clear(length);

    return return_value;
}

not_record_t *
not_attribute(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_binary_t *binary = (not_node_binary_t *)node->value;

    not_record_t *left = not_expression(binary->left, strip, applicant, origin);
    if (left == NOT_PTR_ERROR)
    {
        return NOT_PTR_ERROR;
    }

    if (left->kind == RECORD_KIND_TYPE)
    {
        not_record_type_t *record_type = (not_record_type_t *)left->value;
        not_node_t *type = record_type->type;
        not_strip_t *strip_new = (not_strip_t *)record_type->value;
        if (type->kind == NODE_KIND_CLASS)
        {
            not_record_t *result = not_attribute_from_type(node, strip_new, type, binary->right, applicant);

            if (result == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
            else if (result == NOT_PTR_NULL)
            {
                not_node_class_t *class1 = (not_node_class_t *)type->value;
                not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
                not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
                not_error_type_by_node(binary->right, "'%s' object has no attribute '%s'",
                                       basic2->value, basic1->value);

                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }

            if (not_record_link_decrease(left) < 0)
            {
                return NOT_PTR_ERROR;
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
                return NOT_PTR_ERROR;
            }

            return NOT_PTR_ERROR;
        }
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        not_record_struct_t *record_struct = (not_record_struct_t *)left->value;
        not_node_t *type = record_struct->type;
        not_strip_t *strip_new = (not_strip_t *)record_struct->value;

        not_record_t *result = not_attribute_from_struct(node, strip_new, type, binary->right, applicant);

        if (result == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }
        else if (result == NOT_PTR_NULL)
        {
            not_node_class_t *class1 = (not_node_class_t *)type->value;
            not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
            not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
            not_error_type_by_node(binary->right, "'%s' object has no attribute '%s'",
                                   basic2->value, basic1->value);

            if (not_record_link_decrease(left) < 0)
            {
                return NOT_PTR_ERROR;
            }

            return NOT_PTR_ERROR;
        }

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return result;
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (not_helper_id_strcmp(binary->right, "Remove") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_object_builtin_remove);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }
        else if (not_helper_id_strcmp(binary->right, "Add") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_object_builtin_add);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }

        for (not_record_object_t *item = (not_record_object_t *)left->value; item != NOT_PTR_NULL; item = item->next)
        {
            if (not_helper_id_strcmp(binary->right, item->key) == 0)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return item->value;
            }
        }

        not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
        not_error_type_by_node(node, "'%s' has no attribute '%s'",
                               not_record_type_as_string(left), basic1->value);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return NOT_PTR_ERROR;
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (not_helper_id_strcmp(binary->right, "Remove") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_tuple_builtin_remove);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }
        else if (not_helper_id_strcmp(binary->right, "Set") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_tuple_builtin_set);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }
        else if (not_helper_id_strcmp(binary->right, "Insert") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_tuple_builtin_insert);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }
        else if (not_helper_id_strcmp(binary->right, "Count") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_tuple_builtin_count);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }
        else if (not_helper_id_strcmp(binary->right, "Append") == 0)
        {
            not_record_t *result = not_record_make_builtin(left, &not_attribute_tuple_builtin_append);
            if (result == NOT_PTR_ERROR)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                return NOT_PTR_ERROR;
            }
            return result;
        }

        not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
        not_error_type_by_node(node, "'%s' has no attribute '%s'",
                               not_record_type_as_string(left), basic1->value);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return NOT_PTR_ERROR;
    }
    else
    {
        not_node_basic_t *basic1 = (not_node_basic_t *)binary->right->value;
        not_error_type_by_node(binary->right, "'%s' has no attribute '%s'",
                               not_record_type_as_string(left), basic1->value);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return NOT_PTR_ERROR;
    }
}