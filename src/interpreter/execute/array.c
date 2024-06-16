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
#include "../../config.h"
#include "../../interpreter.h"
#include "../../thread.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

static not_record_t *
not_call_for_bracket(not_node_t *base, not_node_t *arguments, not_strip_t *strip, not_node_t *node, not_node_t *applicant)
{
    not_node_class_t *class1 = (not_node_class_t *)node->value;

    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
            if (not_execute_id_strcmp(fun1->key, "[]") == 1)
            {
                not_strip_t *strip_copy = not_strip_copy(strip);
                if (strip_copy == ERROR)
                {
                    return ERROR;
                }

                not_strip_t *strip_fun = not_strip_create(strip_copy);
                if (strip_fun == ERROR)
                {
                    return ERROR;
                }

                if (not_call_parameters_subs(base, item, strip_fun, fun1->parameters, arguments, applicant) < 0)
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
                    if (rax == ERROR)
                    {
                        return ERROR;
                    }
                }

                return rax;
            }
        }
    }

    not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
    not_error_type_by_node(base, "'%s' no operator [] was found", basic1->value);
    return ERROR;
}

static void
record_to_mpz(not_record_t *value, mpz_t num)
{
    if (value->kind == RECORD_KIND_INT)
    {
        mpz_set(num, *(mpz_t *)value->value);
        return;
    }
}

not_record_t *
not_execute_array(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    not_node_carrier_t *carrier = (not_node_carrier_t *)node->value;

    not_record_t *base = not_execute_expression(carrier->base, strip, applicant, origin);
    if (base == ERROR)
    {
        return ERROR;
    }

    if (carrier->data)
    {
        if (base->kind == RECORD_KIND_STRUCT)
        {
            not_record_struct_t *record_struct = (not_record_struct_t *)base->value;
            not_node_t *type = record_struct->type;

            not_strip_t *strip_new = (not_strip_t *)record_struct->value;
            not_record_t *result = not_call_for_bracket(node, carrier->data, strip_new, type, applicant);

            if (not_record_link_decrease(base) < 0)
            {
                return ERROR;
            }

            if (result == ERROR)
            {
                return ERROR;
            }

            return result;
        }
        else if (base->kind == RECORD_KIND_OBJECT)
        {
            not_node_block_t *block = (not_node_block_t *)carrier->data->value;

            uint64_t arg_cnt = 0;
            for (not_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;
            }

            if ((arg_cnt > 1) || (arg_cnt == 0))
            {
                not_error_type_by_node(node, "'%s' takes %lld positional arguments but %lld were given",
                                       not_record_type_as_string(base), 1, arg_cnt);

                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }

            not_node_t *item1 = block->items;
            not_node_argument_t *argument = (not_node_argument_t *)item1->value;

            if (argument->value)
            {
                not_error_type_by_node(item1, "'%s' not support", "pair");
                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, origin);
            if (record_arg == ERROR)
            {
                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }

            if (record_arg->kind != RECORD_KIND_STRING)
            {
                not_error_type_by_node(item1, "'%s' must be of '%s' type", "key", "string");

                if (not_record_link_decrease(record_arg) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }

                return ERROR;
            }

            for (not_record_object_t *item = (not_record_object_t *)base->value; item != NULL; item = item->next)
            {
                if (strcmp(item->key, (char *)record_arg->value) == 0)
                {
                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return ERROR;
                    }
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return item->value;
                }
            }

            not_error_type_by_node(node, "'%s' has no contain key '%s'",
                                   not_record_type_as_string(base), (char *)record_arg->value);

            if (not_record_link_decrease(record_arg) < 0)
            {
                return ERROR;
            }
            if (not_record_link_decrease(base) < 0)
            {
                return ERROR;
            }

            return ERROR;
        }
        else if (base->kind == RECORD_KIND_TUPLE)
        {
            not_node_block_t *block = (not_node_block_t *)carrier->data->value;

            uint64_t arg_cnt = 0;
            for (not_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;
            }

            if ((arg_cnt > 3) || (arg_cnt == 0))
            {
                not_error_type_by_node(node, "'%s' takes %lld positional arguments but %lld were given",
                                       not_record_type_as_string(base), 3, arg_cnt);

                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }

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
            for (not_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;

                not_node_argument_t *argument = (not_node_argument_t *)item->value;

                if (argument->value)
                {
                    mpz_clears(start, stop, step);
                    not_error_type_by_node(item, "'%s' not support", "pair");
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, origin);
                if (record_arg == ERROR)
                {
                    mpz_clears(start, stop, step);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (record_arg->kind != RECORD_KIND_INT)
                {
                    mpz_clears(start, stop, step);
                    not_error_type_by_node(item, "'%s' must be of '%s' type", "key", "int");

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return ERROR;
                    }
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }

                    return ERROR;
                }

                if (arg_cnt == 1)
                {
                    record_to_mpz(record_arg, start);
                }
                else if (arg_cnt == 2)
                {
                    record_to_mpz(record_arg, stop);
                }
                else if (arg_cnt == 3)
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

            for (not_record_tuple_t *item = (not_record_tuple_t *)base->value; item != NULL; item = item->next)
            {
                mpz_add_ui(length, length, 1);
            }

            if (arg_cnt > 1)
            {
                if (mpz_cmp(start, length) >= 0)
                {
                    not_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (mpz_cmp(stop, length) >= 0)
                {
                    not_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
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

                not_record_tuple_t *top = NULL, *declaration = NULL;
                if (mpz_cmp(start, stop) <= 0)
                {
                    while ((mpz_cmp(term, start) >= 0) && (mpz_cmp(term, stop) <= 0))
                    {
                        mpz_set_si(cnt, 0);
                        for (not_record_tuple_t *item = (not_record_tuple_t *)base->value; item != NULL; item = item->next)
                        {
                            if (mpz_cmp(term, cnt) == 0)
                            {
                                not_record_link_increase(item->value);

                                not_record_tuple_t *tuple = not_record_make_tuple(item->value, NULL);
                                if (tuple == ERROR)
                                {
                                    if (not_record_link_decrease(item->value) < 0)
                                    {
                                        mpz_clear(start);
                                        mpz_clear(stop);
                                        mpz_clear(step);
                                        mpz_clear(term);
                                        mpz_clear(cnt);
                                        mpz_clear(length);
                                        return ERROR;
                                    }
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);

                                    if (top)
                                    {
                                        if (not_record_tuple_destroy(top) < 0)
                                        {
                                            if (not_record_link_decrease(base) < 0)
                                            {
                                                return ERROR;
                                            }
                                            return ERROR;
                                        }
                                    }

                                    if (not_record_link_decrease(base) < 0)
                                    {
                                        return ERROR;
                                    }
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
                    while ((mpz_cmp(term, stop) >= 0) && (mpz_cmp(term, start) <= 0))
                    {
                        mpz_set_si(cnt, 0);
                        for (not_record_tuple_t *item = (not_record_tuple_t *)base->value; item != NULL; item = item->next)
                        {
                            if (mpz_cmp(term, cnt) == 0)
                            {
                                not_record_link_increase(item->value);

                                not_record_tuple_t *tuple = not_record_make_tuple(item->value, NULL);
                                if (tuple == ERROR)
                                {
                                    if (not_record_link_decrease(item->value) < 0)
                                    {
                                        mpz_clear(start);
                                        mpz_clear(stop);
                                        mpz_clear(step);
                                        mpz_clear(term);
                                        mpz_clear(cnt);
                                        mpz_clear(length);
                                        return ERROR;
                                    }
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);

                                    if (top)
                                    {
                                        if (not_record_tuple_destroy(top) < 0)
                                        {
                                            if (not_record_link_decrease(base) < 0)
                                            {
                                                return ERROR;
                                            }
                                            return ERROR;
                                        }
                                    }

                                    if (not_record_link_decrease(base) < 0)
                                    {
                                        return ERROR;
                                    }
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

                not_record_t *result = not_record_create(RECORD_KIND_TUPLE, top);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else
            {
                if (mpz_cmp(start, length) >= 0)
                {
                    not_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                while (mpz_cmp_si(start, 0) < 0)
                {
                    mpz_add(start, length, start);
                }

                mpz_set(term, start);
                for (not_record_tuple_t *item = (not_record_tuple_t *)base->value; item != NULL; item = item->next)
                {
                    if (mpz_cmp(cnt, term) == 0)
                    {
                        mpz_clear(start);
                        mpz_clear(stop);
                        mpz_clear(step);
                        mpz_clear(term);
                        mpz_clear(cnt);
                        mpz_clear(length);
                        if (not_record_link_decrease(base) < 0)
                        {
                            return ERROR;
                        }
                        return item->value;
                    }
                    mpz_add_ui(cnt, cnt, 1);
                }
            }
        }
        else if (base->kind == RECORD_KIND_STRING)
        {
            not_node_block_t *block = (not_node_block_t *)carrier->data->value;

            uint64_t arg_cnt = 0;
            for (not_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;
            }

            if ((arg_cnt > 3) || (arg_cnt == 0))
            {
                not_error_type_by_node(node, "'%s' takes %lld positional arguments but %lld were given",
                                       not_record_type_as_string(base), 3, arg_cnt);

                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }

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
            for (not_node_t *item = block->items; item != NULL; item = item->next)
            {
                arg_cnt += 1;

                not_node_argument_t *argument = (not_node_argument_t *)item->value;

                if (argument->value)
                {
                    mpz_clears(start, stop, step);
                    not_error_type_by_node(item, "'%s' not support", "pair");
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                not_record_t *record_arg = not_execute_expression(argument->key, strip, applicant, origin);
                if (record_arg == ERROR)
                {
                    mpz_clears(start, stop, step);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (record_arg->kind != RECORD_KIND_INT)
                {
                    mpz_clears(start, stop, step);
                    not_error_type_by_node(item, "'%s' must be of '%s' type", "key", "int");

                    if (not_record_link_decrease(record_arg) < 0)
                    {
                        return ERROR;
                    }
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }

                    return ERROR;
                }

                if (arg_cnt == 1)
                {
                    record_to_mpz(record_arg, start);
                }
                else if (arg_cnt == 2)
                {
                    record_to_mpz(record_arg, stop);
                }
                else if (arg_cnt == 3)
                {
                    record_to_mpz(record_arg, step);
                }
            }

            mpz_t length, cnt, term;
            mpz_init(length);
            mpz_init(cnt);
            mpz_init(term);

            mpz_set_si(length, strlen((char *)base->value));
            mpz_set_si(term, 0);
            mpz_set_si(cnt, 0);

            if (arg_cnt > 1)
            {
                if (mpz_cmp(start, length) >= 0)
                {
                    not_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (mpz_cmp(stop, length) >= 0)
                {
                    not_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
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

                not_record_tuple_t *top = NULL, *declaration = NULL;
                if (mpz_cmp(start, stop) <= 0)
                {
                    while ((mpz_cmp(term, start) >= 0) && (mpz_cmp(term, stop) <= 0))
                    {
                        mpz_set_si(cnt, 0);
                        for (char *str = (char *)base->value; str != NULL; str++)
                        {
                            if (mpz_cmp(term, cnt) == 0)
                            {
                                not_record_t *item = not_record_create(RECORD_KIND_CHAR, str);
                                if (item == ERROR)
                                {
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);
                                    if (not_record_link_decrease(base) < 0)
                                    {
                                        return ERROR;
                                    }

                                    if (top)
                                    {
                                        if (not_record_tuple_destroy(top) < 0)
                                        {
                                            return ERROR;
                                        }
                                    }
                                    return ERROR;
                                }
                                item->reference = 1;

                                not_record_tuple_t *tuple = not_record_make_tuple(item, NULL);
                                if (tuple == ERROR)
                                {
                                    if (not_record_link_decrease(item) < 0)
                                    {
                                        mpz_clear(start);
                                        mpz_clear(stop);
                                        mpz_clear(step);
                                        mpz_clear(term);
                                        mpz_clear(cnt);
                                        mpz_clear(length);
                                        return ERROR;
                                    }
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);

                                    if (top)
                                    {
                                        if (not_record_tuple_destroy(top) < 0)
                                        {
                                            if (not_record_link_decrease(base) < 0)
                                            {
                                                return ERROR;
                                            }
                                            return ERROR;
                                        }
                                    }

                                    if (not_record_link_decrease(base) < 0)
                                    {
                                        return ERROR;
                                    }
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
                    while ((mpz_cmp(term, stop) >= 0) && (mpz_cmp(term, start) <= 0))
                    {
                        mpz_set_si(cnt, 0);
                        for (char *str = (char *)base->value; str != NULL; str++)
                        {
                            if (mpz_cmp(term, cnt) == 0)
                            {
                                not_record_t *item = not_record_create(RECORD_KIND_CHAR, str);
                                if (item == ERROR)
                                {
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);
                                    if (not_record_link_decrease(base) < 0)
                                    {
                                        return ERROR;
                                    }

                                    if (top)
                                    {
                                        if (not_record_tuple_destroy(top) < 0)
                                        {
                                            return ERROR;
                                        }
                                    }

                                    return ERROR;
                                }
                                item->reference = 1;

                                not_record_tuple_t *tuple = not_record_make_tuple(item, NULL);
                                if (tuple == ERROR)
                                {
                                    if (not_record_link_decrease(item) < 0)
                                    {
                                        mpz_clear(start);
                                        mpz_clear(stop);
                                        mpz_clear(step);
                                        mpz_clear(term);
                                        mpz_clear(cnt);
                                        mpz_clear(length);
                                        return ERROR;
                                    }
                                    mpz_clear(start);
                                    mpz_clear(stop);
                                    mpz_clear(step);
                                    mpz_clear(term);
                                    mpz_clear(cnt);
                                    mpz_clear(length);

                                    if (top)
                                    {
                                        if (not_record_tuple_destroy(top) < 0)
                                        {
                                            if (not_record_link_decrease(base) < 0)
                                            {
                                                return ERROR;
                                            }
                                            return ERROR;
                                        }
                                    }

                                    if (not_record_link_decrease(base) < 0)
                                    {
                                        return ERROR;
                                    }
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

                not_record_t *result = not_record_create(NODE_KIND_TUPLE, top);
                if (result == ERROR)
                {
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                if (not_record_link_decrease(base) < 0)
                {
                    return ERROR;
                }

                return result;
            }
            else
            {
                if (mpz_cmp(start, length) >= 0)
                {
                    not_error_type_by_node(node, "array bounds exceeded");
                    mpz_clear(start);
                    mpz_clear(stop);
                    mpz_clear(step);
                    mpz_clear(term);
                    mpz_clear(cnt);
                    mpz_clear(length);
                    if (not_record_link_decrease(base) < 0)
                    {
                        return ERROR;
                    }
                    return ERROR;
                }

                while (mpz_cmp_si(start, 0) < 0)
                {
                    mpz_add(start, length, start);
                }

                mpz_set(term, start);

                for (char *str = (char *)base->value; str != NULL; str++)
                {
                    if (mpz_cmp(cnt, term) == 0)
                    {
                        not_record_t *item = not_record_create(RECORD_KIND_CHAR, str);
                        if (item == ERROR)
                        {
                            mpz_clear(start);
                            mpz_clear(stop);
                            mpz_clear(step);
                            mpz_clear(term);
                            mpz_clear(cnt);
                            mpz_clear(length);
                            if (not_record_link_decrease(base) < 0)
                            {
                                return ERROR;
                            }
                            return ERROR;
                        }
                        item->reference = 1;

                        mpz_clear(start);
                        mpz_clear(stop);
                        mpz_clear(step);
                        mpz_clear(term);
                        mpz_clear(cnt);
                        mpz_clear(length);
                        if (not_record_link_decrease(base) < 0)
                        {
                            return ERROR;
                        }

                        return item;
                    }
                    mpz_add_ui(cnt, cnt, 1);
                }
            }
        }
    }

    not_error_type_by_node(node, "array implement not support");
    return ERROR;
}