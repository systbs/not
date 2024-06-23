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
not_shifting_shr(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "nan");
            return NOT_PTR_ERROR;
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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "nan");
            return NOT_PTR_ERROR;
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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "nan");
            return NOT_PTR_ERROR;
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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, ">>", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "null");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }

    return not_record_make_undefined();
}

not_record_t *
not_shifting_shl(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "nan");
            return NOT_PTR_ERROR;
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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "nan");
            return NOT_PTR_ERROR;
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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "nan");
            return NOT_PTR_ERROR;
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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

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

            not_record_t *record = not_record_make_int_from_z(result);

            mpz_clear(result);

            return record;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "string");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        return not_call_operator_by_one_arg(node, left, right, "<<", applicant);
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "undefined");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NAN)
        {

            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "nan");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_INT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "int");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_FLOAT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "float");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_CHAR)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "char");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRING)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "null");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_OBJECT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "object");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TUPLE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "tuple");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_TYPE)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "type");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_STRUCT)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "struct");
            return NOT_PTR_ERROR;
        }
        else if (right->kind == RECORD_KIND_NULL)
        {
            not_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "null");
            return NOT_PTR_ERROR;
        }

        return not_record_make_undefined();
    }

    return not_record_make_undefined();
}

not_record_t *
not_shifting(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_SHR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_addative(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_addative(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_shifting_shr(node, left, right, applicant);

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
    else if (node->kind == NODE_KIND_SHL)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_addative(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_addative(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        not_record_t *record = not_shifting_shl(node, left, right, applicant);

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
        return not_addative(node, strip, applicant, origin);
    }
}
