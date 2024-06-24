#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>
#include <ffi.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../mutex.h"
#include "../memory.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "../repository.h"
#include "../interpreter.h"
#include "../thread.h"
#include "symbol_table.h"
#include "strip.h"
#include "entry.h"
#include "helper.h"
#include "execute.h"

not_record_t *
not_addative_plus(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->null)
    {
        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpz_add(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);
            mpf_init(num1);

            mpf_set_z(num1, (*(mpz_t *)(left->value)));

            mpf_add(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpz_add(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2, result;

            mpz_init(result);
            mpz_init(num1);

            mpz_set_f(num1, (*(mpf_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpz_add(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t result;

            mpf_init(result);

            mpf_add(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_t num2, result;

            mpf_init(result);

            mpf_init_set_si(num2, (*(char *)(right->value)));

            mpf_add(result, (*(mpf_t *)(left->value)), num2);
            mpf_clear(num2);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpz_add(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;
            if ((mpz_cmp_si(result, -127) >= 0) && (mpz_cmp_si(result, 128) <= 0))
            {
                record = not_record_make_char(mpz_get_si(result));
            }
            else
            {
                record = not_record_make_int_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);

            mpf_init_set_si(num1, (*(char *)(left->value)));

            mpf_add(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpz_add(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;
            if ((mpz_cmp_si(result, -127) >= 0) && (mpz_cmp_si(result, 128) <= 0))
            {
                record = not_record_make_char(mpz_get_si(result));
            }
            else
            {
                record = not_record_make_int_from_z(result);
            }

            mpz_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        char *str_left = not_record_to_string(left, "");
        if (str_left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        char *str_all = not_record_to_string(right, str_left);
        if (str_all == NOT_PTR_ERROR)
        {
            not_memory_free(str_left);
            return NOT_PTR_ERROR;
        }
        not_memory_free(str_left);

        not_record_t *result = not_record_make_string(str_all);
        not_memory_free(str_all);

        return result;
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for +: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for +: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "+", applicant);
    }

    return not_record_make_nan();
}

not_record_t *
not_addative_minus(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_INT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpz_sub(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);
            mpf_init(num1);

            mpf_set_z(num1, (*(mpz_t *)(left->value)));

            mpf_add(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set(num1, (*(mpz_t *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpz_sub(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2, result;

            mpz_init(result);
            mpz_init(num1);

            mpz_set_f(num1, (*(mpf_t *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpz_sub(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t result;

            mpf_init(result);

            mpf_sub(result, (*(mpf_t *)(left->value)), (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_t num2, result;

            mpf_init(result);

            mpf_init_set_si(num2, (*(char *)(right->value)));

            mpf_sub(result, (*(mpf_t *)(left->value)), num2);
            mpf_clear(num2);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set(num2, (*(mpz_t *)(right->value)));

            mpz_sub(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;
            if ((mpz_cmp_si(result, -127) >= 0) && (mpz_cmp_si(result, 128) <= 0))
            {
                record = not_record_make_char(mpz_get_si(result));
            }
            else
            {
                record = not_record_make_int_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpf_t num1, result;

            mpf_init(result);

            mpf_init_set_si(num1, (*(char *)(left->value)));

            mpf_sub(result, num1, (*(mpf_t *)(right->value)));
            mpf_clear(num1);

            not_record_t *record = not_record_make_float_from_f(result);

            mpf_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num1, num2, result;

            mpz_init(result);

            mpz_init_set_si(num1, (*(char *)(left->value)));
            mpz_init_set_si(num2, (*(char *)(right->value)));

            mpz_sub(result, num1, num2);
            mpz_clear(num1);
            mpz_clear(num2);

            not_record_t *record = NOT_PTR_NULL;
            if ((mpz_cmp_si(result, -127) >= 0) && (mpz_cmp_si(result, 128) <= 0))
            {
                record = not_record_make_char(mpz_get_si(result));
            }
            else
            {
                record = not_record_make_int_from_z(result);
            }

            mpz_clear(result);

            return record;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        char *str_left = not_record_to_string(left, "");
        if (str_left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        char *str_all = not_record_to_string(right, str_left);
        if (str_all == NOT_PTR_ERROR)
        {
            not_memory_free(str_left);
            return NOT_PTR_ERROR;
        }
        not_memory_free(str_left);

        not_record_t *result = not_record_make_string(str_all);
        not_memory_free(str_all);

        return result;
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for -: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->null)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for -: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }

        return not_record_make_nan();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "-", applicant);
    }

    return not_record_make_nan();
}

not_record_t *
not_addative(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_PLUS)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_addative(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_multipicative(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_addative_plus(node, left, right, applicant);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return record;
    }
    else if (node->kind == NODE_KIND_MINUS)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_addative(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_multipicative(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_addative_minus(node, left, right, applicant);

        if (not_record_link_decrease(left) < 0)
        {
            return NOT_PTR_ERROR;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }

        return record;
    }
    else
    {
        return not_multipicative(node, strip, applicant, origin);
    }
}
