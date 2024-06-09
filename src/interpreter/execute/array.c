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
#include "../../config.h"
#include "../../interpreter.h"
#include "../../thread.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

static sy_record_t *
call_for_bracket(sy_node_t *base, sy_node_t *arguments, sy_strip_t *strip, sy_node_t *node, sy_node_t *applicant)
{    
    sy_node_class_t *class1 = (sy_node_class_t *)node->value;

    for (sy_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
            if (sy_execute_id_strcmp(fun1->key, "[]") == 1)
            {
                sy_strip_t *strip_copy = sy_strip_copy(strip);
                if (strip_copy == ERROR)
                {
                    return ERROR;
                }

                sy_strip_t *strip_fun = sy_strip_create(strip_copy);
                if (strip_fun == ERROR)
                {
                    return ERROR;
                }

                if (sy_execute_parameters_substitute(base, item, strip_fun, fun1->parameters, arguments, applicant) < 0)
                {
                    if (sy_strip_destroy(strip_fun) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                int32_t r1 = sy_execute_run_fun(item, strip_fun, applicant);
                if (r1 == -1)
                {
                    if (sy_strip_destroy(strip_fun) < 0)
                    {
                        return ERROR;
                    }

                    return ERROR;
                }

                if (sy_strip_destroy(strip_fun) < 0)
                {
                    return ERROR;
                }

                sy_record_t *rax = sy_thread_get_and_set_rax(NULL);
                if (rax == ERROR)
                {
                    return ERROR;
                }
                else
                if (!rax)
                {
                    rax = sy_record_make_undefined();
                    if (rax == ERROR)
                    {
                        return ERROR;
                    }
                }

                return rax;
            }
        }
    }

    sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
    sy_error_type_by_node(base, "'%s' no operator [] was found", basic1->value);
    return ERROR;
}

static int32_t
is_integer(sy_record_t *value)
{
    if (value->kind == RECORD_KIND_INT8)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_INT16)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_INT32)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_INT64)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_UINT8)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_UINT16)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_UINT32)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_UINT64)
    {
        return 1;
    }
    else
    if (value->kind == RECORD_KIND_BIGINT)
    {
        return 1;
    }

    return 0;
}

static void
record_to_mpz(sy_record_t *value, mpz_t num)
{
    if (value->kind == RECORD_KIND_INT8)
    {
        mpz_set_si(num, *(int8_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_INT16)
    {
        mpz_set_si(num, *(int16_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_INT32)
    {
        mpz_set_si(num, *(int32_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_INT64)
    {
        mpz_set_si(num, *(int64_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_UINT8)
    {
        mpz_set_ui(num, *(uint8_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_UINT16)
    {
        mpz_set_ui(num, *(uint16_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_UINT32)
    {
        mpz_set_ui(num, *(uint32_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_UINT64)
    {
        mpz_set_ui(num, *(uint64_t *)value->value);
        return;
    }
    else
    if (value->kind == RECORD_KIND_BIGINT)
    {
        mpz_set(num, *(mpz_t *)value->value);
        return;
    }
}


sy_record_t *
sy_execute_array(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_carrier_t *carrier = (sy_node_carrier_t *)node->value;

    sy_record_t *base = sy_execute_expression(carrier->base, strip, applicant, origin);
    if (base == ERROR)
    {
        return ERROR;
    }

    if (carrier->data)
    {
        if (base->kind == RECORD_KIND_STRUCT)
        {
            sy_record_struct_t *record_struct = (sy_record_struct_t *)base->value;
            sy_node_t *type = record_struct->type;

            sy_strip_t *strip_new = (sy_strip_t *)record_struct->value;
            sy_record_t *result = call_for_bracket(node, carrier->data, strip_new, type, applicant);

            base->link -= 1;

            if (result == ERROR)
            {
                return ERROR;
            }

            return result;
        }
        else
        if (base->kind == RECORD_KIND_OBJECT)
        {
            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;

            uint64_t arg_cnt = 0;
            for (sy_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;
            }

            if ((arg_cnt > 1) || (arg_cnt == 0))
            {
                sy_error_type_by_node(node, "'%s' takes %lld positional arguments but %lld were given", 
                    sy_record_type_as_string(base), 1, arg_cnt);

                base->link -= 1;

                return ERROR;
            }

            sy_node_t *item1 = block->items;
            sy_node_argument_t *argument = (sy_node_argument_t *)item1->value;

            if (argument->value)
            {
                sy_error_type_by_node(item1, "'%s' not support", "pair");
                base->link -= 1;
                return ERROR;
            }

            sy_record_t *record_arg = sy_execute_expression(argument->key, strip, applicant, origin);
            if (record_arg == ERROR)
            {
                base->link -= 1;
                return ERROR;
            }

            if (record_arg->kind != RECORD_KIND_STRING)
            {
                sy_error_type_by_node(item1, "'%s' must be of '%s' type", "key", "string");

                record_arg->link -= 1;
                base->link -= 1;

                return ERROR;
            }

            for (sy_record_object_t *item = (sy_record_object_t *)base->value;item != NULL;item = item->next)
            {
                if (sy_execute_id_strcmp(item->key, *(char **)record_arg->value) == 1)
                {
                    record_arg->link -= 1;
                    base->link -= 1;
                    return item->value;
                }
            }

            sy_error_type_by_node(node, "'%s' has no contain key '%s'", 
                sy_record_type_as_string(base), *(char **)record_arg->value);

            record_arg->link -= 1;
            base->link -= 1;

            return ERROR;
        }
        else
        if (base->kind == RECORD_KIND_TUPLE)
        {
            sy_node_block_t *block = (sy_node_block_t *)carrier->data->value;

            uint64_t arg_cnt = 0;
            for (sy_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;
            }

            if ((arg_cnt > 3) || (arg_cnt == 0))
            {
                sy_error_type_by_node(node, "'%s' takes %lld positional arguments but %lld were given", 
                    sy_record_type_as_string(base), 3, arg_cnt);

                base->link -= 1;

                return ERROR;
            }
    
            mpz_t start, stop, step;
            mpz_init(start);
            mpz_init(stop);
            mpz_init(step);

            mpz_set_si(start, 0);
            mpz_set_si(stop, 0);
            mpz_set_si(step, 1);

            arg_cnt = 0;
            for (sy_node_t *item = block->items;item != NULL;item = item->next)
            {
                arg_cnt += 1;

                sy_node_argument_t *argument = (sy_node_argument_t *)item->value;

                if (argument->value)
                {
                    mpz_clears(start, stop, step);
                    sy_error_type_by_node(item, "'%s' not support", "pair");
                    base->link -= 1;
                    return ERROR;
                }

                sy_record_t *record_arg = sy_execute_expression(argument->key, strip, applicant, origin);
                if (record_arg == ERROR)
                {
                    mpz_clears(start, stop, step);
                    base->link -= 1;
                    return ERROR;
                }

                if (!is_integer(record_arg))
                {
                    mpz_clears(start, stop, step);
                    sy_error_type_by_node(item, "'%s' must be of '%s' type", "key", "int");

                    record_arg->link -= 1;
                    base->link -= 1;

                    return ERROR;
                }

                if (arg_cnt == 1)
                {
                    record_to_mpz(record_arg, start);
                }
                else
                if (arg_cnt == 2)
                {
                    record_to_mpz(record_arg, stop);
                }
                else
                if (arg_cnt == 3)
                {
                    record_to_mpz(record_arg, step);
                }
            }
            
            mpz_t length, cnt, term;
            mpz_init(length);
            mpz_init(cnt);
            mpz_init(term);

            mpz_set_si(length, 0);
            mpz_set_si(term, 0);
            mpz_set_si(cnt, 0);

            for (sy_record_tuple_t *item = (sy_record_tuple_t *)base->value;item != NULL;item = item->next)
            {
                mpz_add_ui(length, length, 1);
            }

            if (arg_cnt > 1)
            {
                if (mpz_cmp(start, length) >= 0)
                {
                    sy_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    base->link -= 1;
                    return ERROR;
                }

                if (mpz_cmp(stop, length) >= 0)
                {
                    sy_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    base->link -= 1;
                    return ERROR;
                }

                while (mpz_cmp_si(start, 0) < 0)
                {
                    mpz_add(start, length, start);
                }

                while (mpz_cmp_si(stop, 0) < 0)
                {
                    mpz_add(stop, length, stop);
                }

                mpz_set(term, start);

                sy_record_tuple_t *top = NULL, *declaration = NULL;
                if (mpz_cmp(start, stop) <= 0)
                {
                    while ((mpz_cmp(term ,start) >= 0) && (mpz_cmp(term ,stop) <= 0))
                    {
                        mpz_set_si(cnt, 0);
                        for (sy_record_tuple_t *item = (sy_record_tuple_t *)base->value;item != NULL;item = item->next)
                        {
                            if (mpz_cmp(term, cnt) == 0)
                            {
                                sy_record_tuple_t *tuple = sy_record_make_tuple(item->value, NULL);
                                if (tuple == ERROR)
                                {
                                    item->value->link -= 1;
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);

                                    if (top)
                                    {
                                        if (sy_record_tuple_destroy(top) < 0)
                                        {
                                            base->link -= 1;
                                            return ERROR;
                                        }
                                    }

                                    base->link -= 1;
                                    return ERROR;
                                }

                                if (declaration == NULL)
                                {
                                    declaration = tuple;
                                    top = tuple;
                                }
                                else
                                {
                                    declaration->next = tuple;
                                    declaration = tuple;
                                }

                                break;
                            }
                            mpz_add_ui(cnt, cnt, 1);
                        }
                        mpz_add(term, term, step);
                    }
                }
                else
                {
                    while ((mpz_cmp(term ,stop) >= 0) && (mpz_cmp(term ,start) <= 0))
                    {
                        mpz_set_si(cnt, 0);
                        for (sy_record_tuple_t *item = (sy_record_tuple_t *)base->value;item != NULL;item = item->next)
                        {
                            if (mpz_cmp(term, cnt) == 0)
                            {
                                sy_record_tuple_t *tuple = sy_record_make_tuple(item->value, NULL);
                                if (tuple == ERROR)
                                {
                                    item->value->link -= 1;
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);

                                    if (top)
                                    {
                                        if (sy_record_tuple_destroy(top) < 0)
                                        {
                                            base->link -= 1;
                                            return ERROR;
                                        }
                                    }

                                    base->link -= 1;
                                    return ERROR;
                                }

                                if (declaration == NULL)
                                {
                                    declaration = tuple;
                                    top = tuple;
                                }
                                else
                                {
                                    declaration->next = tuple;
                                    declaration = tuple;
                                }

                                break;
                            }
                            mpz_add_ui(cnt, cnt, 1);
                        }
                        mpz_add(term, term, step);
                    }
                }

                mpz_clear(start);
                mpz_clear(stop);
                mpz_clear(step);
                mpz_clear(term);
                mpz_clear(cnt);
                mpz_clear(length);

                sy_record_t *result = sy_record_create(NODE_KIND_TUPLE, top);
                if (result == ERROR)
                {
                    base->link -= 1;
                    return ERROR;
                }

                base->link -= 1;

                return result;
            }
            else
            {
                if (mpz_cmp(start, length) >= 0)
                {
                    sy_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    base->link -= 1;
                    return ERROR;
                }

                while (mpz_cmp_si(start, 0) < 0)
                {
                    mpz_add(start, length, start);
                }

                mpz_set(term, start);
                for (sy_record_tuple_t *item = (sy_record_tuple_t *)base->value;item != NULL;item = item->next)
                {
                    if (mpz_cmp(cnt, term) == 0)
                    {
                        mpz_clear(start);
                        mpz_clear(stop);
                        mpz_clear(step);
                        mpz_clear(term);
                        mpz_clear(cnt);
                        mpz_clear(length);
                        base->link -= 1;
                        return item->value;
                    }
                    mpz_add_ui(cnt, cnt, 1);
                }
            }
        }
    }

    sy_error_type_by_node(node, "array implement not support");
    return ERROR;
}