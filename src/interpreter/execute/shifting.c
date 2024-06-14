#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
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
#include "execute.h"

sy_record_t *
sy_execute_shr(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0)
            {
                mpz_set_ui(result, 0);
            }
            else
            {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_call_operator_by_one_arg(node, left, right, ">>", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "null");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }

    return sy_record_make_undefined();
}

sy_record_t *
sy_execute_shl(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);

            mpz_set_f(num, (*(mpf_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);

            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));

            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = sy_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "string");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_call_operator_by_one_arg(node, left, right, "<<", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "undefined");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "nan");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "int");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "float");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "char");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "null");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "object");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "tuple");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "type");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "struct");
            return ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }

    return sy_record_make_undefined();
}

sy_record_t *
sy_execute_shifting(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_SHR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_addative(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_addative(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record = sy_execute_shr(node, left, right, applicant);

        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        if (sy_record_link_decrease(right) < 0)
        {
            return ERROR;
        }

        return record;
    }
    else if (node->kind == NODE_KIND_SHL)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_addative(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_addative(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            sy_record_link_decrease(left);
            return ERROR;
        }

        sy_record_t *record = sy_execute_shl(node, left, right, applicant);

        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        if (sy_record_link_decrease(right) < 0)
        {
            return ERROR;
        }

        return record;
    }
    else
    {
        return sy_execute_addative(node, strip, applicant, origin);
    }
}
