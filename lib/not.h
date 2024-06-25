#pragma once
#ifndef __NOT_H__
#define __NOT_H__ 1

#include <stdint.h>
#include <stdbool.h>

#define NOT_PTR_ERROR ((void *)(int *)-1)

typedef struct not_record
{
    uint8_t reference : 1;
    uint8_t readonly : 1;
    uint8_t typed : 1;
    uint8_t null : 1;
    uint8_t reserved : 4;
    int64_t link;
    uint8_t kind;
    void *value;
} not_record_t;

typedef enum not_record_kind
{
    RECORD_KIND_INT,
    RECORD_KIND_FLOAT,
    RECORD_KIND_CHAR,
    RECORD_KIND_STRING,
    RECORD_KIND_OBJECT,
    RECORD_KIND_TUPLE,
    RECORD_KIND_TYPE,
    RECORD_KIND_STRUCT,
    RECORD_KIND_NULL,
    RECORD_KIND_UNDEFINED,
    RECORD_KIND_NAN,
    RECORD_KIND_PROC,
    RECORD_KIND_BUILTIN
} not_record_kind_t;

typedef struct not_record_object
{
    char *key;
    not_record_t *value;

    struct not_record_object *next;
} not_record_object_t;

typedef struct not_record_tuple
{
    not_record_t *value;

    struct not_record_tuple *next;
} not_record_tuple_t;

not_record_t *
not_record_create(uint64_t kind, void *value);

int32_t
not_record_link_decrease(not_record_t *record);

void not_record_link_increase(not_record_t *record);

not_record_t *not_record_make_null();

not_record_t *
not_record_make_undefined();

not_record_t *
not_record_make_nan();

not_record_tuple_t *
not_record_make_tuple(not_record_t *value, not_record_tuple_t *next);

not_record_object_t *
not_record_make_object(const char *key, not_record_t *value, not_record_object_t *next);

not_record_t *
not_record_make_string(char *value);

not_record_t *
not_record_make_char(char value);

not_record_t *
not_record_make_float_from_ui(uint64_t value);

not_record_t *
not_record_make_float_from_si(int64_t value);

not_record_t *
not_record_make_float_from_f(mpf_t value);

not_record_t *
not_record_make_float_from_z(mpz_t value);

not_record_t *
not_record_make_float_from_d(double value);

not_record_t *
not_record_make_float(const char *value);

not_record_t *
not_record_make_int_from_ui(uint64_t value);

not_record_t *
not_record_make_int_from_si(int64_t value);

not_record_t *
not_record_make_int_from_z(mpz_t value);

not_record_t *
not_record_make_int_from_f(mpf_t value);

not_record_t *
not_record_make_int(const char *value);

int32_t
not_record_destroy(not_record_t *record);

int32_t
not_record_tuple_destroy(not_record_tuple_t *tuple);

int32_t
not_record_object_destroy(not_record_object_t *object);

not_record_t *
not_record_tuple_arg_by_index(not_record_t *args, size_t index);

#endif