#pragma once
#ifndef __RECORD_H__
#define __RECORD_H__ 1

typedef struct strip strip_t;

typedef struct record {
    uint8_t kind;
    void *value;
} sy_record_t;

typedef enum sy_record_kind {
    RECORD_KIND_INT8,
    RECORD_KIND_INT16,
    RECORD_KIND_INT32,
    RECORD_KIND_INT64,
    RECORD_KIND_UINT8,
    RECORD_KIND_UINT16,
    RECORD_KIND_UINT32,
    RECORD_KIND_UINT64,
    RECORD_KIND_BIGINT,
    RECORD_KIND_FLOAT32,
    RECORD_KIND_FLOAT64,
    RECORD_KIND_BIGFLOAT,
    RECORD_KIND_CHAR,
    RECORD_KIND_STRING,
    RECORD_KIND_OBJECT,
    RECORD_KIND_ARRAY,
    RECORD_KIND_TUPLE,
    RECORD_KIND_TYPE,
    RECORD_KIND_STRUCT,
    RECORD_KIND_NULL
} sy_record_kind_t;

typedef struct sy_record_struct {
    sy_node_t *type;
    strip_t *value;
} sy_record_struct_t;

typedef struct sy_record_type {
    sy_node_t *type;
    strip_t *value;
} sy_record_type_t;

typedef struct sy_record_object {
    sy_node_t *key;
    sy_record_t *value;

    struct sy_record_object *next;
} sy_record_object_t;

typedef struct sy_record_tuple {
    sy_record_t *value;

    struct sy_record_tuple *next;
} sy_record_tuple_t;


sy_record_t *
sy_record_copy(sy_record_t *record);

sy_record_t *
sy_record_make_null();

sy_record_t *
sy_record_make_undefined();

sy_record_t *
sy_record_make_type(sy_node_t *type, strip_t *value);

sy_record_t *
sy_record_make_struct(sy_node_t *type, strip_t *value);

sy_record_t *
sy_record_make_tuple(sy_record_t *value, sy_record_tuple_t *next);

sy_record_t *
sy_record_make_object(sy_node_t *key, sy_record_t *value, sy_record_object_t *next);

sy_record_t *
sy_record_make_string(char *value);

sy_record_t *
sy_record_make_char(char value);

sy_record_t *
sy_record_make_bigfloat_from_mpf(mpf_t value);

sy_record_t *
sy_record_make_bigfloat_from_d(double value);

sy_record_t *
sy_record_make_bigfloat(const char *value);

sy_record_t *
sy_record_make_float64(double value);

sy_record_t *
sy_record_make_float32(float value);

sy_record_t *
sy_record_make_bigint_from_ui(uint64_t value);

sy_record_t *
sy_record_make_bigint_from_si(int64_t value);

sy_record_t *
sy_record_make_bigint_from_mpz(mpz_t value);

sy_record_t *
sy_record_make_bigint(const char *value);

sy_record_t *
sy_record_make_uint64(uint64_t value);

sy_record_t *
sy_record_make_uint32(uint32_t value);

sy_record_t *
sy_record_make_uint16(uint16_t value);

sy_record_t *
sy_record_make_uint8(uint8_t value);

sy_record_t *
sy_record_make_int64(int64_t value);

sy_record_t *
sy_record_make_int32(int32_t value);

sy_record_t *
sy_record_make_int16(int16_t value);

sy_record_t *
sy_record_make_int8(int8_t value);

int32_t
sy_record_destroy(sy_record_t *record);

#endif