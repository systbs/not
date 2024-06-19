#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
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

#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

not_record_t *
not_execute_and(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_and(result, (*(mpz_t *)(left->value)), (*(mpz_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_and(result, (*(mpz_t *)(left->value)), result);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (*(char *)(right->value)));
            mpz_and(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t result;
            mpz_init(result);
            mpz_and(result, op1_mpz, (*(mpz_t *)(right->value)));
            mpz_clear(op1_mpz);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init(op2_mpz);
            mpz_set_f(op2_mpz, (*(mpf_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_and(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(char *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_and(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(char *)(left->value)));
            mpz_and(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(char *)(left->value)));
            mpz_and(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)) & (*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_record_t *record = not_record_make_char((*(char *)(left->value)));
            return record;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "&", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_record_t *record = not_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            not_record_t *record = not_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_record_t *record = not_record_make_char((*(char *)(right->value)));
            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            return not_record_make_nan();
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            return not_record_make_nan();
        }

        return not_record_make_undefined();
    }

    return not_record_make_undefined();
}

not_record_t *
not_execute_bitwise_and(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_AND)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_execute_bitwise_and(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_execute_equality(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_execute_and(node, left, right, applicant);

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
        return not_execute_equality(node, strip, applicant, origin);
    }
}