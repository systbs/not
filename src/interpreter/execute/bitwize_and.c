#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>

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
#include "execute.h"

sy_record_t *
sy_execute_and(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));            
            return  record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);            
            return  record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));               return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));            
            return  record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);            
            return  record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));            
            return  record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_and(result, (*(mpz_t *)(left->value)), (*(mpz_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_and(result, (*(mpz_t *)(left->value)), result);

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (*(char *)(right->value)));
            mpz_and(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t result;
            mpz_init(result);
            mpz_and(result, op1_mpz, (*(mpz_t *)(right->value)));
            mpz_clear(op1_mpz);

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
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

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
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

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(char *)(left->value)));
            mpz_and(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(char *)(left->value)));
            mpz_and(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)) & (*(char *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)));
            return record;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {            
            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, "&", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {         
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_INT)
        {
            sy_record_t *record = sy_record_make_int_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_int_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {            
            return sy_record_make_nan();
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {            
            return sy_record_make_nan();
        }

        return sy_record_make_undefined();
    }

    return sy_record_make_undefined();
}

sy_record_t *
sy_execute_bitwise_and(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_AND)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_bitwise_and(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {            
            return ERROR;
        }

        sy_record_t *right = sy_execute_equality(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {            
            return ERROR;
        }

        sy_record_t *record = sy_execute_and(node, left, right, applicant);
        
        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    {
        return sy_execute_equality(node, strip, applicant, origin);
    }
}