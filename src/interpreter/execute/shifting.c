#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>

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
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "undefiend", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "nan", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int8", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int16", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int32", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "int64", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint8", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint16", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint32", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "uint64", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_BIGINT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigint", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float32", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "float64", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_BIGFLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "bigfloat", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result, max_shift;

            mpz_init(result);
            mpz_init(max_shift);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            mpz_set_ui(max_shift, mpz_sizeinbase(num, 2));

            if (mpz_cmp(shift_amount, max_shift) >= 0) {
                mpz_set_ui(result, 0);
            } else {
                unsigned long shift_val = mpz_get_ui(shift_amount);
                mpz_fdiv_q_2exp(result, num, shift_val);
            }
            mpz_clear(num);
            mpz_clear(shift_amount);
            mpz_clear(max_shift);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "char", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "string", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "object", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "tuple", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "type", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, ">>", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "null");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for >>: '%s' and '%s'", "null", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
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
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "undefiend", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "nan", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int8", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int16", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int32", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(int64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "int64", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint8_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint8", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint16_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint16", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint32", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint32_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_ui(num, (*(uint64_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "uint64", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_BIGINT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
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

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set(num, (*(mpz_t *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigint", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(float *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float32", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_d(num, (*(double*)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "float64", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_BIGFLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(shift_amount);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_set_f(shift_amount, (*(mpf_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            mpz_init(num);
            
            mpz_set_f(num, (*(mpf_t*)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "bigfloat", "null");
            return ERROR;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {


            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(int64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint8_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint16_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint32_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_ui(shift_amount, (*(uint64_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set(shift_amount, (*(mpz_t *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_d(shift_amount, (*(float *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_d(shift_amount, (*(double *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
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

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_t num, shift_amount, result;

            mpz_init(result);
            
            mpz_init_set_si(num, (*(char *)(left->value)));
            mpz_init_set_si(shift_amount, (*(char *)(right->value)));
            
            unsigned long shift_val = mpz_get_ui(shift_amount);
            mpz_mul_2exp(result, num, shift_val);
            mpz_clear(num);
            mpz_clear(shift_amount);

            sy_record_t *record = NULL;
            // int8_t
            if (mpz_cmp_si(result, INT8_MIN) >= 0 && mpz_cmp_si(result, INT8_MAX) <= 0) {
                record = sy_record_make_int8((int8_t)mpz_get_si(result));
            }
            else
            // uint8_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT8_MAX) <= 0) {
                record = sy_record_make_uint8((uint8_t)mpz_get_ui(result));
            }
            else
            // int16_t
            if (mpz_cmp_si(result, INT16_MIN) >= 0 && mpz_cmp_si(result, INT16_MAX) <= 0) {
                record = sy_record_make_int16((int16_t)mpz_get_si(result));
            }
            else
            // uint16_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT16_MAX) <= 0) {
                record = sy_record_make_uint16((uint16_t)mpz_get_ui(result));
            }
            else
            // int32_t
            if (mpz_cmp_si(result, INT32_MIN) >= 0 && mpz_cmp_si(result, INT32_MAX) <= 0) {
                record = sy_record_make_int32((int32_t)mpz_get_si(result));
            }
            else
            // uint32_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT32_MAX) <= 0) {
                record = sy_record_make_uint32((uint32_t)mpz_get_ui(result));
            }
            else
            // int64_t
            if (mpz_cmp_si(result, INT64_MIN) >= 0 && mpz_cmp_si(result, INT64_MAX) <= 0) {
                record = sy_record_make_int64(mpz_get_si(result));
            }
            else
            // uint64_t
            if (mpz_cmp_ui(result, 0) >= 0 && mpz_cmp_ui(result, UINT64_MAX) <= 0) {
                record = sy_record_make_uint64(mpz_get_ui(result));
            }
            // mpz_t
            else {
                record = sy_record_make_bigint_from_z(result);
            }

            mpz_clear(result);

            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "char", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "string");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "string", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "object", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "tuple", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "type", "null");
            return ERROR;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        return sy_execute_call_for_operator_by_one_argument(node, left, right, "<<", applicant);
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "undefined");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "nan");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "int8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "int16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "int32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "int64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "uint8");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "uint16");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "uint32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "uint64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "bigint");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "float32");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "float64");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "bigfloat");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "char");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "null");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "object");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "tuple");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "type");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_error_type_by_node(node, "unsupported operand type(s) for <<: '%s' and '%s'", "null", "struct");
            return ERROR;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
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

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_SHL)
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
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_shl(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    {
        return sy_execute_addative(node, strip, applicant, origin);
    }
}
