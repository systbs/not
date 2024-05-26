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
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_INT8, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int16(int16_t value)
{
    int16_t *basic = (int16_t *)sy_memory_calloc(1, sizeof(int16_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_INT16, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int32(int32_t value)
{
    int32_t *basic = (int32_t *)sy_memory_calloc(1, sizeof(int32_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_INT32, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int64(int64_t value)
{
    int64_t *basic = (int64_t *)sy_memory_calloc(1, sizeof(int64_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_INT64, basic);
    if (record == ERROR)
    {
        sy_error_system("'%s' could not make '%s'", "sy_record", "RECORD_KIND_INT64");
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_uint8(uint8_t value)
{
    uint8_t *basic = (uint8_t *)sy_memory_calloc(1, sizeof(uint8_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_UINT8, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_uint16(uint16_t value)
{
    uint16_t *basic = (uint16_t *)sy_memory_calloc(1, sizeof(uint16_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_UINT16, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_uint32(uint32_t value)
{
    uint32_t *basic = (uint32_t *)sy_memory_calloc(1, sizeof(uint32_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_UINT32, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_uint64(uint64_t value)
{
    uint64_t *basic = (uint64_t *)sy_memory_calloc(1, sizeof(uint64_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_UINT64, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigint(const char *value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);

    mpz_set_str(*basic, value, 10);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGINT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigint_from_ui(uint64_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);

    mpz_set_ui(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGINT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigint_from_si(int64_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);

    mpz_set_si(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGINT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigint_from_mpz(mpz_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init_set(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGINT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float32(float value)
{
    float *basic = (float *)sy_memory_calloc(1, sizeof(float));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT32, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float64(double value)
{
    double *basic = (double *)sy_memory_calloc(1, sizeof(double));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT64, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigfloat(const char *value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init2(*basic, 256);
    mpf_set_str(*basic, value, 10);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGFLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigfloat_from_d(double value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init2(*basic, 256);
    mpf_set_d(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGFLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_bigfloat_from_mpf(mpf_t value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init_set(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_BIGFLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_char(char value)
{
    char *basic = (char *)sy_memory_calloc(1, sizeof(char));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    *basic = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_CHAR, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_string(char *value)
{
    char **basic = (char **)sy_memory_calloc(1, sizeof(char *));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    char *str = (char *)sy_memory_calloc(1, strlen(value));
    if (str == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }
    strcpy(str, value);

    *basic = str;

    sy_record_t *record = sy_record_create(RECORD_KIND_STRING, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_object(sy_node_t *key, sy_record_t *value, sy_record_object_t *next)
{
    sy_record_object_t *basic = (sy_record_object_t *)sy_memory_calloc(1, sizeof(sy_record_object_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    basic->key = key;
    basic->value = value;
    basic->next = next;

    sy_record_t *record = sy_record_create(RECORD_KIND_OBJECT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_tuple(sy_record_t *value, sy_record_tuple_t *next)
{
    sy_record_tuple_t *basic = (sy_record_tuple_t *)sy_memory_calloc(1, sizeof(sy_record_tuple_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    basic->value = value;
    basic->next = next;

    sy_record_t *record = sy_record_create(RECORD_KIND_TUPLE, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_struct(sy_node_t *type, strip_t *value)
{
    sy_record_struct_t *basic = (sy_record_struct_t *)sy_memory_calloc(1, sizeof(sy_record_struct_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    basic->type = type;
    basic->value = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_STRUCT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_type(sy_node_t *type, strip_t *value)
{
    sy_record_type_t *basic = (sy_record_type_t *)sy_memory_calloc(1, sizeof(sy_record_type_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    basic->type = type;
    basic->value = value;

    sy_record_t *record = sy_record_create(RECORD_KIND_TYPE, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_null()
{
    return sy_record_create(RECORD_KIND_NULL, NULL);
}

int32_t
sy_record_object_destroy(sy_record_object_t *object)
{
    if (object->next)
    {
        if (sy_record_object_destroy(object->next) < 0)
        {
            return -1;
        }
        object->next = NULL;
    }

    object->value->reference = 0;
    if (sy_record_destroy(object->value) < 0)
    {
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
            return -1;
        }
        tuple->next = NULL;
    }

    tuple->value->reference = 0;

    if (sy_record_destroy(tuple->value) < 0)
    {
        return -1;
    }

    sy_memory_free(tuple);

    return 0;
}

sy_record_object_t *
sy_record_object_copy(sy_record_object_t *object)
{
    sy_record_object_t *next = NULL;
    if (object->next)
    {
        next = sy_record_object_copy(object->next);
        if (next == ERROR)
        {
            return ERROR;
        }
    }

    sy_record_object_t *basic = (sy_record_object_t *)sy_memory_calloc(1, sizeof(sy_record_object_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        if (next)
        {
            if (sy_record_object_destroy(next) < 0)
            {
                return ERROR;
            }
        }
        return ERROR;
    }

    sy_record_t *record_copy = sy_record_copy(object->value);
    if (record_copy == ERROR)
    {
        if (next)
        {
            if (sy_record_object_destroy(next) < 0)
            {
                return ERROR;
            }
        }
        sy_memory_free(basic);
        return ERROR;
    }

    record_copy->reference = 1;

    basic->key = object->key;
    basic->value = record_copy;
    basic->next = next;

    return basic;
}

sy_record_tuple_t *
sy_record_tuple_copy(sy_record_tuple_t *tuple)
{
    sy_record_tuple_t *next = NULL;
    if (tuple->next)
    {
        next = sy_record_tuple_copy(tuple->next);
        if (next == ERROR)
        {
            return ERROR;
        }
    }

    sy_record_tuple_t *basic = (sy_record_tuple_t *)sy_memory_calloc(1, sizeof(sy_record_tuple_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        if (next)
        {
            if (sy_record_tuple_destroy(next) < 0)
            {
                return ERROR;
            }
        }
        return ERROR;
    }

    sy_record_t *record_copy = sy_record_copy(tuple->value);
    if (record_copy == ERROR)
    {
        if (next)
        {
            if (sy_record_tuple_destroy(next) < 0)
            {
                return ERROR;
            }
        }
        sy_memory_free(basic);
        return ERROR;
    }

    record_copy->reference = 1;

    basic->value = record_copy;
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
        if (basic == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record_copy = sy_record_create(RECORD_KIND_OBJECT, basic);
        if (record_copy == ERROR)
        {
            if (sy_record_object_destroy(basic) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }
        return record_copy;
    }
    else
    if (record->kind == RECORD_KIND_TUPLE)
    {
        sy_record_tuple_t *basic = sy_record_tuple_copy((sy_record_tuple_t *)record->value);
        if (basic == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record_copy = sy_record_create(RECORD_KIND_TUPLE, basic);
        if (record_copy == ERROR)
        {
            if (sy_record_tuple_destroy(basic) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }
        return record_copy;
    }
    else
    if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        strip_t *strip = sy_strip_copy(struct1->value);
        if (strip == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record_copy = sy_record_make_struct(struct1->type, strip);
        if (record_copy == ERROR)
        {
            if (sy_strip_destroy(strip) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }
        return record_copy;
    }
    else
    if (record->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *type1 = (sy_record_type_t *)record->value;
        strip_t *strip = sy_strip_copy(type1->value);
        if (strip == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record_copy = sy_record_make_type(type1->type, strip);
        if (record_copy == ERROR)
        {
            if (sy_strip_destroy(strip) < 0)
            {
                return ERROR;
            }
            return ERROR;
        }
        return record_copy;
    }
    else
    if (record->kind == RECORD_KIND_NULL)
    {
        return sy_record_make_null();
    }

    return NULL;
}

int32_t
sy_record_destroy(sy_record_t *record)
{
    if (!record)
    {
        return 0;
    }

    if (record->reference > 0)
    {
        if (NULL == sy_garbage_push(record))
        {
            return -1;
        }
    }

    if (record->kind == RECORD_KIND_OBJECT)
    {
        sy_record_object_t *object = (sy_record_object_t *)record->value;
        if (sy_record_object_destroy(object) < 0)
        {
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

