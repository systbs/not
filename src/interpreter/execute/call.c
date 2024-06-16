#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>
#include <ffi.h>
#include <jansson.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../../error.h"
#include "../../mutex.h"
#include "../../config.h"
#include "../../interpreter.h"
#include "../../thread.h"
#include "../../memory.h"
#include "../../scanner/scanner.h"
#include "../../parser/syntax/syntax.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "../entry.h"
#include "execute.h"

static not_record_t *
not_call_provide_class(not_strip_t *strip, not_node_t *node, not_node_t *applicant);

int32_t
not_call_parameters_check_by_one_arg(not_node_t *base, not_strip_t *strip, not_node_t *parameters, not_record_t *arg, not_node_t *applicant)
{
    if (parameters)
    {
        not_node_block_t *block1 = (not_node_block_t *)parameters->value;
        not_node_t *item1 = block1->items;

        not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
        if ((parameter->flag & SYNTAX_MODIFIER_KARG) == SYNTAX_MODIFIER_KARG)
        {
            not_record_tuple_t *tuple = NULL;

            if (arg)
            {
                not_record_t *record_arg = not_record_copy(arg);
                if (record_arg == ERROR)
                {
                    return -1;
                }

                if (parameter->type)
                {
                    not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                    if (record_parameter_type == ERROR)
                    {
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }

                    if (record_parameter_type->kind != RECORD_KIND_TYPE)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                        not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                               basic1->value, not_record_type_as_string(record_parameter_type));

                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }

                        return -1;
                    }

                    int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                    if (r1 < 0)
                    {
                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }

                        return -1;
                    }
                    else if (r1 == 0)
                    {
                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                        {
                            if (tuple == NULL)
                            {
                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return 0;
                            }
                        }
                        else
                        {
                            if (record_arg->link > 1)
                            {
                                not_record_t *record_copy = not_record_copy(record_arg);
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }
                                record_arg = record_copy;
                            }

                            not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                            if (record_arg2 == ERROR)
                            {
                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }
                            else if (record_arg2 == NULL)
                            {
                                if (tuple == NULL)
                                {
                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return 0;
                                }
                            }

                            record_arg = record_arg2;
                        }
                    }

                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }
                }

                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (record_arg->link > 1)
                    {
                        not_record_t *record_copy = not_record_copy(record_arg);
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        record_arg = record_copy;
                    }
                }

                not_record_tuple_t *tuple2 = not_record_make_tuple(record_arg, tuple);
                if (tuple2 == ERROR)
                {
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }

                    if (tuple)
                    {
                        if (not_record_tuple_destroy(tuple) < 0)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }
                    }

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                tuple = tuple2;
            }

            not_record_t *record_arg = not_record_create(RECORD_KIND_TUPLE, tuple);
            if (record_arg == ERROR)
            {
                if (tuple)
                {
                    if (not_record_tuple_destroy(tuple) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            if (not_record_link_decrease(record_arg) < 0)
            {
                return -1;
            }

            item1 = item1->next;
        }
        else if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
        {
            return 0;
        }
        else
        {
            not_record_t *record_arg = not_record_copy(arg);
            if (record_arg == ERROR)
            {
                return -1;
            }

            if (parameter->type)
            {
                not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                if (record_parameter_type == ERROR)
                {
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                if (record_parameter_type->kind != RECORD_KIND_TYPE)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                    not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                           basic1->value, not_record_type_as_string(record_parameter_type));

                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }

                int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                if (r1 < 0)
                {
                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }
                else if (r1 == 0)
                {
                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }

                        return 0;
                    }
                    else
                    {
                        if (record_arg->link > 1)
                        {
                            not_record_t *record_copy = not_record_copy(record_arg);
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            record_arg = record_copy;
                        }

                        not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                        if (record_arg2 == ERROR)
                        {
                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }
                        else if (record_arg2 == NULL)
                        {
                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return 0;
                        }

                        record_arg = record_arg2;
                    }
                }

                if (not_record_link_decrease(record_parameter_type) < 0)
                {
                    return -1;
                }
            }

            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
            {
                if (record_arg->link > 1)
                {
                    not_record_t *record_copy = not_record_copy(record_arg);
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }
                    record_arg = record_copy;
                }
            }

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            if (not_record_link_decrease(record_arg) < 0)
            {
                return -1;
            }

            item1 = item1->next;
        }

        for (; item1 != NULL; item1 = item1->next)
        {
            not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
            if (!parameter->value)
            {
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

int32_t
not_call_parameters_subs_by_one_arg(not_node_t *base, not_node_t *scope, not_strip_t *strip, not_node_t *parameters, not_record_t *arg, not_node_t *applicant)
{
    if (parameters)
    {
        not_node_block_t *block1 = (not_node_block_t *)parameters->value;
        not_node_t *item1 = block1->items;

        not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
        if ((parameter->flag & SYNTAX_MODIFIER_KARG) == SYNTAX_MODIFIER_KARG)
        {
            not_record_tuple_t *tuple = NULL;

            if (arg)
            {
                not_record_t *record_arg = not_record_copy(arg);
                if (record_arg == ERROR)
                {
                    return -1;
                }

                if (parameter->type)
                {
                    not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                    if (record_parameter_type == ERROR)
                    {
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }

                    if (record_parameter_type->kind != RECORD_KIND_TYPE)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                        not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                               basic1->value, not_record_type_as_string(record_parameter_type));

                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }

                        return -1;
                    }

                    int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                    if (r1 < 0)
                    {
                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }

                        return -1;
                    }
                    else if (r1 == 0)
                    {
                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                        {
                            if (tuple == NULL)
                            {
                                not_error_type_by_node(base, "'%s' mismatch: '%s' and '%s'",
                                                       "argument", not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }
                        }
                        else
                        {
                            if (record_arg->link > 1)
                            {
                                not_record_t *record_copy = not_record_copy(record_arg);
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }
                                record_arg = record_copy;
                            }

                            not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                            if (record_arg2 == ERROR)
                            {
                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }
                            else if (record_arg2 == NULL)
                            {
                                if (tuple == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                    not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                           basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }
                            }

                            record_arg = record_arg2;
                        }
                    }

                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }
                }

                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                {
                    if (record_arg->link > 1)
                    {
                        not_record_t *record_copy = not_record_copy(record_arg);
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        record_arg = record_copy;
                    }
                }

                not_record_tuple_t *tuple2 = not_record_make_tuple(record_arg, tuple);
                if (tuple2 == ERROR)
                {
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }

                    if (tuple)
                    {
                        if (not_record_tuple_destroy(tuple) < 0)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }
                    }

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                tuple = tuple2;
            }

            not_record_t *record_arg = not_record_create(RECORD_KIND_TUPLE, tuple);
            if (record_arg == ERROR)
            {
                if (tuple)
                {
                    if (not_record_tuple_destroy(tuple) < 0)
                    {
                        return -1;
                    }
                }
                return -1;
            }

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            not_entry_t *entry = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
            if (entry == ERROR)
            {
                if (not_record_link_decrease(record_arg) < 0)
                {
                    return -1;
                }
                return -1;
            }

            item1 = item1->next;
        }
        else if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
        {
            not_error_type_by_node(base, "'%s' mismatch: '%s' and '%s'",
                                   "argument", not_record_type_as_string(arg), "kwarg");
            return -1;
        }
        else
        {
            not_record_t *record_arg = not_record_copy(arg);
            if (record_arg == ERROR)
            {
                return -1;
            }

            if (parameter->type)
            {
                not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                if (record_parameter_type == ERROR)
                {
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }
                    return -1;
                }

                if (record_parameter_type->kind != RECORD_KIND_TYPE)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                    not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                           basic1->value, not_record_type_as_string(record_parameter_type));

                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }

                int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                if (r1 < 0)
                {
                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }
                else if (r1 == 0)
                {
                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                    {
                        not_error_type_by_node(base, "'%s' mismatch: '%s' and '%s'",
                                               "argument", not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }

                        return -1;
                    }
                    else
                    {
                        if (record_arg->link > 1)
                        {
                            not_record_t *record_copy = not_record_copy(record_arg);
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            record_arg = record_copy;
                        }

                        not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                        if (record_arg2 == ERROR)
                        {
                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }
                        else if (record_arg2 == NULL)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                            not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                   basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }

                        record_arg = record_arg2;
                    }
                }

                if (not_record_link_decrease(record_parameter_type) < 0)
                {
                    return -1;
                }
            }

            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
            {
                if (record_arg->link > 1)
                {
                    not_record_t *record_copy = not_record_copy(record_arg);
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return -1;
                    }
                    record_arg = record_copy;
                }
            }

            if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
            {
                record_arg->readonly = 1;
            }

            if (parameter->type)
            {
                record_arg->typed = 1;
            }

            not_entry_t *entry = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
            if (entry == ERROR)
            {
                if (not_record_link_decrease(record_arg) < 0)
                {
                    return -1;
                }
                return -1;
            }

            item1 = item1->next;
        }

        for (; item1 != NULL; item1 = item1->next)
        {
            not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
            not_entry_t *entry = not_strip_input_find(strip, scope, parameter->key);
            if (!entry)
            {
                if (parameter->value)
                {
                    not_record_t *record_arg = not_execute_expression(parameter->value, strip, applicant, NULL);
                    if (record_arg == ERROR)
                    {
                        return -1;
                    }

                    if (parameter->type)
                    {
                        not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                        if (record_parameter_type == ERROR)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        if (record_parameter_type->kind != RECORD_KIND_TYPE)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                            not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                                   basic1->value, not_record_type_as_string(record_parameter_type));

                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }

                        int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                        if (r1 < 0)
                        {
                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }
                        else if (r1 == 0)
                        {
                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                       basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }
                            else
                            {
                                if (record_arg->link > 1)
                                {
                                    not_record_t *record_copy = not_record_copy(record_arg);
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    record_arg = record_copy;
                                }

                                not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                if (record_arg2 == ERROR)
                                {
                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }
                                else if (record_arg2 == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                    not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                           basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }

                                record_arg = record_arg2;
                            }
                        }

                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (record_arg->link > 1)
                        {
                            not_record_t *record_copy = not_record_copy(record_arg);
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            record_arg = record_copy;
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                    {
                        record_arg->readonly = 1;
                    }

                    if (parameter->type)
                    {
                        record_arg->typed = 1;
                    }

                    not_entry_t *entry2 = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                    if (entry2 == ERROR)
                    {
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }
                }
                else
                {
                    if (scope->kind == NODE_KIND_FUN)
                    {
                        not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                        not_node_basic_t *basic2 = (not_node_basic_t *)parameter->key->value;
                        not_error_type_by_node(base, "'%s' missing '%s' required positional argument", basic1->value, basic2->value);
                        return -1;
                    }
                    else
                    {
                        not_node_basic_t *basic2 = (not_node_basic_t *)parameter->key->value;
                        not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "lambda", basic2->value);
                        return -1;
                    }
                }
            }
        }
    }
    else
    {
        if (scope->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
            not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", basic1->value, 0, 1);
            return -1;
        }
        else
        {
            not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "lambda", 0, 1);
            return -1;
        }
    }
    return 0;
}

int32_t
not_call_parameters_subs(not_node_t *base, not_node_t *scope, not_strip_t *strip, not_node_t *parameters, not_node_t *arguments, not_node_t *applicant)
{
    if (arguments)
    {
        not_node_block_t *block1 = (not_node_block_t *)arguments->value;

        if (!parameters)
        {
            uint64_t cnt1 = 0;
            for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
            {
                cnt1 += 1;
            }

            if (scope->kind == NODE_KIND_FUN)
            {
                not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
                not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", basic1->value, 0, cnt1);
                return -1;
            }
            else
            {
                not_error_type_by_node(base, "'%s' takes %lld positional arguments but %lld were given", "lambda", 0, cnt1);
                return -1;
            }
        }
    }

    if (parameters)
    {
        not_node_block_t *block1 = (not_node_block_t *)parameters->value;
        not_node_t *item1 = block1->items;
        if (arguments)
        {
            not_node_block_t *block2 = (not_node_block_t *)arguments->value;
            for (not_node_t *item2 = block2->items; item2 != NULL;)
            {
                not_node_argument_t *argument = (not_node_argument_t *)item2->value;

                if (!item1)
                {
                    if (argument->value)
                    {
                        if (scope->kind == NODE_KIND_FUN)
                        {
                            not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
                            not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                            not_node_basic_t *basic2 = (not_node_basic_t *)argument->key->value;
                            not_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", basic1->value, basic2->value);
                            return -1;
                        }
                        else
                        {
                            not_node_basic_t *basic2 = (not_node_basic_t *)argument->key->value;
                            not_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", "lambda", basic2->value);
                            return -1;
                        }
                    }
                    else
                    {
                        uint64_t cnt1 = 0;
                        for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                        {
                            cnt1 += 1;
                        }

                        uint64_t cnt2 = 0;
                        for (not_node_t *item1 = block2->items; item1 != NULL; item1 = item1->next)
                        {
                            cnt2 += 1;
                        }

                        if (scope->kind == NODE_KIND_FUN)
                        {
                            not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
                            not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                            not_error_type_by_node(argument->key, "'%s' takes %lld positional arguments but %lld were given", basic1->value, cnt1, cnt2);
                            return -1;
                        }
                        else
                        {
                            not_error_type_by_node(argument->key, "'%s' takes %lld positional arguments but %lld were given", "lambda", cnt1, cnt2);
                            return -1;
                        }

                        return -1;
                    }
                }

                if (argument->value)
                {
                    not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
                    if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
                    {
                        not_record_object_t *object = NULL, *top = NULL;

                        for (; item2 != NULL; item2 = item2->next)
                        {
                            argument = (not_node_argument_t *)item2->value;
                            if (!argument->value)
                            {
                                break;
                            }

                            not_record_t *record_arg = not_execute_expression(argument->value, strip, applicant, NULL);
                            if (record_arg == ERROR)
                            {
                                return -1;
                            }

                            if (parameter->type)
                            {
                                not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                                if (record_parameter_type == ERROR)
                                {
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    return -1;
                                }

                                if (record_parameter_type->kind != RECORD_KIND_TYPE)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                    not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                                           basic1->value, not_record_type_as_string(record_parameter_type));

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }

                                int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                if (r1 < 0)
                                {
                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }
                                else if (r1 == 0)
                                {
                                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                    {
                                        if (object == NULL)
                                        {
                                            not_node_basic_t *basic1 = (not_node_basic_t *)argument->key->value;
                                            not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                                   basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                            if (not_record_link_decrease(record_parameter_type) < 0)
                                            {
                                                return -1;
                                            }

                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }

                                            return -1;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (record_arg->link > 1)
                                        {
                                            not_record_t *record_copy = not_record_copy(record_arg);
                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                            record_arg = record_copy;
                                        }

                                        not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                        if (record_arg2 == ERROR)
                                        {
                                            if (not_record_link_decrease(record_parameter_type) < 0)
                                            {
                                                return -1;
                                            }

                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }

                                            return -1;
                                        }
                                        else if (record_arg2 == NULL)
                                        {
                                            if (object == NULL)
                                            {
                                                not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                                not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                                       basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                                if (not_record_link_decrease(record_parameter_type) < 0)
                                                {
                                                    return -1;
                                                }

                                                if (not_record_link_decrease(record_arg) < 0)
                                                {
                                                    return -1;
                                                }

                                                return -1;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }

                                        record_arg = record_arg2;
                                    }
                                }

                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }
                            }

                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                            {
                                if (record_arg->link > 1)
                                {
                                    not_record_t *record_copy = not_record_copy(record_arg);
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    record_arg = record_copy;
                                }
                            }

                            not_node_basic_t *basic = (not_node_basic_t *)argument->key->value;
                            not_record_object_t *object2 = not_record_make_object(basic->value, record_arg, NULL);
                            if (object2 == ERROR)
                            {
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                if (object)
                                {
                                    if (not_record_object_destroy(object) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            if (object == NULL)
                            {
                                object = top = object2;
                            }
                            else
                            {
                                object->next = object2;
                                object = object->next;
                            }
                        }

                        not_record_t *record_arg = not_record_create(RECORD_KIND_OBJECT, top);
                        if (record_arg == ERROR)
                        {
                            if (object)
                            {
                                if (not_record_object_destroy(object) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                        {
                            record_arg->readonly = 1;
                        }

                        if (parameter->type)
                        {
                            record_arg->typed = 1;
                        }

                        not_entry_t *entry = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        item1 = item1->next;
                        continue;
                    }
                    else
                    {
                        int8_t found = 0;
                        for (not_node_t *item3 = item1; item3 != NULL; item3 = item3->next)
                        {
                            parameter = (not_node_parameter_t *)item3->value;
                            if (not_execute_id_cmp(argument->key, parameter->key) == 1)
                            {
                                found = 1;

                                not_record_t *record_arg = not_execute_expression(argument->value, strip, applicant, NULL);
                                if (record_arg == ERROR)
                                {
                                    return -1;
                                }

                                if (parameter->type)
                                {
                                    not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                                    if (record_parameter_type == ERROR)
                                    {
                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                        return -1;
                                    }

                                    if (record_parameter_type->kind != RECORD_KIND_TYPE)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                        not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                                               basic1->value, not_record_type_as_string(record_parameter_type));

                                        if (not_record_link_decrease(record_parameter_type) < 0)
                                        {
                                            return -1;
                                        }

                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }

                                        return -1;
                                    }

                                    int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                    if (r1 < 0)
                                    {
                                        if (not_record_link_decrease(record_parameter_type) < 0)
                                        {
                                            return -1;
                                        }

                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }

                                        return -1;
                                    }
                                    else if (r1 == 0)
                                    {
                                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                        {
                                            not_node_basic_t *basic1 = (not_node_basic_t *)argument->key->value;
                                            not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                                   basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                            if (not_record_link_decrease(record_parameter_type) < 0)
                                            {
                                                return -1;
                                            }

                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }

                                            return -1;
                                        }
                                        else
                                        {
                                            if (record_arg->link > 1)
                                            {
                                                not_record_t *record_copy = not_record_copy(record_arg);
                                                if (not_record_link_decrease(record_arg) < 0)
                                                {
                                                    return -1;
                                                }
                                                record_arg = record_copy;
                                            }

                                            not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                            if (record_arg2 == ERROR)
                                            {
                                                if (not_record_link_decrease(record_parameter_type) < 0)
                                                {
                                                    return -1;
                                                }

                                                if (not_record_link_decrease(record_arg) < 0)
                                                {
                                                    return -1;
                                                }

                                                return -1;
                                            }
                                            else if (record_arg2 == NULL)
                                            {
                                                not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                                not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                                       basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                                if (not_record_link_decrease(record_parameter_type) < 0)
                                                {
                                                    return -1;
                                                }

                                                if (not_record_link_decrease(record_arg) < 0)
                                                {
                                                    return -1;
                                                }

                                                return -1;
                                            }

                                            record_arg = record_arg2;
                                        }
                                    }

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }
                                }

                                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                                {
                                    if (record_arg->link > 1)
                                    {
                                        not_record_t *record_copy = not_record_copy(record_arg);
                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                        record_arg = record_copy;
                                    }
                                }

                                if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                                {
                                    record_arg->readonly = 1;
                                }

                                if (parameter->type)
                                {
                                    record_arg->typed = 1;
                                }

                                not_entry_t *entry = not_strip_input_push(strip, scope, item3, parameter->key, record_arg);
                                if (entry == ERROR)
                                {
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    return -1;
                                }

                                break;
                            }
                        }

                        if (found == 0)
                        {
                            if (scope->kind == NODE_KIND_FUN)
                            {
                                not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
                                not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                                not_node_basic_t *basic2 = (not_node_basic_t *)argument->key->value;
                                not_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", basic1->value, basic2->value);
                                return -1;
                            }
                            else
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)argument->key->value;
                                not_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", "lambda", basic1->value);
                                return -1;
                            }
                        }
                    }
                }
                else
                {
                    not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
                    if ((parameter->flag & SYNTAX_MODIFIER_KARG) == SYNTAX_MODIFIER_KARG)
                    {
                        not_record_tuple_t *tuple = NULL, *top = NULL;

                        for (; item2 != NULL; item2 = item2->next)
                        {
                            argument = (not_node_argument_t *)item2->value;

                            not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, NULL);
                            if (record_arg == ERROR)
                            {
                                return -1;
                            }

                            if (parameter->type)
                            {
                                not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                                if (record_parameter_type == ERROR)
                                {
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    return -1;
                                }

                                if (record_parameter_type->kind != RECORD_KIND_TYPE)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                    not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                                           basic1->value, not_record_type_as_string(record_parameter_type));

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }

                                int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                if (r1 < 0)
                                {
                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }
                                else if (r1 == 0)
                                {
                                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                    {
                                        if (tuple == NULL)
                                        {
                                            not_node_basic_t *basic1 = (not_node_basic_t *)argument->key->value;
                                            not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                                   basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                            if (not_record_link_decrease(record_parameter_type) < 0)
                                            {
                                                return -1;
                                            }

                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }

                                            return -1;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (record_arg->link > 1)
                                        {
                                            not_record_t *record_copy = not_record_copy(record_arg);
                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }
                                            record_arg = record_copy;
                                        }

                                        not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                        if (record_arg2 == ERROR)
                                        {
                                            if (not_record_link_decrease(record_parameter_type) < 0)
                                            {
                                                return -1;
                                            }

                                            if (not_record_link_decrease(record_arg) < 0)
                                            {
                                                return -1;
                                            }

                                            return -1;
                                        }
                                        else if (record_arg2 == NULL)
                                        {
                                            if (tuple == NULL)
                                            {
                                                not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                                not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                                       basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                                if (not_record_link_decrease(record_parameter_type) < 0)
                                                {
                                                    return -1;
                                                }

                                                if (not_record_link_decrease(record_arg) < 0)
                                                {
                                                    return -1;
                                                }

                                                return -1;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }

                                        record_arg = record_arg2;
                                    }
                                }

                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }
                            }

                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                            {
                                if (record_arg->link > 1)
                                {
                                    not_record_t *record_copy = not_record_copy(record_arg);
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    record_arg = record_copy;
                                }
                            }

                            not_record_tuple_t *tuple2 = not_record_make_tuple(record_arg, NULL);
                            if (tuple2 == ERROR)
                            {
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                if (tuple)
                                {
                                    if (not_record_tuple_destroy(tuple) < 0)
                                    {
                                        return -1;
                                    }
                                }
                                return -1;
                            }

                            if (tuple == NULL)
                            {
                                tuple = top = tuple2;
                            }
                            else
                            {
                                tuple->next = tuple2;
                                tuple = tuple->next;
                            }
                        }

                        not_record_t *record_arg = not_record_create(RECORD_KIND_TUPLE, top);
                        if (record_arg == ERROR)
                        {
                            if (tuple)
                            {
                                if (not_record_tuple_destroy(tuple) < 0)
                                {
                                    return -1;
                                }
                            }
                            return -1;
                        }

                        if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                        {
                            record_arg->readonly = 1;
                        }

                        if (parameter->type)
                        {
                            record_arg->typed = 1;
                        }

                        not_entry_t *entry = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        item1 = item1->next;
                        continue;
                    }
                    else if ((parameter->flag & SYNTAX_MODIFIER_KWARG) == SYNTAX_MODIFIER_KWARG)
                    {
                        not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, NULL);
                        if (record_arg == ERROR)
                        {
                            return -1;
                        }

                        not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                               "argument", not_record_type_as_string(record_arg), "kwarg");

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }
                    else
                    {
                        not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, NULL);
                        if (record_arg == ERROR)
                        {
                            return -1;
                        }

                        if (parameter->type)
                        {
                            not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                            if (record_parameter_type == ERROR)
                            {
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }
                                return -1;
                            }

                            if (record_parameter_type->kind != RECORD_KIND_TYPE)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                                       basic1->value, not_record_type_as_string(record_parameter_type));

                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }

                            int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                            if (r1 < 0)
                            {
                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }
                            else if (r1 == 0)
                            {
                                if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                                {
                                    not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                           "argument", not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }
                                else
                                {
                                    if (record_arg->link > 1)
                                    {
                                        not_record_t *record_copy = not_record_copy(record_arg);
                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }
                                        record_arg = record_copy;
                                    }

                                    not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                    if (record_arg2 == ERROR)
                                    {
                                        if (not_record_link_decrease(record_parameter_type) < 0)
                                        {
                                            return -1;
                                        }

                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }

                                        return -1;
                                    }
                                    else if (record_arg2 == NULL)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                        not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                               basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                        if (not_record_link_decrease(record_parameter_type) < 0)
                                        {
                                            return -1;
                                        }

                                        if (not_record_link_decrease(record_arg) < 0)
                                        {
                                            return -1;
                                        }

                                        return -1;
                                    }

                                    record_arg = record_arg2;
                                }
                            }

                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }
                        }

                        if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                        {
                            if (record_arg->link > 1)
                            {
                                not_record_t *record_copy = not_record_copy(record_arg);
                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }
                                record_arg = record_copy;
                            }
                        }

                        if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                        {
                            record_arg->readonly = 1;
                        }

                        if (parameter->type)
                        {
                            record_arg->typed = 1;
                        }

                        not_entry_t *entry = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                        if (entry == ERROR)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        item1 = item1->next;
                    }
                }

                item2 = item2->next;
            }
        }

        for (; item1 != NULL; item1 = item1->next)
        {
            not_node_parameter_t *parameter = (not_node_parameter_t *)item1->value;
            not_entry_t *entry = not_strip_input_find(strip, scope, parameter->key);
            if (!entry)
            {
                if (parameter->value)
                {
                    not_record_t *record_arg = not_execute_expression(parameter->value, strip, applicant, NULL);
                    if (record_arg == ERROR)
                    {
                        return -1;
                    }

                    if (parameter->type)
                    {
                        not_record_t *record_parameter_type = not_execute_expression(parameter->type, strip, applicant, NULL);
                        if (record_parameter_type == ERROR)
                        {
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            return -1;
                        }

                        if (record_parameter_type->kind != RECORD_KIND_TYPE)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                            not_error_type_by_node(parameter->type, "'%s' unsupported type: '%s'",
                                                   basic1->value, not_record_type_as_string(record_parameter_type));

                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }

                        int32_t r1 = not_execute_value_check_by_type(base, record_arg, record_parameter_type, strip, applicant);
                        if (r1 < 0)
                        {
                            if (not_record_link_decrease(record_parameter_type) < 0)
                            {
                                return -1;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }

                            return -1;
                        }
                        else if (r1 == 0)
                        {
                            if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                       basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                if (not_record_link_decrease(record_parameter_type) < 0)
                                {
                                    return -1;
                                }

                                if (not_record_link_decrease(record_arg) < 0)
                                {
                                    return -1;
                                }

                                return -1;
                            }
                            else
                            {
                                if (record_arg->link > 1)
                                {
                                    not_record_t *record_copy = not_record_copy(record_arg);
                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }
                                    record_arg = record_copy;
                                }

                                not_record_t *record_arg2 = not_execute_value_casting_by_type(base, record_arg, record_parameter_type, strip, applicant);
                                if (record_arg2 == ERROR)
                                {
                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }
                                else if (record_arg2 == NULL)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)parameter->key->value;
                                    not_error_type_by_node(parameter->key, "'%s' mismatch: '%s' and '%s'",
                                                           basic1->value, not_record_type_as_string(record_arg), not_record_type_as_string(record_parameter_type));

                                    if (not_record_link_decrease(record_parameter_type) < 0)
                                    {
                                        return -1;
                                    }

                                    if (not_record_link_decrease(record_arg) < 0)
                                    {
                                        return -1;
                                    }

                                    return -1;
                                }

                                record_arg = record_arg2;
                            }
                        }

                        if (not_record_link_decrease(record_parameter_type) < 0)
                        {
                            return -1;
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_REFERENCE) != SYNTAX_MODIFIER_REFERENCE)
                    {
                        if (record_arg->link > 1)
                        {
                            not_record_t *record_copy = not_record_copy(record_arg);
                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                return -1;
                            }
                            record_arg = record_copy;
                        }
                    }

                    if ((parameter->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
                    {
                        record_arg->readonly = 1;
                    }

                    if (parameter->type)
                    {
                        record_arg->typed = 1;
                    }

                    not_entry_t *entry2 = not_strip_input_push(strip, scope, item1, parameter->key, record_arg);
                    if (entry2 == ERROR)
                    {
                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            return -1;
                        }
                        return -1;
                    }
                }
                else
                {
                    if (scope->kind == NODE_KIND_FUN)
                    {
                        not_node_fun_t *fun1 = (not_node_fun_t *)scope->value;
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                        not_node_basic_t *basic2 = (not_node_basic_t *)parameter->key->value;
                        not_error_type_by_node(base, "'%s' missing '%s' required positional argument", basic1->value, basic2->value);
                        return -1;
                    }
                    else
                    {
                        not_node_basic_t *basic2 = (not_node_basic_t *)parameter->key->value;
                        not_error_type_by_node(base, "'%s' missing '%s' required positional argument", "lambda", basic2->value);
                        return -1;
                    }
                }
            }
        }
    }

    return 0;
}

static int32_t
not_call_heritage_subs(not_node_t *scope, not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_node_heritage_t *heritage = (not_node_heritage_t *)node->value;

    not_record_t *record_heritage = not_execute_expression(heritage->type, strip, applicant, NULL);
    if (record_heritage == ERROR)
    {
        return -1;
    }

    not_node_class_t *class1 = (not_node_class_t *)scope->value;

    if (record_heritage->kind != RECORD_KIND_TYPE)
    {
        not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
        not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
        not_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'",
                               basic2->value, basic1->value);

        if (not_record_link_decrease(record_heritage) < 0)
        {
            return -1;
        }
        return -1;
    }

    not_record_type_t *record_type = (not_record_type_t *)record_heritage->value;
    not_node_t *type = record_type->type;

    if (type->kind == NODE_KIND_CLASS)
    {
        not_strip_t *strip_new = (not_strip_t *)record_type->value;

        not_record_t *content = not_call_provide_class(strip_new, type, applicant);
        if (content == ERROR)
        {
            if (not_record_link_decrease(record_heritage) < 0)
            {
                return -1;
            }
            return -1;
        }

        if (not_record_link_decrease(record_heritage) < 0)
        {
            return -1;
        }

        content->readonly = 1;
        content->typed = 1;

        not_entry_t *entry = not_strip_variable_push(strip, scope, node, heritage->key, content);
        if (entry == ERROR)
        {
            if (not_record_link_decrease(content) < 0)
            {
                return -1;
            }
            return -1;
        }
        else if (entry == NULL)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
            not_error_type_by_node(heritage->key, "'%s' already set", basic1->value);
            if (not_record_link_decrease(content) < 0)
            {
                return -1;
            }
            return -1;
        }
    }
    else
    {
        not_node_basic_t *basic1 = (not_node_basic_t *)heritage->key->value;
        not_node_basic_t *basic2 = (not_node_basic_t *)class1->key->value;
        not_error_type_by_node(heritage->key, "'%s' unexpected type as heritage '%s'",
                               basic2->value, basic1->value);

        if (not_record_link_decrease(record_heritage) < 0)
        {
            return -1;
        }
        return -1;
    }

    return 0;
}

static int32_t
not_call_property_subs(not_node_t *scope, not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_node_property_t *property = (not_node_property_t *)node->value;
    not_record_t *record_value = NULL;

    if (property->value)
    {

        record_value = not_execute_expression(property->value, strip, applicant, NULL);
        if (record_value == ERROR)
        {
            return -1;
        }

        if (property->type)
        {
            not_record_t *record_parameter_type = not_execute_expression(property->type, strip, applicant, NULL);
            if (record_parameter_type == ERROR)
            {
                if (not_record_link_decrease(record_value) < 0)
                {
                    return -1;
                }
                return -1;
            }

            if (record_parameter_type->kind != RECORD_KIND_TYPE)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                not_error_type_by_node(property->type, "'%s' unsupported type: '%s'",
                                       basic1->value, not_record_type_as_string(record_parameter_type));

                if (not_record_link_decrease(record_parameter_type) < 0)
                {
                    return -1;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return -1;
                }

                return -1;
            }

            int32_t r1 = not_execute_value_check_by_type(property->key, record_value, record_parameter_type, strip, applicant);
            if (r1 < 0)
            {
                if (not_record_link_decrease(record_parameter_type) < 0)
                {
                    return -1;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return -1;
                }

                return -1;
            }
            else if (r1 == 0)
            {
                if (record_value->link > 0)
                {
                    record_value = not_record_copy(record_value);
                }

                not_record_t *record_arg2 = not_execute_value_casting_by_type(property->key, record_value, record_parameter_type, strip, applicant);
                if (record_arg2 == ERROR)
                {
                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }

                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }
                else if (record_arg2 == NULL)
                {
                    not_node_basic_t *basic1 = (not_node_basic_t *)property->key->value;
                    not_error_type_by_node(property->key, "'%s' mismatch: '%s' and '%s'",
                                           basic1->value, not_record_type_as_string(record_value), not_record_type_as_string(record_parameter_type));

                    if (not_record_link_decrease(record_parameter_type) < 0)
                    {
                        return -1;
                    }

                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return -1;
                    }

                    return -1;
                }

                record_value = record_arg2;
            }

            if (not_record_link_decrease(record_parameter_type) < 0)
            {
                return -1;
            }
        }
    }
    else
    {
        record_value = not_record_make_undefined();
        if (record_value == ERROR)
        {
            return -1;
        }
    }

    if (record_value->link > 0)
    {
        not_record_t *record_copy = not_record_copy(record_value);
        if (not_record_link_decrease(record_value) < 0)
        {
            return -1;
        }
        record_value = record_copy;
    }

    if ((property->flag & SYNTAX_MODIFIER_READONLY) == SYNTAX_MODIFIER_READONLY)
    {
        record_value->readonly = 1;
    }

    if (property->type)
    {
        record_value->typed = 1;
    }

    not_entry_t *entry = not_strip_variable_push(strip, scope, node, property->key, record_value);
    if (entry == ERROR)
    {
        if (not_record_link_decrease(record_value) < 0)
        {
            return -1;
        }
        return -1;
    }

    return 0;
}

static not_record_t *
not_call_provide_class(not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_node_class_t *class1 = (not_node_class_t *)node->value;

    not_strip_t *strip_class = not_strip_copy(strip);
    if (strip_class == ERROR)
    {
        return ERROR;
    }

    if (class1->heritages)
    {
        not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
        for (not_node_t *item = block->items; item != NULL; item = item->next)
        {
            if (not_call_heritage_subs(node, strip_class, item, applicant) < 0)
            {
                if (not_strip_destroy(strip_class) < 0)
                {
                    return ERROR;
                }
            }
        }
    }

    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_PROPERTY)
        {
            not_node_property_t *property = (not_node_property_t *)item->value;
            if ((property->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
            {
                if (not_call_property_subs(node, strip_class, item, applicant) < 0)
                {
                    if (not_strip_destroy(strip_class) < 0)
                    {
                        return ERROR;
                    }
                }
            }
        }
    }

    not_record_t *result = not_record_make_struct(node, strip_class);
    if (result == ERROR)
    {
        if (not_strip_destroy(strip_class) < 0)
        {
            return ERROR;
        }
    }

    return result;
}

not_record_t *
not_call_operator_by_one_arg(not_node_t *base, not_record_t *content, not_record_t *arg, const char *operator, not_node_t * applicant)
{
    not_record_struct_t *struct1 = (not_record_struct_t *)content->value;
    not_node_t *type = struct1->type;
    not_strip_t *strip_class = struct1->value;

    not_node_class_t *class1 = (not_node_class_t *)type->value;

    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_execute_id_strcmp(fun1->key, operator) == 1)
            {
                not_strip_t *strip_copy = not_strip_copy(strip_class);
                if (strip_copy == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_fun = not_strip_create(strip_copy);
                if (strip_fun == ERROR)
                {
                    if (not_strip_destroy(strip_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_call_parameters_subs_by_one_arg(base, item, strip_fun, fun1->parameters, arg, applicant) < 0)
                {
                    if (not_strip_destroy(strip_fun) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                int32_t r1 = not_execute_run_fun(item, strip_fun, applicant);
                if (r1 == -1)
                {
                    if (not_strip_destroy(strip_fun) < 0)
                    {
                        return ERROR;
                    }

                    return ERROR;
                }

                if (not_strip_destroy(strip_fun) < 0)
                {
                    return ERROR;
                }

                not_record_t *rax = not_thread_get_and_set_rax(NULL);
                if (rax == ERROR)
                {
                    return ERROR;
                }
                else if (!rax)
                {
                    rax = not_record_make_undefined();
                }

                return rax;
            }
        }
    }

    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
    not_error_type_by_node(base, "'%s' no operator %s was found", basic1->value, operator);

    return ERROR;
}

static not_record_t *
not_call_class(not_node_t *base, not_node_t *arguments, not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_record_t *content = not_call_provide_class(strip, node, applicant);
    if (content == ERROR)
    {
        return ERROR;
    }

    not_record_struct_t *struct1 = (not_record_struct_t *)content->value;
    not_node_t *type = struct1->type;
    not_strip_t *strip_class = struct1->value;

    not_node_class_t *class1 = (not_node_class_t *)type->value;

    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_execute_id_strcmp(fun1->key, CONSTRUCTOR_STR) == 1)
            {
                not_strip_t *strip_copy = not_strip_copy(strip_class);
                if (strip_copy == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_fun = not_strip_create(strip_copy);
                if (strip_fun == ERROR)
                {
                    if (not_record_link_decrease(content) < 0)
                    {
                        return ERROR;
                    }
                    if (not_strip_destroy(strip_copy) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_call_parameters_subs(base, item, strip_fun, fun1->parameters, arguments, applicant) < 0)
                {
                    if (not_strip_destroy(strip_fun) < 0)
                    {
                        if (not_record_link_decrease(content) < 0)
                        {
                            return ERROR;
                        }
                        return ERROR;
                    }
                    if (not_record_link_decrease(content) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }
                int32_t r1 = not_execute_run_fun(item, strip_fun, applicant);
                if (r1 == -1)
                {
                    if (not_strip_destroy(strip_fun) < 0)
                    {
                        if (not_record_link_decrease(content) < 0)
                        {
                            return ERROR;
                        }
                        return ERROR;
                    }

                    if (not_record_link_decrease(content) < 0)
                    {
                        return ERROR;
                    }

                    return ERROR;
                }

                if (not_strip_destroy(strip_fun) < 0)
                {
                    if (not_record_link_decrease(content) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                not_record_t *rax = not_thread_get_and_set_rax(NULL);
                if (rax == ERROR)
                {
                    return ERROR;
                }
                else if (!rax)
                {
                    rax = content;
                }
                else
                {
                    if (not_record_link_decrease(content) < 0)
                    {
                        return ERROR;
                    }
                }

                return rax;
            }
        }
    }

    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
    not_error_type_by_node(base, "'%s' no constructor was found", basic1->value);
    if (not_record_link_decrease(content) < 0)
    {
        return ERROR;
    }
    return ERROR;
}

static not_record_t *
not_call_fun(not_node_t *base, not_node_t *arguments, not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_node_fun_t *fun1 = (not_node_fun_t *)node->value;

    not_strip_t *strip_copy = not_strip_copy(strip);
    if (strip_copy == ERROR)
    {
        return ERROR;
    }

    not_strip_t *strip_lambda = not_strip_create(strip_copy);
    if (strip_lambda == ERROR)
    {
        if (not_strip_destroy(strip_copy) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (not_call_parameters_subs(base, node, strip_lambda, fun1->parameters, arguments, applicant) < 0)
    {
        if (not_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    int32_t r1 = not_execute_run_fun(node, strip_lambda, applicant);
    if (r1 == -1)
    {
        if (not_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (not_strip_destroy(strip_lambda) < 0)
    {
        return ERROR;
    }

    not_record_t *rax = not_thread_get_and_set_rax(NULL);
    if (rax == ERROR)
    {
        return ERROR;
    }
    else if (!rax)
    {
        rax = not_record_make_undefined();
        if (rax == ERROR)
        {
            return ERROR;
        }
    }

    return rax;
}

static not_record_t *
not_call_lambda(not_node_t *base, not_node_t *arguments, not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_node_lambda_t *fun1 = (not_node_lambda_t *)node->value;

    not_strip_t *strip_copy = not_strip_copy(strip);
    if (strip_copy == ERROR)
    {
        return ERROR;
    }

    not_strip_t *strip_lambda = not_strip_create(strip_copy);
    if (strip_lambda == ERROR)
    {
        if (not_strip_destroy(strip_copy) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (not_call_parameters_subs(base, node, strip_lambda, fun1->parameters, arguments, applicant) < 0)
    {
        if (not_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    int32_t r1 = not_execute_run_lambda(node, strip_lambda, applicant);
    if (r1 == -1)
    {
        if (not_strip_destroy(strip_lambda) < 0)
        {
            return ERROR;
        }
        return ERROR;
    }

    if (not_strip_destroy(strip_lambda) < 0)
    {
        return ERROR;
    }

    not_record_t *rax = not_thread_get_and_set_rax(NULL);
    if (rax == ERROR)
    {
        return ERROR;
    }
    else if (!rax)
    {
        rax = not_record_make_undefined();
        if (rax == ERROR)
        {
            return ERROR;
        }
    }

    return rax;
}

typedef union
{
    char c;
    short s;
    int i;
    long l;
    unsigned char uc;
    unsigned short us;
    unsigned int ui;
    unsigned long ul;
    void *ptr;
    float f;
    double d;
    long double ld;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
} ffi_ret_type;

typedef struct ffi_sized
{
    void *ptr;
    ffi_type *type;
    size_t size;
} ffi_sized_t;

static void
ffi_type_destroy(ffi_type *type)
{
}

static ffi_type *
json_to_ffi(json_t *type)
{
    if (json_is_string(type))
    {
        const char *str = json_string_value(type);
        if (strcmp(str, "schar") == 0)
        {
            return &ffi_type_schar;
        }
        else if (strcmp(str, "uchar") == 0)
        {
            return &ffi_type_uchar;
        }
        else if (strcmp(str, "sshort") == 0)
        {
            return &ffi_type_sshort;
        }
        else if (strcmp(str, "ushort") == 0)
        {
            return &ffi_type_ushort;
        }
        else if (strcmp(str, "sint") == 0)
        {
            return &ffi_type_sint;
        }
        else if (strcmp(str, "uint") == 0)
        {
            return &ffi_type_uint;
        }
        else if (strcmp(str, "slong") == 0)
        {
            return &ffi_type_slong;
        }
        else if (strcmp(str, "ulong") == 0)
        {
            return &ffi_type_ulong;
        }
        else if (strcmp(str, "sint8") == 0)
        {
            return &ffi_type_sint8;
        }
        else if (strcmp(str, "uint8") == 0)
        {
            return &ffi_type_uint8;
        }
        else if (strcmp(str, "sint16") == 0)
        {
            return &ffi_type_sint16;
        }
        else if (strcmp(str, "uint16") == 0)
        {
            return &ffi_type_uint16;
        }
        else if (strcmp(str, "sint32") == 0)
        {
            return &ffi_type_sint32;
        }
        else if (strcmp(str, "uint32") == 0)
        {
            return &ffi_type_uint32;
        }
        else if (strcmp(str, "sint64") == 0)
        {
            return &ffi_type_sint64;
        }
        else if (strcmp(str, "uint64") == 0)
        {
            return &ffi_type_uint64;
        }
        else if (strcmp(str, "float") == 0)
        {
            return &ffi_type_float;
        }
        else if (strcmp(str, "double") == 0)
        {
            return &ffi_type_double;
        }
        else if (strcmp(str, "string") == 0)
        {
            return &ffi_type_pointer;
        }
        else if (strcmp(str, "void") == 0)
        {
            return &ffi_type_void;
        }
    }
    else if (json_is_object(type))
    {
    }
    else if (json_is_array(type))
    {
    }
    else
    {
    }

    return &ffi_type_void;
}

static int32_t
record_cvt_by_json(not_record_t *arg, json_t *type, ffi_sized_t *ret)
{
    if (json_is_string(type))
    {
        const char *str = json_string_value(type);
        if (strcmp(str, "schar") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(char));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(char *)ptr = *(char *)arg->value;
                ret->size = sizeof(char);
                ret->type = &ffi_type_schar;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(char));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(char *)ptr = (char)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(char);
                ret->type = &ffi_type_schar;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "uchar") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned char));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned char *)ptr = (unsigned char)(*(char *)arg->value);
                ret->size = sizeof(unsigned char);
                ret->type = &ffi_type_uchar;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned char));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned char *)ptr = (unsigned char)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(unsigned char);
                ret->type = &ffi_type_uchar;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "sshort") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(short));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(short *)ptr = (short)(*(char *)arg->value);
                ret->size = sizeof(short);
                ret->type = &ffi_type_sshort;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(short));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(short *)ptr = (short)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(short);
                ret->type = &ffi_type_sshort;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "ushort") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned short));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned short *)ptr = (unsigned short)(*(char *)arg->value);
                ret->size = sizeof(unsigned short);
                ret->type = &ffi_type_ushort;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned short));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned short *)ptr = (unsigned short)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(unsigned short);
                ret->type = &ffi_type_ushort;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "sint") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(int));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int *)ptr = (int)(*(char *)arg->value);
                ret->size = sizeof(int);
                ret->type = &ffi_type_sint;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(int));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int *)ptr = (int)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int);
                ret->type = &ffi_type_sint;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "uint") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned int));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned int *)ptr = (unsigned int)(*(char *)arg->value);
                ret->size = sizeof(unsigned int);
                ret->type = &ffi_type_uint;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned int));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned int *)ptr = (unsigned int)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(unsigned int);
                ret->type = &ffi_type_uint;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "slong") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(long));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(long *)ptr = (long)(*(char *)arg->value);
                ret->size = sizeof(long);
                ret->type = &ffi_type_slong;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(long));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(long *)ptr = (long)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(long);
                ret->type = &ffi_type_slong;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "ulong") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned long));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned long *)ptr = (unsigned long)(*(char *)arg->value);
                ret->size = sizeof(unsigned long);
                ret->type = &ffi_type_ulong;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(unsigned long));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(unsigned long *)ptr = (unsigned long)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(unsigned long);
                ret->type = &ffi_type_ulong;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "sint8") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(int8_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int8_t *)ptr = (int8_t)(*(char *)arg->value);
                ret->size = sizeof(int8_t);
                ret->type = &ffi_type_sint8;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(int8_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int8_t *)ptr = (int8_t)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int8_t);
                ret->type = &ffi_type_sint8;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "uint8") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint8_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint8_t *)ptr = (uint8_t)(*(char *)arg->value);
                ret->size = sizeof(uint8_t);
                ret->type = &ffi_type_uint8;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint8_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint8_t *)ptr = (uint8_t)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(uint8_t);
                ret->type = &ffi_type_uint8;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "sint16") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(int16_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int16_t *)ptr = (int16_t)(*(char *)arg->value);
                ret->size = sizeof(int16_t);
                ret->type = &ffi_type_sint16;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(int16_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int16_t *)ptr = (int16_t)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int16_t);
                ret->type = &ffi_type_sint16;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "uint16") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint16_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint16_t *)ptr = (uint16_t)(*(char *)arg->value);
                ret->size = sizeof(uint16_t);
                ret->type = &ffi_type_uint16;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint16_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint16_t *)ptr = (uint16_t)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(uint16_t);
                ret->type = &ffi_type_uint16;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "sint32") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(int32_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int32_t *)ptr = (int32_t)(*(char *)arg->value);
                ret->size = sizeof(int32_t);
                ret->type = &ffi_type_sint32;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(int32_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int32_t *)ptr = (int32_t)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int32_t);
                ret->type = &ffi_type_sint32;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "uint32") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint32_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint32_t *)ptr = (uint32_t)(*(char *)arg->value);
                ret->size = sizeof(uint32_t);
                ret->type = &ffi_type_uint32;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint32_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint32_t *)ptr = (uint32_t)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(uint32_t);
                ret->type = &ffi_type_uint32;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "sint64") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(int64_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int64_t *)ptr = (int64_t)(*(char *)arg->value);
                ret->size = sizeof(int64_t);
                ret->type = &ffi_type_sint64;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(int64_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int64_t *)ptr = (int64_t)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int64_t);
                ret->type = &ffi_type_sint64;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "uint64") == 0)
        {
            if (arg->kind == RECORD_KIND_CHAR)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint64_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint64_t *)ptr = (uint64_t)(*(char *)arg->value);
                ret->size = sizeof(uint64_t);
                ret->type = &ffi_type_uint64;
                ret->ptr = ptr;
                return 1;
            }
            else if (arg->kind == RECORD_KIND_INT)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint64_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint64_t *)ptr = (uint64_t)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(uint64_t);
                ret->type = &ffi_type_uint64;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "float") == 0)
        {
            if (arg->kind == RECORD_KIND_FLOAT)
            {
                void *ptr = not_memory_calloc(1, sizeof(float));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(float *)ptr = (float)mpf_get_d(*(mpf_t *)arg->value);
                ret->size = sizeof(float);
                ret->type = &ffi_type_float;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "double") == 0)
        {
            if (arg->kind == RECORD_KIND_FLOAT)
            {
                void *ptr = not_memory_calloc(1, sizeof(double));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(double *)ptr = (double)mpf_get_d(*(mpf_t *)arg->value);
                ret->size = sizeof(double);
                ret->type = &ffi_type_double;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "string") == 0)
        {
            if (arg->kind == RECORD_KIND_STRING)
            {
                char **ptr = not_memory_calloc(1, sizeof(char *));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }

                size_t length = strlen((char *)arg->value);
                ptr[0] = not_memory_calloc(1, length);
                if (!ptr[0])
                {
                    not_error_no_memory();
                    not_memory_free(ptr);
                    return -1;
                }
                memcpy(ptr[0], arg->value, length);
                ret->size = sizeof(char *);
                ret->type = &ffi_type_pointer;
                ret->ptr = ptr;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "mpz") == 0)
        {
            if (arg->kind == RECORD_KIND_INT)
            {
                mpz_t **ptr = not_memory_calloc(1, sizeof(mpz_t *));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }

                mpz_t *num = not_memory_calloc(1, sizeof(mpz_t));
                if (!num)
                {
                    not_error_no_memory();
                    not_memory_free(ptr);
                    return -1;
                }
                mpz_init_set(*num, *(mpz_t *)arg->value);
                ptr[0] = num;

                ret->size = sizeof(mpz_t *);
                ret->type = &ffi_type_pointer;
                ret->ptr = ptr;

                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (strcmp(str, "mpf") == 0)
        {
            if (arg->kind == RECORD_KIND_FLOAT)
            {
                mpf_t **ptr = not_memory_calloc(1, sizeof(mpf_t *));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }

                mpf_t *num = not_memory_calloc(1, sizeof(mpf_t));
                if (!num)
                {
                    not_error_no_memory();
                    not_memory_free(ptr);
                    return -1;
                }
                mpf_init_set(*num, *(mpf_t *)arg->value);
                ptr[0] = num;

                ret->size = sizeof(mpf_t *);
                ret->type = &ffi_type_pointer;
                ret->ptr = ptr;

                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else if (json_is_object(type))
    {
    }
    else if (json_is_array(type))
    {
    }
    else
    {
        if (arg->kind == RECORD_KIND_CHAR)
        {
            void *ptr = not_memory_calloc(1, sizeof(char));
            if (!ptr)
            {
                not_error_no_memory();
                return -1;
            }
            *(char *)ptr = *(char *)arg->value;
            ret->size = sizeof(char);
            ret->type = &ffi_type_schar;
            ret->ptr = ptr;
            return 1;
        }
        else if (arg->kind == RECORD_KIND_INT)
        {
            if (mpz_cmp_si(*(mpz_t *)arg->value, INT32_MIN) >= 0 && mpz_cmp_si(*(mpz_t *)arg->value, INT32_MAX) <= 0)
            {
                void *ptr = not_memory_calloc(1, sizeof(int32_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int32_t *)ptr = (int32_t)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int32_t);
                ret->type = &ffi_type_sint32;
                ret->ptr = ptr;
                return 1;
            }

            // Check uint32_t range
            else if (mpz_cmp_ui(*(mpz_t *)arg->value, 0) >= 0 && mpz_cmp_ui(*(mpz_t *)arg->value, UINT32_MAX) <= 0)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint32_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint32_t *)ptr = (uint32_t)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(uint32_t);
                ret->type = &ffi_type_uint32;
                ret->ptr = ptr;
                return 1;
            }

            // Check int64_t range
            else if (mpz_cmp_si(*(mpz_t *)arg->value, INT64_MIN) >= 0 && mpz_cmp_si(*(mpz_t *)arg->value, INT64_MAX) <= 0)
            {
                void *ptr = not_memory_calloc(1, sizeof(int64_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(int64_t *)ptr = (int64_t)mpz_get_si(*(mpz_t *)arg->value);
                ret->size = sizeof(int64_t);
                ret->type = &ffi_type_sint64;
                ret->ptr = ptr;
                return 1;
            }

            // Check uint64_t range
            else if (mpz_cmp_ui(*(mpz_t *)arg->value, 0) >= 0 && mpz_cmp_ui(*(mpz_t *)arg->value, UINT64_MAX) <= 0)
            {
                void *ptr = not_memory_calloc(1, sizeof(uint64_t));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }
                *(uint64_t *)ptr = (uint64_t)mpz_get_ui(*(mpz_t *)arg->value);
                ret->size = sizeof(uint64_t);
                ret->type = &ffi_type_uint64;
                ret->ptr = ptr;
                return 1;
            }

            else
            {
                mpz_t **ptr = not_memory_calloc(1, sizeof(mpz_t *));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }

                mpz_t *num = not_memory_calloc(1, sizeof(mpz_t));
                if (!num)
                {
                    not_error_no_memory();
                    not_memory_free(ptr);
                    return -1;
                }
                mpz_init_set(*num, *(mpz_t *)arg->value);
                ptr[0] = num;

                ret->size = sizeof(mpz_t *);
                ret->type = &ffi_type_pointer;
                ret->ptr = ptr;

                return 1;
            }
        }
        else if (arg->kind == RECORD_KIND_FLOAT)
        {
            double d_val = mpf_get_d(*(mpf_t *)arg->value);

            // Check float range
            if (d_val >= FLT_MIN && d_val <= FLT_MAX)
            {
                float f_val = (float)d_val;
                mpf_t temp;
                mpf_init_set_d(temp, f_val);
                if (mpf_cmp(*(mpf_t *)arg->value, temp) == 0)
                {
                    void *ptr = not_memory_calloc(1, sizeof(float));
                    if (!ptr)
                    {
                        not_error_no_memory();
                        return -1;
                    }
                    *(float *)ptr = f_val;
                    ret->size = sizeof(float);
                    ret->type = &ffi_type_float;
                    ret->ptr = ptr;
                    return 1;
                }
                else
                {
                    void *ptr = not_memory_calloc(1, sizeof(double));
                    if (!ptr)
                    {
                        not_error_no_memory();
                        return -1;
                    }
                    *(double *)ptr = d_val;
                    ret->size = sizeof(double);
                    ret->type = &ffi_type_double;
                    ret->ptr = ptr;
                    return 1;
                }
                mpf_clear(temp);
            }

            // Check double range
            else if (d_val >= DBL_MIN && d_val <= DBL_MAX)
            {
                mpf_t temp;
                mpf_init_set_d(temp, d_val);
                if (mpf_cmp(*(mpf_t *)arg->value, temp) == 0)
                {
                    void *ptr = not_memory_calloc(1, sizeof(double));
                    if (!ptr)
                    {
                        not_error_no_memory();
                        return -1;
                    }
                    *(double *)ptr = d_val;
                    ret->size = sizeof(double);
                    ret->type = &ffi_type_double;
                    ret->ptr = ptr;
                    return 1;
                }
                else
                {
                    mpf_t **ptr = not_memory_calloc(1, sizeof(mpf_t *));
                    if (!ptr)
                    {
                        not_error_no_memory();
                        return -1;
                    }

                    mpf_t *num = not_memory_calloc(1, sizeof(mpf_t));
                    if (!num)
                    {
                        not_error_no_memory();
                        not_memory_free(ptr);
                        return -1;
                    }
                    mpf_init_set(*num, *(mpf_t *)arg->value);
                    ptr[0] = num;

                    ret->size = sizeof(mpf_t *);
                    ret->type = &ffi_type_pointer;
                    ret->ptr = ptr;

                    return 1;
                }
                mpf_clear(temp);
            }
            else
            {
                mpf_t **ptr = not_memory_calloc(1, sizeof(mpf_t *));
                if (!ptr)
                {
                    not_error_no_memory();
                    return -1;
                }

                mpf_t *num = not_memory_calloc(1, sizeof(mpf_t));
                if (!num)
                {
                    not_error_no_memory();
                    not_memory_free(ptr);
                    return -1;
                }
                mpf_init_set(*num, *(mpf_t *)arg->value);
                ptr[0] = num;

                ret->size = sizeof(mpf_t *);
                ret->type = &ffi_type_pointer;
                ret->ptr = ptr;

                return 1;
            }
        }
        else if (arg->kind == RECORD_KIND_STRING)
        {
            char **ptr = not_memory_calloc(1, sizeof(char *));
            if (!ptr)
            {
                not_error_no_memory();
                return -1;
            }

            size_t length = strlen((char *)arg->value);
            ptr[0] = not_memory_calloc(1, length);
            if (!ptr[0])
            {
                not_error_no_memory();
                not_memory_free(ptr);
                return -1;
            }
            memcpy(ptr[0], arg->value, length);
            ret->size = sizeof(char *);
            ret->type = &ffi_type_pointer;
            ret->ptr = ptr;
            return 1;
        }
    }

    return 0;
}

static not_record_t *
union_cvt_record_by_json(ffi_ret_type *value, json_t *type)
{
    if (json_is_string(type))
    {
        const char *str = json_string_value(type);
        if (strcmp(str, "schar") == 0)
        {
            return not_record_make_char(value->c);
        }
        else if (strcmp(str, "uchar") == 0)
        {
            return not_record_make_int_from_ui(value->uc);
        }
        else if (strcmp(str, "sshort") == 0)
        {
            return not_record_make_int_from_si(value->s);
        }
        else if (strcmp(str, "ushort") == 0)
        {
            return not_record_make_int_from_ui(value->us);
        }
        else if (strcmp(str, "sint") == 0)
        {
            return not_record_make_int_from_si(value->i);
        }
        else if (strcmp(str, "uint") == 0)
        {
            return not_record_make_int_from_ui(value->ui);
        }
        else if (strcmp(str, "slong") == 0)
        {
            return not_record_make_int_from_si(value->l);
        }
        else if (strcmp(str, "ulong") == 0)
        {
            return not_record_make_int_from_ui(value->ul);
        }
        else if (strcmp(str, "sint8") == 0)
        {
            return not_record_make_int_from_si(value->i8);
        }
        else if (strcmp(str, "uint8") == 0)
        {
            return not_record_make_int_from_ui(value->ui8);
        }
        else if (strcmp(str, "sint16") == 0)
        {
            return not_record_make_int_from_si(value->i16);
        }
        else if (strcmp(str, "uint16") == 0)
        {
            return not_record_make_int_from_ui(value->ui16);
        }
        else if (strcmp(str, "sint32") == 0)
        {
            return not_record_make_int_from_si(value->i32);
        }
        else if (strcmp(str, "uint32") == 0)
        {
            return not_record_make_int_from_ui(value->ui32);
        }
        else if (strcmp(str, "sint64") == 0)
        {
            return not_record_make_int_from_si(value->i64);
        }
        else if (strcmp(str, "uint64") == 0)
        {
            return not_record_make_int_from_ui(value->ui64);
        }
        else if (strcmp(str, "float") == 0)
        {
            return not_record_make_float_from_d(value->f);
        }
        else if (strcmp(str, "double") == 0)
        {
            return not_record_make_float_from_d(value->d);
        }
        else if (strcmp(str, "string") == 0)
        {
            return not_record_make_string((char *)value->ptr);
        }
    }
    else if (json_is_object(type))
    {
    }
    else if (json_is_array(type))
    {
    }
    else
    {
    }

    return not_record_make_undefined();
}

static not_record_t *
not_call_ffi(not_node_t *base, not_node_t *arguments, not_strip_t *strip, void *handle, json_t *map, not_node_t *applicant)
{
    size_t arg_count = 0;
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;
        for (not_node_t *item = block->items; item != NULL; item = item->next)
        {
            arg_count += 1;
        }
    }

    json_t *parameters = json_object_get(map, "parameters");

    if (arguments)
    {
        if (!parameters || !json_is_array(parameters))
        {
            not_error_type_by_node(base, "'%s' takes %lu positional arguments but %lu were given", "proc", 0, arg_count);
            return ERROR;
        }
    }

    not_queue_t *repo = not_queue_create();
    if (repo == ERROR)
    {
        return ERROR;
    }

    typedef struct
    {
        const char *key;
        void *ptr;
        ffi_type *type;
    } ffi_repository_entry_t;

    size_t param_count = json_array_size(parameters);
    size_t param_index = 0;
    if (arguments)
    {
        not_node_block_t *block = (not_node_block_t *)arguments->value;
        for (not_node_t *item = block->items; item != NULL;)
        {
            not_node_argument_t *argument = (not_node_argument_t *)item->value;

            if (param_index >= param_count)
            {
                json_t *var_arg = json_object_get(map, "arg");
                if (json_is_boolean(var_arg) && json_boolean_value(var_arg))
                {
                    goto region_enable_vararg;
                }

                if (argument->value)
                {
                    not_node_basic_t *basic2 = (not_node_basic_t *)argument->key->value;
                    not_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", "proc", basic2->value);
                    goto region_cleanup;
                }
                else
                {
                    not_error_type_by_node(argument->key, "'%s' takes %lld positional arguments but %lld were given", "proc", param_count, arg_count);
                    goto region_cleanup;
                }
            }

            json_t *parameter = json_array_get(parameters, param_index);
            json_t *prefix = json_object_get(parameter, "prefix");
            json_t *name = json_object_get(parameter, "name");
            json_t *type = json_object_get(parameter, "type");

            if (argument->value)
            {
                if (json_is_string(prefix) && (strcmp(json_string_value(prefix), "**") == 0))
                {
                    void **ptr = NULL;

                    ffi_type **elements = NULL;
                    size_t index = 0;

                    ffi_type *struct_type = not_memory_calloc(1, sizeof(ffi_type));
                    if (!struct_type)
                    {
                        not_error_no_memory();
                        goto region_cleanup;
                    }

                    for (; item != NULL; item = item->next)
                    {
                        argument = (not_node_argument_t *)item->value;
                        if (!argument->value)
                        {
                            break;
                        }

                        not_record_t *record_arg = not_execute_expression(argument->value, strip, applicant, NULL);
                        if (record_arg == ERROR)
                        {
                            not_error_no_memory();
                            goto region_cleanup_kwarg;
                        }

                        ffi_sized_t cvt_ffi;
                        int32_t r = record_cvt_by_json(record_arg, type, &cvt_ffi);
                        if (r < 0)
                        {
                            not_record_link_decrease(record_arg);
                            goto region_cleanup_kwarg;
                        }
                        else if (r == 0)
                        {
                            not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                   "argument", not_record_type_as_string(record_arg),
                                                   json_is_string(type) ? json_string_value(type) : "struct");
                            not_record_link_decrease(record_arg);
                            goto region_cleanup_kwarg;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            if (cvt_ffi.type)
                                ffi_type_destroy(cvt_ffi.type);

                            if (cvt_ffi.ptr)
                                not_memory_free(cvt_ffi.ptr);

                            goto region_cleanup_kwarg;
                        }

                        if (index == 0)
                        {
                            ffi_type **elements_ptr = not_memory_calloc(1, sizeof(ffi_type *) * (index + 2));
                            if (!elements_ptr)
                            {
                                not_error_no_memory();

                                if (cvt_ffi.type)
                                    ffi_type_destroy(cvt_ffi.type);

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                goto region_cleanup_kwarg;
                            }

                            elements_ptr[index] = cvt_ffi.type;
                            elements = elements_ptr;

                            void **ptr_copy = not_memory_calloc(1, sizeof(void *) * (index + 2));
                            if (!ptr_copy)
                            {
                                not_error_no_memory();

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                goto region_cleanup_kwarg;
                            }

                            ptr_copy[index] = cvt_ffi.ptr;
                            ptr = ptr_copy;

                            index += 1;
                        }
                        else
                        {
                            ffi_type **elements_ptr = not_memory_realloc(elements, sizeof(ffi_type *) * (index + 2));
                            if (!elements_ptr)
                            {
                                not_error_no_memory();

                                if (cvt_ffi.type)
                                    ffi_type_destroy(cvt_ffi.type);

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                goto region_cleanup_kwarg;
                            }

                            elements_ptr[index] = cvt_ffi.type;
                            elements = elements_ptr;

                            void **ptr_copy = not_memory_realloc(ptr, sizeof(void *) * (index + 2));
                            if (!ptr_copy)
                            {
                                not_error_no_memory();

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                goto region_cleanup_kwarg;
                            }

                            ptr_copy[index] = cvt_ffi.ptr;
                            ptr = ptr_copy;

                            index += 1;
                        }

                        continue;
                    }

                    elements[index] = NULL;

                    struct_type->size = 0;
                    struct_type->alignment = 0;
                    struct_type->type = FFI_TYPE_STRUCT;
                    struct_type->elements = elements;

                    ffi_repository_entry_t *entry = not_memory_calloc(1, sizeof(ffi_repository_entry_t));
                    if (!entry)
                    {
                        not_error_no_memory();
                        goto region_cleanup_kwarg;
                    }

                    entry->key = json_string_value(name);
                    entry->type = struct_type;
                    entry->ptr = ptr;

                    if (ERROR == not_queue_right_push(repo, entry))
                    {
                        not_memory_free(entry);
                        goto region_cleanup_kwarg;
                    }

                    param_index += 1;
                    continue;

                region_cleanup_kwarg:
                    if (ptr)
                        not_memory_free(ptr);

                    for (size_t i = 0; i < index; i++)
                    {
                        ffi_type_destroy(elements[i]);
                    }

                    if (struct_type)
                        not_memory_free(struct_type);

                    goto region_cleanup;
                }
                else
                {
                    int8_t found = 0;
                    for (size_t param_index2 = param_index; param_index2 < param_count; param_index2 += 1)
                    {
                        parameter = json_array_get(parameters, param_index2);
                        name = json_object_get(parameter, "name");
                        type = json_object_get(parameter, "type");

                        if (not_execute_id_strcmp(argument->key, json_string_value(name)) == 1)
                        {
                            found = 1;

                            not_record_t *record_arg = not_execute_expression(argument->value, strip, applicant, NULL);
                            if (record_arg == ERROR)
                            {
                                not_error_no_memory();
                                goto region_cleanup;
                            }

                            ffi_sized_t cvt_ffi;
                            int32_t r = record_cvt_by_json(record_arg, type, &cvt_ffi);
                            if (r < 0)
                            {
                                not_record_link_decrease(record_arg);
                                goto region_cleanup;
                            }
                            else if (r == 0)
                            {
                                not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                       "argument", not_record_type_as_string(record_arg),
                                                       json_is_string(type) ? json_string_value(type) : "struct");
                                not_record_link_decrease(record_arg);
                                goto region_cleanup;
                            }

                            if (not_record_link_decrease(record_arg) < 0)
                            {
                                if (cvt_ffi.type)
                                    ffi_type_destroy(cvt_ffi.type);

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                goto region_cleanup;
                            }

                            ffi_repository_entry_t *entry = not_memory_calloc(1, sizeof(ffi_repository_entry_t));
                            if (!entry)
                            {
                                not_error_no_memory();

                                if (cvt_ffi.type)
                                    ffi_type_destroy(cvt_ffi.type);

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                goto region_cleanup;
                            }

                            entry->key = json_string_value(name);
                            entry->type = cvt_ffi.type;
                            entry->ptr = cvt_ffi.ptr;

                            if (ERROR == not_queue_right_push(repo, entry))
                            {
                                if (cvt_ffi.type)
                                    ffi_type_destroy(cvt_ffi.type);

                                if (cvt_ffi.ptr)
                                    not_memory_free(cvt_ffi.ptr);

                                not_memory_free(entry);
                                goto region_cleanup;
                            }

                            break;
                        }
                    }

                    if (found == 0)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)argument->key->value;
                        not_error_type_by_node(argument->key, "'%s' got an unexpected keyword argument '%s'", "proc", basic1->value);
                        goto region_cleanup;
                    }

                    continue;
                }
            }
            else
            {
                if (json_is_string(prefix) && (strcmp(json_string_value(prefix), "*") == 0))
                {
                    void **ptr = NULL;

                    ffi_type **elements = NULL;
                    size_t index = 0;

                    ffi_type *struct_type = not_memory_calloc(1, sizeof(ffi_type));
                    if (!struct_type)
                    {
                        not_error_no_memory();
                        goto region_cleanup;
                    }

                    for (; item != NULL; item = item->next)
                    {
                        argument = (not_node_argument_t *)item->value;
                        if (argument->value)
                        {
                            break;
                        }

                        not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, NULL);
                        if (record_arg == ERROR)
                        {
                            not_error_no_memory();
                            goto region_cleanup_karg;
                        }

                        ffi_sized_t cvt_ffi;
                        int32_t r = record_cvt_by_json(record_arg, type, &cvt_ffi);
                        if (r < 0)
                        {
                            not_record_link_decrease(record_arg);
                            goto region_cleanup_karg;
                        }
                        else if (r == 0)
                        {
                            not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                                   "argument", not_record_type_as_string(record_arg),
                                                   json_is_string(type) ? json_string_value(type) : "struct");
                            not_record_link_decrease(record_arg);
                            goto region_cleanup_karg;
                        }

                        if (not_record_link_decrease(record_arg) < 0)
                        {
                            if (cvt_ffi.type)
                                ffi_type_destroy(cvt_ffi.type);

                            if (cvt_ffi.ptr)
                                not_memory_free(cvt_ffi.ptr);

                            goto region_cleanup_karg;
                        }

                        if (index == 0)
                        {
                            ffi_type **elements_ptr = not_memory_calloc(1, sizeof(ffi_type *) * (index + 2));
                            if (!elements_ptr)
                            {
                                not_error_no_memory();
                                goto region_cleanup_karg;
                            }

                            elements_ptr[index] = cvt_ffi.type;
                            elements = elements_ptr;

                            void **ptr_copy = not_memory_calloc(1, sizeof(void *) * (index + 2));
                            if (!ptr_copy)
                            {
                                not_error_no_memory();
                                goto region_cleanup_karg;
                            }

                            ptr_copy[index] = cvt_ffi.ptr;
                            ptr = ptr_copy;

                            index += 1;
                        }
                        else
                        {
                            ffi_type **elements_ptr = not_memory_realloc(elements, sizeof(ffi_type *) * (index + 2));
                            if (!elements_ptr)
                            {
                                not_error_no_memory();
                                goto region_cleanup_karg;
                            }

                            elements_ptr[index] = cvt_ffi.type;
                            elements = elements_ptr;

                            void **ptr_copy = not_memory_realloc(ptr, sizeof(void *) * (index + 2));
                            if (!ptr_copy)
                            {
                                not_error_no_memory();
                                goto region_cleanup_karg;
                            }

                            ptr_copy[index] = cvt_ffi.ptr;
                            ptr = ptr_copy;

                            index += 1;
                        }

                        continue;
                    }

                    elements[index] = NULL;
                    ptr[index] = NULL;

                    struct_type->size = 0;
                    struct_type->alignment = 0;
                    struct_type->type = FFI_TYPE_STRUCT;
                    struct_type->elements = elements;

                    ffi_repository_entry_t *entry = not_memory_calloc(1, sizeof(ffi_repository_entry_t));
                    if (!entry)
                    {
                        not_error_no_memory();
                        goto region_cleanup_karg;
                    }

                    entry->key = json_string_value(name);
                    entry->type = struct_type;
                    entry->ptr = ptr;

                    if (ERROR == not_queue_right_push(repo, entry))
                    {
                        not_memory_free(entry);
                        goto region_cleanup_karg;
                    }

                    param_index += 1;
                    continue;

                region_cleanup_karg:
                    if (ptr)
                        not_memory_free(ptr);

                    for (size_t i = 0; i < index; i++)
                    {
                        ffi_type_destroy(elements[i]);
                    }

                    if (struct_type)
                        not_memory_free(struct_type);

                    goto region_cleanup;
                }
                else if (json_is_string(prefix) && (strcmp(json_string_value(prefix), "**") == 0))
                {
                    not_error_type_by_node(argument->key, "'%s' required '%s'", "json", "kwarg");
                    goto region_cleanup;
                }
                else
                {
                    not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, NULL);
                    if (record_arg == ERROR)
                    {
                        not_error_no_memory();
                        goto region_cleanup;
                    }

                    ffi_sized_t cvt_ffi;
                    int32_t r = record_cvt_by_json(record_arg, type, &cvt_ffi);
                    if (r < 0)
                    {
                        not_record_link_decrease(record_arg);
                        goto region_cleanup;
                    }
                    else if (r == 0)
                    {
                        not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                               "argument", not_record_type_as_string(record_arg),
                                               json_is_string(type) ? json_string_value(type) : "struct|tuple");
                        not_record_link_decrease(record_arg);
                        goto region_cleanup;
                    }

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        if (cvt_ffi.type)
                            ffi_type_destroy(cvt_ffi.type);

                        if (cvt_ffi.ptr)
                            not_memory_free(cvt_ffi.ptr);

                        goto region_cleanup;
                    }

                    ffi_repository_entry_t *entry = not_memory_calloc(1, sizeof(ffi_repository_entry_t));
                    if (!entry)
                    {
                        not_error_no_memory();

                        if (cvt_ffi.type)
                            ffi_type_destroy(cvt_ffi.type);

                        if (cvt_ffi.ptr)
                            not_memory_free(cvt_ffi.ptr);

                        goto region_cleanup;
                    }

                    entry->key = json_string_value(name);
                    entry->type = cvt_ffi.type;
                    entry->ptr = cvt_ffi.ptr;

                    if (ERROR == not_queue_right_push(repo, entry))
                    {
                        if (cvt_ffi.type)
                            ffi_type_destroy(cvt_ffi.type);

                        if (cvt_ffi.ptr)
                            not_memory_free(cvt_ffi.ptr);

                        not_memory_free(entry);
                        goto region_cleanup;
                    }

                    param_index += 1;

                    item = item->next;
                    continue;
                }
            }

            item = item->next;
            continue;

        region_enable_vararg:
            not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, NULL);
            if (record_arg == ERROR)
            {
                not_error_no_memory();
                goto region_cleanup;
            }

            json_t *type2 = json_object_get(map, "arg");

            ffi_sized_t cvt_ffi;
            int32_t r = record_cvt_by_json(record_arg, type2, &cvt_ffi);
            if (r < 0)
            {
                not_record_link_decrease(record_arg);
                goto region_cleanup;
            }
            else if (r == 0)
            {
                not_error_type_by_node(argument->key, "'%s' mismatch: '%s' and '%s'",
                                       "argument", not_record_type_as_string(record_arg),
                                       json_is_string(type2) ? json_string_value(type2) : "struct");
                not_record_link_decrease(record_arg);
                goto region_cleanup;
            }

            if (not_record_link_decrease(record_arg) < 0)
            {
                if (cvt_ffi.type)
                    ffi_type_destroy(cvt_ffi.type);

                if (cvt_ffi.ptr)
                    not_memory_free(cvt_ffi.ptr);

                goto region_cleanup;
            }

            ffi_repository_entry_t *entry = not_memory_calloc(1, sizeof(ffi_repository_entry_t));
            if (!entry)
            {
                not_error_no_memory();

                if (cvt_ffi.type)
                    ffi_type_destroy(cvt_ffi.type);

                if (cvt_ffi.ptr)
                    not_memory_free(cvt_ffi.ptr);

                goto region_cleanup;
            }

            entry->key = "";
            entry->type = cvt_ffi.type;
            entry->ptr = cvt_ffi.ptr;

            if (ERROR == not_queue_right_push(repo, entry))
            {
                if (cvt_ffi.type)
                    ffi_type_destroy(cvt_ffi.type);

                if (cvt_ffi.ptr)
                    not_memory_free(cvt_ffi.ptr);

                not_memory_free(entry);
                goto region_cleanup;
            }

            item = item->next;
            continue;
        }
    }

    if (param_index < param_count)
    {
        not_error_type_by_node(base, "'%s' missing '%lld' required positional argument", "proc", (param_count - 1) - param_index);
        goto region_cleanup;
    }

    size_t index = 0, var_index = 0, total = (size_t)not_queue_count(repo);

    ffi_type **args = not_memory_calloc(total, sizeof(ffi_type *));
    if (!args)
    {
        goto region_cleanup;
    }
    void **values = not_memory_calloc(total, sizeof(void *));
    if (!values)
    {
        not_memory_free(args);
        goto region_cleanup;
    }

    for (size_t i = 0; i < param_count; i++)
    {
        json_t *parameter = json_array_get(parameters, i);
        json_t *name = json_object_get(parameter, "name");

        for (not_queue_entry_t *item = repo->begin; item != repo->end; item = item->next)
        {
            ffi_repository_entry_t *entry = (ffi_repository_entry_t *)item->value;

            if (strcmp(entry->key, json_string_value(name)) == 0)
            {
                args[index] = entry->type;
                values[index] = entry->ptr;
                index += 1;
                break;
            }
        }
    }

    for (not_queue_entry_t *item = repo->begin; item != repo->end; item = item->next)
    {
        ffi_repository_entry_t *entry = (ffi_repository_entry_t *)item->value;

        if (strcmp(entry->key, "") == 0)
        {
            args[index] = entry->type;
            values[index] = entry->ptr;

            index += 1;
            var_index += 1;
        }
    }

    json_t *return_type = json_object_get(map, "return_type");

    ffi_type *ret_arg = json_to_ffi(return_type);
    if (ret_arg == ERROR)
    {
        not_memory_free(args);
        not_memory_free(values);
        goto region_cleanup;
    }

    ffi_ret_type result;

    ffi_cif cif;
    if (ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, index - var_index, index, ret_arg, args) == FFI_OK)
    {
        ffi_call(&cif, FFI_FN(handle), &result, values);
        goto region_result;
    }
    else
    {
        not_error_system("ffi_prep_cif failed");
        goto region_cleanup;
    }

region_result:
    for (not_queue_entry_t *item = repo->begin; item != repo->end; item = item->next)
    {
        ffi_repository_entry_t *entry = (ffi_repository_entry_t *)item->value;
        if (entry->ptr)
            not_memory_free(entry->ptr);

        if (entry->type)
            ffi_type_destroy(entry->type);
    }

    not_queue_destroy(repo);

    return union_cvt_record_by_json(&result, return_type);

region_cleanup:
    for (not_queue_entry_t *item = repo->begin; item != repo->end; item = item->next)
    {
        ffi_repository_entry_t *entry = (ffi_repository_entry_t *)item->value;
        if (entry->ptr)
            not_memory_free(entry->ptr);

        if (entry->type)
            ffi_type_destroy(entry->type);
    }

    not_queue_destroy(repo);

    return ERROR;
}

not_record_t *
not_call(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_carrier_t *carrier = (not_node_carrier_t *)node->value;

    not_record_t *record_base = not_execute_expression(carrier->base, strip, applicant, origin);
    if (record_base == ERROR)
    {
        return ERROR;
    }

    if (record_base->kind == RECORD_KIND_TYPE)
    {
        not_record_type_t *record_type = (not_record_type_t *)record_base->value;
        not_node_t *type = record_type->type;

        if (type->kind == NODE_KIND_CLASS)
        {
            not_strip_t *strip_new = (not_strip_t *)record_type->value;
            not_record_t *result = not_call_class(node, carrier->data, strip_new, type, applicant);
            if (not_record_link_decrease(record_base) < 0)
            {
                return ERROR;
            }

            if (result == ERROR)
            {
                return ERROR;
            }
            return result;
        }
        else if (type->kind == NODE_KIND_FUN)
        {
            not_strip_t *strip_new = (not_strip_t *)record_type->value;
            not_record_t *result = not_call_fun(node, carrier->data, strip_new, type, applicant);

            if (not_record_link_decrease(record_base) < 0)
            {
                return ERROR;
            }

            if (result == ERROR)
            {
                return ERROR;
            }

            return result;
        }
        else if (type->kind == NODE_KIND_LAMBDA)
        {
            not_strip_t *strip_new = (not_strip_t *)record_type->value;
            not_record_t *result = not_call_lambda(node, carrier->data, strip_new, type, applicant);

            if (not_record_link_decrease(record_base) < 0)
            {
                return ERROR;
            }

            if (result == ERROR)
            {
                return ERROR;
            }

            return result;
        }
        else if (type->kind == NODE_KIND_KCHAR)
        {
            if (!carrier->data)
            {
                not_record_t *result = not_record_make_char(0);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }

            not_node_block_t *block = (not_node_block_t *)carrier->data->value;
            not_node_argument_t *argument = (not_node_argument_t *)block->items->value;
            if (argument->value)
            {
                not_error_type_by_node(argument->key, "'%s' not support", "pair");
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            not_record_t *record_value = not_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_CHAR)
            {
                not_record_t *result = not_record_make_char((char)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_INT)
            {
                not_record_t *result = not_record_make_char((char)mpz_get_si(*(mpz_t *)record_value->value));
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_FLOAT)
            {
                not_record_t *result = not_record_make_char((char)mpf_get_si(*(mpf_t *)record_value->value));
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else
            {
                not_error_type_by_node(carrier->base, "'%s' object is not castable",
                                       not_record_type_as_string(record_base));

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }
        }
        else if (type->kind == NODE_KIND_KINT)
        {
            if (!carrier->data)
            {
                not_record_t *result = not_record_make_int_from_si(0);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }

            not_node_block_t *block = (not_node_block_t *)carrier->data->value;
            not_node_argument_t *argument = (not_node_argument_t *)block->items->value;
            if (argument->value)
            {
                not_error_type_by_node(argument->key, "'%s' not support", "pair");
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            not_record_t *record_value = not_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_CHAR)
            {
                not_record_t *result = not_record_make_int_from_si((int64_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_INT)
            {
                not_record_t *result = not_record_make_int_from_z(*(mpz_t *)record_value->value);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_FLOAT)
            {
                not_record_t *result = not_record_make_int_from_f(*(mpf_t *)record_value->value);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else
            {
                not_error_type_by_node(carrier->base, "'%s' object is not castable",
                                       not_record_type_as_string(record_base));

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }
        }
        else if (type->kind == NODE_KIND_KFLOAT)
        {
            if (!carrier->data)
            {
                not_record_t *result = not_record_make_float_from_d(0.0);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }

            not_node_block_t *block = (not_node_block_t *)carrier->data->value;
            not_node_argument_t *argument = (not_node_argument_t *)block->items->value;
            if (argument->value)
            {
                not_error_type_by_node(argument->key, "'%s' not support", "pair");
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            not_record_t *record_value = not_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            if (record_value->kind == RECORD_KIND_INT)
            {
                not_record_t *result = not_record_make_float_from_si((int64_t)(*(int8_t *)record_value->value));
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_CHAR)
            {
                not_record_t *result = not_record_make_float_from_si((int64_t)(*(char *)record_value->value));
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_INT)
            {
                not_record_t *result = not_record_make_int_from_z(*(mpz_t *)record_value->value);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else if (record_value->kind == RECORD_KIND_FLOAT)
            {
                not_record_t *result = not_record_make_float_from_f(*(mpf_t *)record_value->value);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_value) < 0)
                    {
                        return ERROR;
                    }

                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else
            {
                not_error_type_by_node(carrier->base, "'%s' object is not castable",
                                       not_record_type_as_string(record_base));

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }
        }
        else if (type->kind == NODE_KIND_KSTRING)
        {
            if (!carrier->data)
            {
                not_record_t *result = not_record_make_string("");
                if (result == ERROR)
                {
                    if (not_record_link_decrease(record_base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }

                return result;
            }

            not_node_block_t *block = (not_node_block_t *)carrier->data->value;
            not_node_argument_t *argument = (not_node_argument_t *)block->items->value;
            if (argument->value)
            {
                not_error_type_by_node(argument->key, "'%s' not support", "pair");
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
            not_record_t *record_value = not_execute_expression(argument->key, strip, applicant, NULL);
            if (record_value == ERROR)
            {
                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            char *str = not_record_to_string(record_value, "");
            // printf("%s\n", str);
            not_record_t *result = not_record_make_string(str);
            not_memory_free(str);
            if (result == ERROR)
            {
                if (not_record_link_decrease(record_value) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(record_base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            if (not_record_link_decrease(record_value) < 0)
            {
                return ERROR;
            }

            if (not_record_link_decrease(record_base) < 0)
            {
                return ERROR;
            }

            return result;
        }
        else
        {
            not_error_type_by_node(carrier->base, "'%s' object is not callable",
                                   not_record_type_as_string(record_base));

            if (not_record_link_decrease(record_base) < 0)
            {
                return ERROR;
            }

            return ERROR;
        }
    }
    else if (record_base->kind == RECORD_KIND_PROC)
    {
        not_record_proc_t *record_proc = (not_record_proc_t *)record_base->value;

        not_record_t *result = not_call_ffi(node, carrier->data, strip, record_proc->handle, record_proc->map, applicant);
        if (not_record_link_decrease(record_base) < 0)
        {
            return ERROR;
        }

        if (result == ERROR)
        {
            return ERROR;
        }

        return result;
    }

    not_error_type_by_node(carrier->base, "'%s' object is not callable",
                           not_record_type_as_string(record_base));

    if (not_record_link_decrease(record_base) < 0)
    {
        return ERROR;
    }

    return ERROR;
}