#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>
#include <assert.h>

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

static const char * const symbols[] = {
  [RECORD_KIND_INT8]        = "instance int8",
  [RECORD_KIND_INT16]       = "instance int16",
  [RECORD_KIND_INT32]       = "instance int32",
  [RECORD_KIND_INT64]       = "instance int64",
  [RECORD_KIND_UINT8]       = "instance uint8",
  [RECORD_KIND_UINT16]      = "instance uint16",
  [RECORD_KIND_UINT32]      = "instance uint32",
  [RECORD_KIND_UINT64]      = "instance uint64",
  [RECORD_KIND_BIGINT]      = "instance bigint",
  [RECORD_KIND_FLOAT32]     = "instance float32",
  [RECORD_KIND_FLOAT64]     = "instance float64",
  [RECORD_KIND_BIGFLOAT]    = "instance bigfloat",
  [RECORD_KIND_CHAR]        = "instance char",
  [RECORD_KIND_STRING]      = "instance string",
  [RECORD_KIND_OBJECT]      = "instance object",
  [RECORD_KIND_TUPLE]       = "instance tuple",
  [RECORD_KIND_TYPE]        = "type",
  [RECORD_KIND_STRUCT]      = "struct",
  [RECORD_KIND_NULL]        = "null",
  [RECORD_KIND_UNDEFINED]   = "undefined",
  [RECORD_KIND_NAN]         = "nan"
};

const char *
sy_record_type_as_string(sy_record_t *record)
{
    return symbols[record->kind];
}

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
    record->link = 0;
    record->readonly = 0;
    record->reference = 0;

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
sy_record_make_struct(sy_node_t *type, sy_strip_t *value)
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
sy_record_make_type(sy_node_t *type, sy_strip_t *value)
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

sy_record_t *
sy_record_make_undefined()
{
    return sy_record_create(RECORD_KIND_UNDEFINED, NULL);
}

sy_record_t *
sy_record_make_nan()
{
    return sy_record_create(RECORD_KIND_NAN, NULL);
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

    object->value->link = 0;
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

    tuple->value->link = 0;

    if (sy_record_destroy(tuple->value) < 0)
    {
        return -1;
    }

    sy_memory_free(tuple);

    return 0;
}

int32_t
sy_record_struct_destroy(sy_record_struct_t *struct1)
{
    if (struct1->value)
    {
        if (sy_strip_destroy(struct1->value) < 0)
        {
            return -1;
        }
    }

    sy_memory_free(struct1);

    return 0;
}

int32_t
sy_record_type_destroy(sy_record_type_t *type)
{
    if (type->type->kind == NODE_KIND_OBJECT)
    {
        if (sy_record_object_destroy((sy_record_object_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else
    if (type->type->kind == NODE_KIND_ARRAY)
    {
        if (sy_record_destroy((sy_record_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else
    if (type->type->kind == NODE_KIND_TUPLE)
    {
        if (sy_record_tuple_destroy((sy_record_tuple_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else
    if (type->type->kind == NODE_KIND_CLASS)
    {
        if (sy_strip_destroy((sy_strip_t *)type->value) < 0)
        {
            return -1;
        }
    }

    sy_memory_free(type);

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

    record_copy->link = 1;

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

    record_copy->link = 1;

    basic->value = record_copy;
    basic->next = next;

    return basic;
}

sy_record_struct_t *
sy_record_struct_copy(sy_record_struct_t *struct1)
{
    sy_strip_t *value = NULL;

    if (struct1->value)
    {
        value = sy_strip_copy(struct1->value);
        if (value == ERROR)
        {
            return ERROR;
        }
    }

    sy_record_struct_t *basic = (sy_record_struct_t *)sy_memory_calloc(1, sizeof(sy_record_struct_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        if (value)
        {
            if (sy_strip_destroy(value) < 0)
            {
                return ERROR;
            }
        }

        return ERROR;
    }

    basic->type = struct1->type;
    basic->value = value;

    return basic;
}

sy_record_type_t *
sy_record_type_copy(sy_record_type_t *type)
{
    void *value = NULL;

    if (type->type->kind == NODE_KIND_OBJECT)
    {
        if (type->value)
        {
            value = sy_record_object_copy((sy_record_object_t *)type->value);
            if (value == ERROR)
            {
                return ERROR;
            }
        }
    }
    else
    if (type->type->kind == NODE_KIND_ARRAY)
    {
        if (type->value)
        {
            value = sy_record_copy((sy_record_t *)type->value);
            if (value == ERROR)
            {
                return ERROR;
            }
        }
    }
    else
    if (type->type->kind == NODE_KIND_TUPLE)
    {
        if (type->value)
        {
            value = sy_record_tuple_copy((sy_record_tuple_t *)type->value);
            if (value == ERROR)
            {
                return ERROR;
            }
        }
    }
    else
    if (type->type->kind == NODE_KIND_CLASS)
    {
        if (type->value)
        {
            value = sy_strip_copy((sy_strip_t *)type->value);
            if (value == ERROR)
            {
                return ERROR;
            }
        }
    }

    sy_record_type_t *basic = (sy_record_type_t *)sy_memory_calloc(1, sizeof(sy_record_type_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        if (type->type->kind == NODE_KIND_OBJECT)
        {
            if (sy_record_object_destroy((sy_record_object_t *)value) < 0)
            {
                return ERROR;
            }
        }
        else
        if (type->type->kind == NODE_KIND_ARRAY)
        {
            if (sy_record_destroy((sy_record_t *)value) < 0)
            {
                return ERROR;
            }
        }
        else
        if (type->type->kind == NODE_KIND_TUPLE)
        {
            if (sy_record_tuple_destroy((sy_record_tuple_t *)value) < 0)
            {
                return ERROR;
            }
        }
        else
        if (type->type->kind == NODE_KIND_CLASS)
        {
            if (sy_strip_destroy((sy_strip_t *)value) < 0)
            {
                return ERROR;
            }
        }

        return ERROR;
    }

    basic->type = type->type;
    basic->value = value;

    return basic;
}



sy_record_t *
sy_record_copy(sy_record_t *record)
{
    assert (record != NULL);

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
        sy_record_struct_t *basic = sy_record_struct_copy((sy_record_struct_t *)record->value);;
        if (basic == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record_copy = sy_record_create(RECORD_KIND_STRUCT, basic);
        if (record_copy == ERROR)
        {
            if (sy_record_struct_destroy(basic) < 0)
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
        sy_record_type_t *basic = sy_record_type_copy((sy_record_type_t *)record->value);;
        if (basic == ERROR)
        {
            return ERROR;
        }

        sy_record_t *record_copy = sy_record_create(RECORD_KIND_TYPE, basic);
        if (record_copy == ERROR)
        {
            if (sy_record_type_destroy(basic) < 0)
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
    else
    if (record->kind == RECORD_KIND_UNDEFINED)
    {
        return sy_record_make_undefined();
    }
    else
    if (record->kind == RECORD_KIND_NAN)
    {
        return sy_record_make_nan();
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

    if (record->link > 0)
    {
        if (NULL == sy_garbage_push(record))
        {
            return -1;
        }
        return 0;
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
        if (sy_record_type_destroy(type) < 0)
        {
            return -1;
        }
        sy_memory_free(record);
    }
    else
    if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        if (sy_record_struct_destroy(struct1) < 0)
        {
            return -1;
        }
        sy_memory_free(record);
    }
    else
    {
        if (record->kind == RECORD_KIND_BIGINT)
        {
            mpz_clear(*(mpz_t *)(record->value));
        }
        else
        if (record->kind == RECORD_KIND_BIGFLOAT)
        {
            mpf_clear(*(mpf_t *)(record->value));
        }
        else
        if (record->kind == RECORD_KIND_NULL)
        {
        }
        else
        if (record->kind == RECORD_KIND_UNDEFINED)
        {
        }
        else
        if (record->kind == RECORD_KIND_NAN)
        {
        }
        else
        {
            sy_memory_free(record->value);
        }
        sy_memory_free(record);
    }

    return 0;
}


