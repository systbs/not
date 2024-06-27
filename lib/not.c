#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <stdint.h>
#include <float.h>
#include <jansson.h>
#include <errno.h>

#include "not.h"

not_record_t *
not_record_create(uint64_t kind, void *value)
{
    not_record_t *record = (not_record_t *)calloc(1, sizeof(not_record_t));
    if (record == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    record->kind = kind;
    record->value = value;
    record->link = 1;

    return record;
}

void not_record_link_increase(not_record_t *record)
{
    record->link += 1;
}

int32_t
not_record_link_decrease(not_record_t *record)
{
    record->link -= 1;
    if (record->link == 0)
    {
        if (not_record_destroy(record) < 0)
        {
            return -1;
        }
    }
    return 0;
}

not_record_t *
not_record_make_int(const char *value)
{
    mpz_t *basic = (mpz_t *)calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);

    mpz_set_str(*basic, value, 10);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_ui(uint64_t value)
{
    mpz_t *basic = (mpz_t *)calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);

    mpz_set_ui(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_si(int64_t value)
{
    mpz_t *basic = (mpz_t *)calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);
    mpz_set_si(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_z(mpz_t value)
{
    mpz_t *basic = (mpz_t *)calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpz_init_set(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_f(mpf_t value)
{
    mpz_t *basic = (mpz_t *)calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);
    mpz_set_f(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float(const char *value)
{
    mpf_t *basic = (mpf_t *)calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_str(*basic, value, 10);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_d(double value)
{
    mpf_t *basic = (mpf_t *)calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_d(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_si(int64_t value)
{
    mpf_t *basic = (mpf_t *)calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_si(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_ui(uint64_t value)
{
    mpf_t *basic = (mpf_t *)calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_ui(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_f(mpf_t value)
{
    mpf_t *basic = (mpf_t *)calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpf_init_set(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_z(mpz_t value)
{
    mpf_t *basic = (mpf_t *)calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_z(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_char(char value)
{
    char *basic = (char *)calloc(1, sizeof(char));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    *basic = value;

    not_record_t *record = not_record_create(RECORD_KIND_CHAR, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_string(char *value)
{
    char *basic = (char *)calloc(1, strlen(value) + 1);
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }
    strcpy(basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_STRING, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_object_t *
not_record_make_object(const char *key, not_record_t *value, not_record_object_t *next)
{
    not_record_object_t *basic = (not_record_object_t *)calloc(1, sizeof(not_record_object_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    size_t length = strlen(key);
    basic->key = calloc(length + 1, sizeof(char));
    if (basic->key == NOT_PTR_ERROR)
    {
        free(basic);
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }
    strcpy(basic->key, key);

    basic->value = value;
    basic->next = next;

    return basic;
}

not_record_tuple_t *
not_record_make_tuple(not_record_t *value, not_record_tuple_t *next)
{
    not_record_tuple_t *basic = (not_record_tuple_t *)calloc(1, sizeof(not_record_tuple_t));
    if (basic == NULL)
    {
        errno = ENOMEM;
        return NOT_PTR_ERROR;
    }

    basic->value = value;
    basic->next = next;

    return basic;
}

not_record_t *
not_record_make_null()
{
    return not_record_create(RECORD_KIND_NULL, NULL);
}

not_record_t *
not_record_make_undefined()
{
    return not_record_create(RECORD_KIND_UNDEFINED, NULL);
}

not_record_t *
not_record_make_nan()
{
    return not_record_create(RECORD_KIND_NAN, NULL);
}

int32_t
not_record_object_destroy(not_record_object_t *object)
{
    if (!object)
    {
        return 0;
    }

    if (object->next)
    {
        if (not_record_object_destroy(object->next) < 0)
        {
            return -1;
        }
        object->next = NULL;
    }

    if (not_record_link_decrease(object->value) < 0)
    {
        return -1;
    }

    free(object->key);
    free(object);

    return 0;
}

int32_t
not_record_tuple_destroy(not_record_tuple_t *tuple)
{
    if (!tuple)
    {
        return 0;
    }

    if (tuple->next)
    {
        if (not_record_tuple_destroy(tuple->next) < 0)
        {
            return -1;
        }
        tuple->next = NULL;
    }

    if (not_record_link_decrease(tuple->value) < 0)
    {
        return -1;
    }

    free(tuple);

    return 0;
}

int32_t
not_record_destroy(not_record_t *record)
{
    if (record->kind == RECORD_KIND_OBJECT)
    {
        not_record_object_t *object = (not_record_object_t *)record->value;
        if (not_record_object_destroy(object) < 0)
        {
            return -1;
        }
        free(record);
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        not_record_tuple_t *tuple = (not_record_tuple_t *)record->value;
        if (not_record_tuple_destroy(tuple) < 0)
        {
            return -1;
        }
        free(record);
    }
    else if (record->kind == RECORD_KIND_TYPE)
    {
    }
    else if (record->kind == RECORD_KIND_STRUCT)
    {
    }
    else if (record->kind == RECORD_KIND_BUILTIN)
    {
    }
    else
    {
        if (record->kind == RECORD_KIND_INT)
        {
            mpz_clear(*(mpz_t *)(record->value));
        }
        else if (record->kind == RECORD_KIND_FLOAT)
        {
            mpf_clear(*(mpf_t *)(record->value));
        }
        else if (record->kind == RECORD_KIND_NULL)
        {
        }
        else if (record->kind == RECORD_KIND_UNDEFINED)
        {
        }
        else if (record->kind == RECORD_KIND_NAN)
        {
        }
        else if (record->reference != 1)
        {
            free(record->value);
        }
        free(record);
    }

    return 0;
}

not_record_t *
not_record_tuple_arg_by_index(not_record_t *args, size_t index)
{
    size_t i = 0;
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)args->value; tuple != NULL; tuple = tuple->next)
    {
        if (i == index)
        {
            return tuple->value;
        }
        i += 1;
    }

    return NULL;
}
