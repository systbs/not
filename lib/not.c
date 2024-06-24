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

#include "not.h"

not_record_t *
not_record_create(uint64_t kind, void *value)
{
    not_record_t *record = (not_record_t *)calloc(1, sizeof(not_record_t));
    if (record == NOT_PTR_NULL)
    {
        return NOT_PTR_ERROR;
    }

    record->kind = kind;
    record->value = value;
    record->link = 1;

    return record;
}

not_record_t *
not_record_make_int(const char *value)
{
    mpz_t *basic = (mpz_t *)calloc(1, sizeof(mpz_t));
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
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
    if (basic == NOT_PTR_NULL)
    {
        return NOT_PTR_ERROR;
    }
    strcpy(basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_STRING, basic);
    if (record == NOT_PTR_ERROR)
    {
        free(basic);
        free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_object_t *
not_record_make_object(const char *key, not_record_t *value, not_record_object_t *next)
{
    not_record_object_t *basic = (not_record_object_t *)calloc(1, sizeof(not_record_object_t));
    if (basic == NOT_PTR_NULL)
    {
        return NOT_PTR_ERROR;
    }

    size_t length = strlen(key);
    basic->key = calloc(length, sizeof(char) + 1);
    if (basic->key == NOT_PTR_ERROR)
    {

        free(basic);
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
    if (basic == NOT_PTR_NULL)
    {
        return NOT_PTR_ERROR;
    }

    basic->value = value;
    basic->next = next;

    return basic;
}

not_record_t *
not_record_make_null()
{
    return not_record_create(RECORD_KIND_NULL, NOT_PTR_NULL);
}

not_record_t *
not_record_make_undefined()
{
    return not_record_create(RECORD_KIND_UNDEFINED, NOT_PTR_NULL);
}

not_record_t *
not_record_make_nan()
{
    return not_record_create(RECORD_KIND_NAN, NOT_PTR_NULL);
}

not_record_t *
tuple_arg_by_index(not_record_t *args, size_t index)
{
    size_t i = 0;
    for (not_record_tuple_t *tuple = (not_record_tuple_t *)args->value; tuple != NOT_PTR_NULL; tuple = tuple->next)
    {
        if (i == index)
        {
            return tuple->value;
        }
        i += 1;
    }

    return NOT_PTR_NULL;
}