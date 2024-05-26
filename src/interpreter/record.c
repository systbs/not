#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../parser/syntax/syntax.h"
#include "../error.h"
#include "../memory.h"
#include "../mutex.h"
#include "record.h"
#include "strip.h"
#include "garbage.h"

sy_record_t *
sy_record_create(uint64_t kind, void *value)
{
    sy_record_t *record = (sy_record_t *)sy_memory_calloc(1, sizeof(sy_record_t));
    if (record == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    record->kind = kind;
    record->value = value;

    return record;
}

sy_record_t *
sy_record_make_int8(int8_t value)
{
    int8_t *basic = (int8_t *)sy_memory_calloc(1, sizeof(int8_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_INT8, basic);
}

sy_record_t *
sy_record_make_int16(int16_t value)
{
    int16_t *basic = (int16_t *)sy_memory_calloc(1, sizeof(int16_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_INT16, basic);
}

sy_record_t *
sy_record_make_int32(int32_t value)
{
    int32_t *basic = (int32_t *)sy_memory_calloc(1, sizeof(int32_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_INT32, basic);
}

sy_record_t *
sy_record_make_int64(int64_t value)
{
    int64_t *basic = (int64_t *)sy_memory_calloc(1, sizeof(int64_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_INT64, basic);
}

sy_record_t *
sy_record_make_uint8(uint8_t value)
{
    uint8_t *basic = (uint8_t *)sy_memory_calloc(1, sizeof(uint8_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_UINT8, basic);
}

sy_record_t *
sy_record_make_uint16(uint16_t value)
{
    uint16_t *basic = (uint16_t *)sy_memory_calloc(1, sizeof(uint16_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_UINT16, basic);
}

sy_record_t *
sy_record_make_uint32(uint32_t value)
{
    uint32_t *basic = (uint32_t *)sy_memory_calloc(1, sizeof(uint32_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_UINT32, basic);
}

sy_record_t *
sy_record_make_uint64(uint64_t value)
{
    uint64_t *basic = (uint64_t *)sy_memory_calloc(1, sizeof(uint64_t));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_UINT64, basic);
}

sy_record_t *
sy_record_make_bigint(const char *value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpz_init(*basic);

    mpz_set_str(*basic, value, 10);

    return sy_record_create(RECORD_KIND_BIGINT, basic);
}

sy_record_t *
sy_record_make_bigint_from_ui(uint64_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpz_init(*basic);

    mpz_set_ui(*basic, value);

    return sy_record_create(RECORD_KIND_BIGINT, basic);
}

sy_record_t *
sy_record_make_bigint_from_si(int64_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpz_init(*basic);

    mpz_set_si(*basic, value);

    return sy_record_create(RECORD_KIND_BIGINT, basic);
}

sy_record_t *
sy_record_make_bigint_from_mpz(mpz_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpz_init_set(*basic, value);

    return sy_record_create(RECORD_KIND_BIGINT, basic);
}

sy_record_t *
sy_record_make_float32(float value)
{
    float *basic = (float *)sy_memory_calloc(1, sizeof(float));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_FLOAT32, basic);
}

sy_record_t *
sy_record_make_float64(double value)
{
    double *basic = (double *)sy_memory_calloc(1, sizeof(double));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_FLOAT64, basic);
}

sy_record_t *
sy_record_make_bigfloat(const char *value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpf_init2(*basic, 256);
    mpf_set_str(*basic, value, 10);

    return sy_record_create(RECORD_KIND_BIGFLOAT, basic);
}

sy_record_t *
sy_record_make_bigfloat_from_d(double value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpf_init2(*basic, 256);
    mpf_set_d(*basic, value);

    return sy_record_create(RECORD_KIND_BIGFLOAT, basic);
}

sy_record_t *
sy_record_make_bigfloat_from_mpf(mpf_t value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        return NULL;
    }

    mpf_init_set(*basic, value);

    return sy_record_create(RECORD_KIND_BIGFLOAT, basic);
}

sy_record_t *
sy_record_make_char(char value)
{
    char *basic = (char *)sy_memory_calloc(1, sizeof(char));
    if (basic == NULL)
    {
        return NULL;
    }

    *basic = value;

    return sy_record_create(RECORD_KIND_CHAR, basic);
}

sy_record_t *
sy_record_make_string(char *value)
{
    char **basic = (char **)sy_memory_calloc(1, sizeof(char *));
    if (basic == NULL)
    {
        return NULL;
    }

    char *str = (char *)sy_memory_calloc(1, strlen(value));
    if (str == NULL)
    {
        return NULL;
    }
    strcpy(str, value);

    *basic = str;

    return sy_record_create(RECORD_KIND_STRING, basic);
}

sy_record_t *
sy_record_make_object(sy_node_t *key, sy_record_t *value, sy_record_object_t *next)
{
    sy_record_object_t *basic = (sy_record_object_t *)sy_memory_calloc(1, sizeof(sy_record_object_t));
    if (basic == NULL)
    {
        return NULL;
    }

    basic->key = key;
    basic->value = value;
    basic->next = next;

    return sy_record_create(RECORD_KIND_OBJECT, basic);
}

sy_record_t *
sy_record_make_tuple(sy_record_t *value, sy_record_tuple_t *next)
{
    sy_record_tuple_t *basic = (sy_record_tuple_t *)sy_memory_calloc(1, sizeof(sy_record_tuple_t));
    if (basic == NULL)
    {
        return NULL;
    }

    basic->value = value;
    basic->next = next;

    return sy_record_create(RECORD_KIND_TUPLE, basic);
}

sy_record_t *
sy_record_make_struct(sy_node_t *type, strip_t *value)
{
    sy_record_struct_t *basic = (sy_record_struct_t *)sy_memory_calloc(1, sizeof(sy_record_struct_t));
    if (basic == NULL)
    {
        return NULL;
    }

    basic->type = type;
    basic->value = value;

    return sy_record_create(RECORD_KIND_STRUCT, basic);
}

sy_record_t *
sy_record_make_type(sy_node_t *type, strip_t *value)
{
    sy_record_type_t *basic = (sy_record_type_t *)sy_memory_calloc(1, sizeof(sy_record_type_t));
    if (basic == NULL)
    {
        return NULL;
    }

    basic->type = type;
    basic->value = value;

    return sy_record_create(RECORD_KIND_TYPE, basic);
}

sy_record_t *
sy_record_make_null()
{
    return sy_record_create(RECORD_KIND_NULL, NULL);
}

sy_record_object_t *
sy_record_object_copy(sy_record_object_t *object)
{
    sy_record_t *sy_record_value1 = sy_record_copy(object->value);
    if (sy_record_value1 == NULL)
    {
        return NULL;
    }

    sy_record_object_t *next = NULL;
    if (object->next != NULL)
    {
        next = sy_record_object_copy(object->next);
        if (next == NULL)
        {
            return NULL;
        }
    }

    sy_record_object_t *basic = (sy_record_object_t *)sy_memory_calloc(1, sizeof(sy_record_object_t));
    if (basic == NULL)
    {
        return NULL;
    }

    basic->key = object->key;
    basic->value = sy_record_value1;
    basic->next = next;

    return basic;
}

sy_record_tuple_t *
sy_record_tuple_copy(sy_record_tuple_t *tuple)
{
    sy_record_t *sy_record_value1 = sy_record_copy(tuple->value);
    if (sy_record_value1 == NULL)
    {
        return NULL;
    }

    sy_record_tuple_t *next = NULL;
    if (tuple->next != NULL)
    {
        next = sy_record_tuple_copy(tuple->next);
        if (next == NULL)
        {
            return NULL;
        }
    }

    sy_record_tuple_t *basic = (sy_record_tuple_t *)sy_memory_calloc(1, sizeof(sy_record_tuple_t));
    if (basic == NULL)
    {
        return NULL;
    }

    basic->value = sy_record_value1;
    basic->next = next;

    return basic;
}

sy_record_t *
sy_record_copy(sy_record_t *record)
{
    if (!record)
    {
        return record;
    }

    if (record->kind == RECORD_KIND_INT8)
    {
        return sy_record_make_int8(*(int8_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_INT16)
    {
        return sy_record_make_int16(*(int16_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_INT32)
    {
        return sy_record_make_int32(*(int32_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_INT64)
    {
        return sy_record_make_int64(*(int64_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_UINT8)
    {
        return sy_record_make_uint8(*(uint8_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_UINT16)
    {
        return sy_record_make_uint16(*(uint16_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_UINT32)
    {
        return sy_record_make_uint32(*(uint32_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_UINT64)
    {
        return sy_record_make_uint64(*(uint64_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_BIGINT)
    {
        return sy_record_make_bigint_from_mpz(*(mpz_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_FLOAT32)
    {
        return sy_record_make_float32(*(float *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_FLOAT64)
    {
        return sy_record_make_float64(*(double *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_BIGFLOAT)
    {
        return sy_record_make_bigfloat_from_mpf(*(mpf_t *)(record->value));
    }
    else
    if (record->kind == RECORD_KIND_OBJECT)
    {
        sy_record_object_t *basic = sy_record_object_copy((sy_record_object_t *)record->value);
        if (basic == NULL)
        {
            return NULL;
        }

        return sy_record_create(RECORD_KIND_OBJECT, basic);
    }
    else
    if (record->kind == RECORD_KIND_TUPLE)
    {
        sy_record_tuple_t *basic = sy_record_tuple_copy((sy_record_tuple_t *)record->value);
        if (basic == NULL)
        {
            return NULL;
        }

        return sy_record_create(RECORD_KIND_TUPLE, basic);
    }
    else
    if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        strip_t *ar1 = sy_strip_copy(struct1->value);
        if (ar1 == NULL)
        {
            return NULL;
        }

        return sy_record_make_struct(struct1->type, ar1);
    }
    else
    if (record->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *type1 = (sy_record_type_t *)record->value;
        strip_t *ar1 = sy_strip_copy(type1->value);
        if (ar1 == NULL)
        {
            return NULL;
        }

        return sy_record_make_type(type1->type, ar1);
    }
    else
    if (record->kind == RECORD_KIND_NULL)
    {
        return sy_record_make_null();
    }

    return NULL;
}

int32_t
sy_record_object_destroy(sy_record_object_t *object)
{
    if (object->next)
    {
        if (sy_record_object_destroy(object->next) < 0)
        {
            sy_error_system("'%s' could not destroy an object", "sy_record");
            return -1;
        }
        object->next = NULL;
    }

    if (sy_record_destroy(object->value) < 0)
    {
        sy_error_system("'%s' could not destroy a record", "sy_record");
        return -1;
    }

    sy_memory_free(object);

    return 0;
}

int32_t
sy_record_tuple_destroy(sy_record_tuple_t *tuple)
{
    if (tuple->next)
    {
        if (sy_record_tuple_destroy(tuple->next) < 0)
        {
            sy_error_system("'%s' could not destroy a tuple", "sy_record");
            return -1;
        }
        tuple->next = NULL;
    }

    if (sy_record_destroy(tuple->value) < 0)
    {
        sy_error_system("'%s' could not destroy a record", "sy_record");
        return -1;
    }

    sy_memory_free(tuple);

    return 0;
}

int32_t
sy_record_destroy(sy_record_t *record)
{
    if (!record)
    {
        return 0;
    }

    if (record->kind == RECORD_KIND_OBJECT)
    {
        sy_record_object_t *object = (sy_record_object_t *)record->value;
        if (sy_record_object_destroy(object) < 0)
        {
            sy_error_system("'%s' could not destroy an object", "sy_record");
            return -1;
        }
        sy_memory_free(record);
    }
    else
    if (record->kind == RECORD_KIND_TUPLE)
    {
        sy_record_tuple_t *tuple = (sy_record_tuple_t *)record->value;
        if (sy_record_tuple_destroy(tuple) < 0)
        {
            sy_error_system("'%s' could not destroy a tuple", "sy_record");
            return -1;
        }
        sy_memory_free(record);
    }
    else
    if (record->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *type = (sy_record_type_t *)record->value;
        if (sy_strip_destroy(type->value) < 0)
        {
            sy_error_system("'%s' could not destroy a type", "sy_record");
            return -1;
        }
        sy_memory_free(record->value);
        sy_memory_free(record);
    }
    else
    if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        if (sy_strip_destroy(struct1->value) < 0)
        {
            sy_error_system("'%s' could not destroy a struct", "sy_record");
            return -1;
        }
        sy_memory_free(record->value);
        sy_memory_free(record);
    }
    else
    {
        if (record->kind != RECORD_KIND_NULL)
        {
            sy_memory_free(record->value);
        }
        sy_memory_free(record);
    }

    return 0;
}