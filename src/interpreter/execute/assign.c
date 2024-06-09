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
#include "../../memory.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

int32_t
sy_execute_set_value(sy_node_t *node, sy_record_t *left, sy_record_t *right)
{
    if (left->readonly)
    {
        sy_error_type_by_node(node, "readonly assignment: '%s' and '%s'", "int8", "int16");
        return -1;
    }

    if (left->kind == RECORD_KIND_INT8)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "uint8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int8", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT16)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = (int16_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = (int16_t)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = (int16_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int16", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT32)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = (int32_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = (int32_t)(*(int16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = (int32_t)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = (int32_t)(*(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = (int32_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int32", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_INT64)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(int16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(int32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(uint32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = (int64_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "int64", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT8)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "int8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "char");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint8", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT16)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "int8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = (uint16_t)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "char");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint16", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT32)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "int8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = (uint32_t)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = (uint32_t)(*(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "char");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint32", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UINT64)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "int8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = (uint64_t)(*(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = (uint64_t)(*(uint32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "char");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "uint64", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_BIGINT)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_si(*(mpz_t *)(ptr), *(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_si(*(mpz_t *)(ptr), *(int16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_si(*(mpz_t *)(ptr), *(int32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_si(*(mpz_t *)(ptr), *(int64_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(uint32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(uint64_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "float32");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "float64");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "bigfloat");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init(*(mpz_t *)(ptr));
            mpz_set_ui(*(mpz_t *)(ptr), *(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "string");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "object");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "tuple");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "type");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "struct");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "null");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "undefined");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigint", "nan");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT32)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = (float)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = (float)(*(int16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = (float)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = (float)(*(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "bigint");
                return -1;
            }
            mpz_clear(*(mpz_t *)(left->value));
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = (float)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float32", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_FLOAT64)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(int16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(int32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(uint32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "bigint");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(float *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = (double)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "float64", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_BIGFLOAT)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_si(*(mpf_t *)(ptr), *(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_si(*(mpf_t *)(ptr), *(int16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_si(*(mpf_t *)(ptr), *(int32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_si(*(mpf_t *)(ptr), *(int64_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_ui(*(mpf_t *)(ptr), *(uint8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_ui(*(mpf_t *)(ptr), *(uint16_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_ui(*(mpf_t *)(ptr), *(uint32_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_ui(*(mpf_t *)(ptr), *(uint64_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_z(*(mpf_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_d(*(mpf_t *)(ptr), *(float *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_d(*(mpf_t *)(ptr), *(float *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init(*(mpf_t *)(ptr));
            mpf_set_d(*(mpf_t *)(ptr), *(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "string");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "object");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "tuple");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "type");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "struct");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "null");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "undefined");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "bigfloat", "nan");
                return -1;
            }
            mpf_clear(*(mpf_t *)left->value);
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_CHAR)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(int8_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "uint8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "bigfloat");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(char));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(char *)(ptr) = (char)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_CHAR;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "char", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRING)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "int8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "uint8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "bigint");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "bigfloat");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "string", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int8");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int16");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int32");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int64");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint8");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint16");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint32");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint64");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigint");
                return -1;
            }
            
            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float32");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float64");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigfloat");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_object_destroy((sy_record_object_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int8");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int16");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int32");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int64");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint8");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint16");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint32");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint64");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigint");
                return -1;
            }
            
            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float32");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float64");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigfloat");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_tuple_destroy((sy_record_tuple_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_TYPE)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int8");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int16");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int32");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int64");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint8");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint16");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint32");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint64");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigint");
                return -1;
            }
            
            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float32");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float64");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigfloat");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_type_destroy((sy_record_type_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_STRUCT)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int8");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int16");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int32");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "int64");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint8");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint16");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint32");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "uint64");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigint");
                return -1;
            }
            
            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float32");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "float64");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "bigfloat");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "char");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "string");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_memory_calloc(1, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "object");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "tuple");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "type");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "struct");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "null");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "undefined");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "object", "nan");
                return -1;
            }

            if (sy_record_struct_destroy((sy_record_struct_t *)left->value) < 0)
            {
                return -1;
            }

            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NULL)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "int8");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "uint8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "char");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "null", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "int8");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "uint8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "char");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "undefined", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }
    else
    if (left->kind == RECORD_KIND_NAN)
    {
        if (right->kind == RECORD_KIND_INT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "int8");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = *(int8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "int16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int16_t *)(ptr) = *(int16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "int32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int32_t *)(ptr) = *(int32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_INT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "int64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(int64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int64_t *)(ptr) = *(int64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_INT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT8)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "uint8");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint8_t *)(ptr) = *(uint8_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT16)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "uint16");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint16_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint16_t *)(ptr) = *(uint16_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT16;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "uint32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint32_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint32_t *)(ptr) = *(uint32_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UINT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "uint64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(uint64_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(uint64_t *)(ptr) = *(uint64_t *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_UINT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGINT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "bigint");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpz_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpz_init_set(*(mpz_t *)(ptr), *(mpz_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGINT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT32)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "float32");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(float));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(float *)(ptr) = *(float *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT32;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_FLOAT64)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "float64");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, sizeof(double));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(double *)(ptr) = *(double *)(right->value);
            left->value = ptr;
            left->kind = RECORD_KIND_FLOAT64;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_BIGFLOAT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "bigfloat");
                return -1;
            }
            
            void *ptr = sy_memory_realloc(left->value, sizeof(mpf_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            mpf_init_set(*(mpf_t *)(ptr), *(mpf_t *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_BIGFLOAT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_CHAR)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "char");
                return -1;
            }

            void *ptr = sy_memory_realloc(left->value, sizeof(int8_t));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            *(int8_t *)(ptr) = (int8_t)(*(char *)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_INT8;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRING)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "string");
                return -1;
            }
            void *ptr = sy_memory_realloc(left->value, strlen(*(char **)(right->value)));
            if (ptr == NULL)
            {
                sy_error_no_memory();
                return -1;
            }
            strcpy(*(char **)(ptr), *(char **)(right->value));
            left->value = ptr;
            left->kind = RECORD_KIND_STRING;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_OBJECT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "object");
                return -1;
            }
            void *ptr = sy_record_object_copy((sy_record_object_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_OBJECT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TUPLE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "tuple");
                return -1;
            }
            void *ptr = sy_record_tuple_copy((sy_record_tuple_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TUPLE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_TYPE)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "type");
                return -1;
            }
            void *ptr = sy_record_type_copy((sy_record_type_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_TYPE;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_STRUCT)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "struct");
                return -1;
            }
            void *ptr = sy_record_struct_copy((sy_record_struct_t *)(right->value));
            if (ptr == ERROR)
            {
                return -1;
            }
            sy_memory_free(left->value);
            left->value = ptr;
            left->kind = RECORD_KIND_STRUCT;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NULL)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "null");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NULL;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_UNDEFINED)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "undefined");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_UNDEFINED;


            return 0;
        }
        else
        if (right->kind == RECORD_KIND_NAN)
        {
            if (left->typed == 1)
            {
                sy_error_type_by_node(node, "mismatch type: '%s' and '%s'", "nan", "nan");
                return -1;
            }
            sy_memory_free(left->value);
            left->value = NULL;
            left->kind = RECORD_KIND_NAN;


            return 0;
        }

        return 0;
    }

    return 0;
}

int32_t
sy_execute_assign(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        if (sy_execute_set_value(node, left, right) < 0)
        {
            left->link -= 1;
            right->link -= 1;
            return -1;
        }
        
        left->link -= 1;
        right->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_ADD_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_plus(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;

            left->link -= 1;

            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SUB_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_minus(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_MUL_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_mul(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_DIV_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_plus(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_EPI_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_epi(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_MOD_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_mod(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;
        return 0;
    }
    else
    if (node->kind == NODE_KIND_POW_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_pow(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SHL_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_shl(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SHR_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_shr(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_AND_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_and(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;

            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;

        return 0;
    }
    else
    if (node->kind == NODE_KIND_OR_ASSIGN)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return -1;
        }

        sy_record_t *right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return -1;
        }

        sy_record_t *result = sy_execute_or(node, left, right, applicant);
        if (result == ERROR)
        {
            right->link -= 1;
            left->link -= 1;
            return -1;
        }

        right->link -= 1;

        if (sy_execute_set_value(node, left, result) < 0)
        {
            left->link -= 1;
            return -1;
        }

        left->link -= 1;
        result->link -= 1;
        return 0;
    }
    else
    {
        sy_record_t *value = sy_execute_expression(node, strip, applicant, origin);
        if (value == ERROR)
        {
            return -1;
        }

        value->link -= 1;

        return 0;
    }
}