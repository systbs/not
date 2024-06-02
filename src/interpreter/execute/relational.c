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

int32_t
sy_execute_truthy_lt(sy_record_t *left, sy_record_t *right)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    if (left->kind == RECORD_KIND_INT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return (*(int8_t *)(left->value)) < (*(int8_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return (*(int8_t *)(left->value)) < (*(int16_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return (*(int8_t *)(left->value)) < (*(int32_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return (*(int8_t *)(left->value)) < (*(int64_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int8_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int8_t *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int8_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int8_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int8_t *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(int16_t *)(left->value)) < (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(int16_t *)(left->value)) < (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(int16_t *)(left->value)) < (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(int16_t *)(left->value)) < (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int16_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int16_t *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int16_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int16_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int16_t *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(int32_t *)(left->value)) < (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(int32_t *)(left->value)) < (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(int32_t *)(left->value)) < (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(int32_t *)(left->value)) < (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int32_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int32_t *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int32_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int32_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int32_t *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return ((*(int32_t *)(left->value)));
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(int64_t *)(left->value)) < (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(int64_t *)(left->value)) < (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(int64_t *)(left->value)) < (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(int64_t *)(left->value)) < (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int64_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int64_t *)(left->value)) < (int64_t)(*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int64_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int64_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int64_t *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) < ((uint8_t)(*(int8_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) < ((uint16_t)(*(int16_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) < ((uint32_t)(*(int32_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) < ((uint64_t)(*(int64_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint8_t *)(left->value)) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint8_t *)(left->value)) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint8_t *)(left->value)) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint8_t *)(left->value)) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint8_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint8_t *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint8_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint8_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char*)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) < ((unsigned char)(*(char *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) < ((uint8_t)(*(int8_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) < ((uint16_t)(*(int16_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) < ((uint32_t)(*(int32_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) < ((uint64_t)(*(int64_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint16_t *)(left->value)) < (*(uint8_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint16_t *)(left->value)) < (*(uint16_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint16_t *)(left->value)) < (*(uint32_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint16_t *)(left->value)) < (*(uint64_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint16_t *)(left->value))) >= 0);

        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint16_t *)(left->value)) < (*(float *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint16_t *)(left->value)) < (*(double *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint16_t *)(left->value))) >= 0);

        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char*)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) < ((unsigned char)(*(char *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) < ((uint8_t)(*(int8_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) < ((uint16_t)(*(int16_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) < ((uint32_t)(*(int32_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) < ((uint64_t)(*(int64_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint32_t *)(left->value)) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint32_t *)(left->value)) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint32_t *)(left->value)) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint32_t *)(left->value)) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint32_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint32_t *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint32_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint32_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char*)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) < ((unsigned char)(*(char *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) < ((uint8_t)(*(int8_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) < ((uint16_t)(*(int16_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) < ((uint32_t)(*(int32_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) < ((uint64_t)(*(int64_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint64_t *)(left->value)) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint64_t *)(left->value)) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint64_t *)(left->value)) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint64_t *)(left->value)) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint64_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint64_t *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint64_t *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint64_t *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char*)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) < ((unsigned char)(*(char *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_BIGINT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int8_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int16_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int32_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int64_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint8_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint16_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint32_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint64_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return (mpz_cmp_d((*(mpz_t *)(left->value)), (*(float *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return (mpz_cmp_d((*(mpz_t *)(left->value)), (*(double *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) < 0);
            mpf_clear(mpf_from_mpz);
            return result;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return (mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(float *)(left->value)) < (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(float *)(left->value)) < (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(float *)(left->value)) < (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(float *)(left->value)) < (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(float *)(left->value)) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(float *)(left->value)) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(float *)(left->value)) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(float *)(left->value)) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_d((*(mpz_t *)(right->value)), (*(float *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(float *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(float *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_d((*(mpf_t *)(right->value)), (*(float *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(float *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(double *)(left->value)) < (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(double *)(left->value)) < (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(double *)(left->value)) < (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(double *)(left->value)) < (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(double *)(left->value)) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(double *)(left->value)) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(double *)(left->value)) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(double *)(left->value)) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_d((*(mpz_t *)(right->value)), (*(double *)(left->value))) >= 0);

        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(double *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(double *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_d((*(mpf_t *)(right->value)), (*(double *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(double *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_BIGFLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int8_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int16_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int32_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int64_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint8_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint16_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint32_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint64_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) < 0);
            mpf_clear(mpf_from_mpz);
            return result;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(float *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(double *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) < 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(char *)(left->value)) < (*(int8_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(char *)(left->value)) < (*(int16_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(char *)(left->value)) < (*(int32_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(char *)(left->value)) < (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) < (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) < (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) < (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) < (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(char *)(left->value)) < (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(char *)(left->value)) < (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) >= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(char *)(left->value)) < (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return (strcmp((*(char **)(left->value)), (*(char **)(right->value))) >= 0);

        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    return 0;
}

int32_t
sy_execute_truthy_le(sy_record_t *left, sy_record_t *right)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 1;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (left->kind == RECORD_KIND_INT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return (*(int8_t *)(left->value)) <= (*(int8_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return (*(int8_t *)(left->value)) <= (*(int16_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return (*(int8_t *)(left->value)) <= (*(int32_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return (*(int8_t *)(left->value)) <= (*(int64_t *)(right->value));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int8_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint8_t)(*(int8_t *)(left->value))) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int8_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int8_t *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int8_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int8_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int8_t *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(int16_t *)(left->value)) <= (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(int16_t *)(left->value)) <= (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(int16_t *)(left->value)) <= (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(int16_t *)(left->value)) <= (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int16_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint16_t)(*(int16_t *)(left->value))) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int16_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int16_t *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int16_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int16_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int16_t *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(int32_t *)(left->value)) <= (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(int32_t *)(left->value)) <= (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(int32_t *)(left->value)) <= (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(int32_t *)(left->value)) <= (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int32_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint32_t)(*(int32_t *)(left->value))) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int32_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int32_t *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int32_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int32_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int32_t *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return ((*(int32_t *)(left->value)));
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return ((*(int32_t *)(left->value)));
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(int64_t *)(left->value)) <= (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(int64_t *)(left->value)) <= (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(int64_t *)(left->value)) <= (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(int64_t *)(left->value)) <= (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(int64_t *)(left->value)) < 0)
            {
                return 1;
            }
            return (((uint64_t)(*(int64_t *)(left->value))) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(int64_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(int64_t *)(left->value)) <= (int64_t)(*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(int64_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(int64_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(int64_t *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT8)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) <= ((uint8_t)(*(int8_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) <= ((uint16_t)(*(int16_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) <= ((uint32_t)(*(int32_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) <= ((uint64_t)(*(int64_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint8_t *)(left->value)) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint8_t *)(left->value)) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint8_t *)(left->value)) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint8_t *)(left->value)) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint8_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint8_t *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint8_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint8_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint8_t *)(left->value)) <= ((unsigned char)(*(char *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT16)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) <= ((uint8_t)(*(int8_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) <= ((uint16_t)(*(int16_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) <= ((uint32_t)(*(int32_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) <= ((uint64_t)(*(int64_t *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint16_t *)(left->value)) <= (*(uint8_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint16_t *)(left->value)) <= (*(uint16_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint16_t *)(left->value)) <= (*(uint32_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint16_t *)(left->value)) <= (*(uint64_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint16_t *)(left->value))) > 0);

        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint16_t *)(left->value)) <= (*(float *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint16_t *)(left->value)) <= (*(double *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint16_t *)(left->value))) > 0);

        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint16_t *)(left->value)) <= ((unsigned char)(*(char *)(right->value))));

        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) <= ((uint8_t)(*(int8_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) <= ((uint16_t)(*(int16_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) <= ((uint32_t)(*(int32_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) <= ((uint64_t)(*(int64_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint32_t *)(left->value)) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint32_t *)(left->value)) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint32_t *)(left->value)) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint32_t *)(left->value)) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint32_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint32_t *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint32_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint32_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint32_t *)(left->value)) <= ((unsigned char)(*(char *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            if ((*(int8_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) <= ((uint8_t)(*(int8_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if ((*(int16_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) <= ((uint16_t)(*(int16_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if ((*(int32_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) <= ((uint32_t)(*(int32_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if ((*(int64_t *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) <= ((uint64_t)(*(int64_t *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(uint64_t *)(left->value)) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(uint64_t *)(left->value)) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(uint64_t *)(left->value)) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(uint64_t *)(left->value)) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_ui((*(mpz_t *)(right->value)), (*(uint64_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(uint64_t *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(uint64_t *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_ui((*(mpf_t *)(right->value)), (*(uint64_t *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if ((*(char *)(right->value)) < 0)
            {
                return 0;
            }
            return ((*(uint64_t *)(left->value)) <= ((unsigned char)(*(char *)(right->value))));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_BIGINT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int8_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int16_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int32_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return (mpz_cmp_si((*(mpz_t *)(left->value)), (*(int64_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint8_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint16_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint32_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return (mpz_cmp_ui((*(mpz_t *)(left->value)), (*(uint64_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp((*(mpz_t *)(left->value)), (*(mpz_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return (mpz_cmp_d((*(mpz_t *)(left->value)), (*(float *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return (mpz_cmp_d((*(mpz_t *)(left->value)), (*(double *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(left->value)));
            int32_t result = (mpf_cmp(mpf_from_mpz, (*(mpf_t *)(right->value))) <= 0);
            mpf_clear(mpf_from_mpz);
            return result;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return (mpz_cmp_d((*(mpz_t *)(left->value)), (*(char *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT32)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(float *)(left->value)) <= (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(float *)(left->value)) <= (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(float *)(left->value)) <= (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(float *)(left->value)) <= (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(float *)(left->value)) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(float *)(left->value)) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(float *)(left->value)) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(float *)(left->value)) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_d((*(mpz_t *)(right->value)), (*(float *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(float *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(float *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_d((*(mpf_t *)(right->value)), (*(float *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(float *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT64)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(double *)(left->value)) <= (*(int8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(double *)(left->value)) <= (*(int16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(double *)(left->value)) <= (*(int32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(double *)(left->value)) <= (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return ((*(double *)(left->value)) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return ((*(double *)(left->value)) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return ((*(double *)(left->value)) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return ((*(double *)(left->value)) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_d((*(mpz_t *)(right->value)), (*(double *)(left->value))) > 0);

        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(double *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(double *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_d((*(mpf_t *)(right->value)), (*(double *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(double *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_BIGFLOAT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {

            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int8_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int16_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int32_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(int64_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint8_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint16_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint32_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(uint64_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpf_t mpf_from_mpz;
            mpf_init(mpf_from_mpz);
            mpf_set_z(mpf_from_mpz, (*(mpz_t *)(right->value)));
            int32_t result = (mpf_cmp((*(mpf_t *)(left->value)), mpf_from_mpz) <= 0);
            mpf_clear(mpf_from_mpz);
            return result;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(float *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return (mpf_cmp_ui((*(mpf_t *)(left->value)), (*(double *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp((*(mpf_t *)(left->value)), (*(mpf_t *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return (mpf_cmp_si((*(mpf_t *)(left->value)), (*(char *)(right->value))) <= 0);
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }
        
        return 0;
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return ((*(char *)(left->value)) <= (*(int8_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            return ((*(char *)(left->value)) <= (*(int16_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return ((*(char *)(left->value)) <= (*(int32_t *)(right->value)));

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return ((*(char *)(left->value)) <= (*(int64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) <= (*(uint8_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) <= (*(uint16_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) <= (*(uint32_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if ((*(char *)(left->value)) < 0)
            {
                return 1;
            }
            return (((unsigned char)(*(char *)(left->value))) <= (*(uint64_t *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return (mpz_cmp_si((*(mpz_t *)(right->value)), (*(char *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return ((*(char *)(left->value)) <= (*(float *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return ((*(char *)(left->value)) <= (*(double *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return (mpf_cmp_si((*(mpf_t *)(right->value)), (*(char *)(left->value))) > 0);
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return ((*(char *)(left->value)) <= (*(char *)(right->value)));
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return (strcmp((*(char **)(left->value)), (*(char **)(right->value))) <= 0);

        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;

        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            return 0;
        }

        return 0;
    }
    
    return 0;
}

sy_record_t *
sy_execute_relational(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_LT)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }
        
        sy_record_t *record = sy_record_make_int32(sy_execute_truthy_lt(left, right));
        
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;
    }
    else
    if (node->kind == NODE_KIND_LE)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record = sy_record_make_int32(sy_execute_truthy_le(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;
    }
    else
    if (node->kind == NODE_KIND_GT)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record = sy_record_make_int32(!sy_execute_truthy_le(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;
    }
    else
    if (node->kind == NODE_KIND_GE)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_shifting(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_shifting(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record = sy_record_make_int32(!sy_execute_truthy_lt(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;
    }
    else
    {
        return sy_execute_shifting(node, strip, applicant, origin);
    }
}
