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
sy_execute_xor(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
    if (left->kind == RECORD_KIND_INT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int16((*(int8_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int8_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int8_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(int8_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(int8_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(int8_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(int8_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(int8_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int8_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int8_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int8_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(left->value)));
            return record;
        }

        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int16_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int16_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(int16_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(int16_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(int16_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(int16_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int16_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int16_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int16_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_uint64((*(int16_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_int16((*(int16_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int32_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(int32_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(int32_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(int32_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int32_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int32_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_INT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(int64_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(int64_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (int64_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int64_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int16((*(uint8_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(uint8_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(uint8_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint8_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint8_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint8_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_ui(op_mpz, (*(uint8_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((*(uint8_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(uint8_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint8_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(uint16_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(uint16_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint16_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint16_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_ui(op_mpz, (*(uint16_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((*(uint16_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(uint16_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint16_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(uint32_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint32_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_ui(op_mpz, (*(uint32_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(uint32_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint32_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_UINT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(int32_t *)(right->value)));           
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_ui(op_mpz, (*(uint64_t *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint64_t *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_BIGINT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (*(int8_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (*(int16_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (*(int32_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (*(int64_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_ui(op_mpz, (*(uint8_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_ui(op_mpz, (*(uint16_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_ui(op_mpz, (*(uint32_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_ui(op_mpz, (*(uint64_t *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_xor(result, (*(mpz_t *)(left->value)), (*(mpz_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (int32_t)(*(float *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t result;
            mpz_init(result);

            mpz_t op_mpz;
            mpz_init_set_si(op_mpz, (int64_t)(*(double *)(right->value)));
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_xor(result, (*(mpz_t *)(left->value)), result);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
            mpz_xor(result, (*(mpz_t *)(left->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((int32_t)(*(float *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((int32_t)(*(float *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((int32_t)(*(float *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (int32_t)(*(float *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((int32_t)(*(float *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (int32_t)(*(float *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_int32((int32_t)(*(float *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_FLOAT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((int64_t)(*(double *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (int64_t)(*(double *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (int64_t)(*(double *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_int64((int64_t)(*(double *)(left->value)) ^ (*(char *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            sy_record_t *record = sy_record_make_float64((*(double *)(left->value)));
            return record;
        }
        
        return sy_record_make_undefined();
    }
    else
    if (left->kind == RECORD_KIND_BIGFLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int8_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int16_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int32_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(int64_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint8_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint16_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint32_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (*(uint64_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, (*(mpz_t *)(right->value)));
            mpz_clear(op1_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (int32_t)(*(float *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init_set_ui(op2_mpz, (int64_t)(*(double *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t op1_mpz;
            mpz_init(op1_mpz);
            mpz_set_f(op1_mpz, (*(mpf_t *)(left->value)));

            mpz_t op2_mpz;
            mpz_init(op2_mpz);
            mpz_set_f(op2_mpz, (*(mpf_t *)(right->value)));

            mpz_t result;
            mpz_init(result);
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
            mpz_xor(result, op1_mpz, op2_mpz);
            mpz_clear(op1_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);
            return record;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(left->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_char((*(char *)(left->value)) ^ (*(int8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            sy_record_t *record = sy_record_make_int16((*(char *)(left->value)) ^ (*(int16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(char *)(left->value)) ^ (*(int32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(char *)(left->value)) ^ (*(int64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(char *)(left->value)) ^ (*(uint8_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(char *)(left->value)) ^ (*(uint16_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(char *)(left->value)) ^ (*(uint32_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(char *)(left->value)) ^ (*(uint64_t *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_t op_mpz, result;
            mpz_init(result);
            mpz_init_set_si(op_mpz, (*(char *)(left->value)));
            mpz_xor(result, (*(mpz_t *)(right->value)), op_mpz);
            mpz_clear(op_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_int32((*(char *)(left->value)) ^ (int32_t)(*(float *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_int64((*(char *)(left->value)) ^ (int64_t)(*(double *)(right->value)));            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            mpz_t op2_mpz;
            mpz_init_set_si(op2_mpz, (*(char *)(left->value)));
            mpz_xor(result, result, op2_mpz);
            mpz_clear(op2_mpz);

            sy_record_t *record = sy_record_make_bigint_from_z(result);
            mpz_clear(result);            
            return record;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            sy_record_t *record = sy_record_make_uint64((*(char *)(left->value)) ^ (*(char *)(right->value)));            
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
        return sy_execute_call_for_operator_by_one_argument(node, left, right, "^", applicant);
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
        if (right->kind == RECORD_KIND_INT8)
        {
            sy_record_t *record = sy_record_make_int8((*(int8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            sy_record_t *record = sy_record_make_int32((*(int32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            sy_record_t *record = sy_record_make_int64((*(int64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            sy_record_t *record = sy_record_make_uint8((*(uint8_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            sy_record_t *record = sy_record_make_uint16((*(uint16_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            sy_record_t *record = sy_record_make_uint32((*(uint32_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            sy_record_t *record = sy_record_make_uint64((*(uint64_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            sy_record_t *record = sy_record_make_bigint_from_z((*(mpz_t *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            sy_record_t *record = sy_record_make_float32((*(float *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            sy_record_t *record = sy_record_make_uint8((*(double *)(right->value)));
            return record;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpz_t result;
            mpz_init(result);
            mpz_set_f(result, (*(mpf_t *)(right->value)));

            sy_record_t *record = sy_record_make_bigint_from_z(result);
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
sy_execute_bitwise_xor(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_XOR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_bitwise_xor(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_bitwise_and(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record = sy_execute_xor(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    {
        return sy_execute_bitwise_and(node, strip, applicant, origin);
    }
}
