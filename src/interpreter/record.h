#pragma once
#ifndef __RECORD_H__
#define __RECORD_H__ 1

typedef struct sy_strip sy_strip_t;
typedef struct sy_entry sy_entry_t;

typedef struct sy_record {
    uint8_t reference:1;
    uint8_t readonly:1;
    uint8_t typed:1;
    uint8_t reserved:5;
    int64_t link;
    uint8_t kind;
    void *value;
} sy_record_t;

typedef enum sy_record_kind {
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
    RECORD_KIND_NAN
} sy_record_kind_t;

typedef struct sy_record_struct {
    sy_node_t *type;
    sy_strip_t *value;
} sy_record_struct_t;

typedef struct sy_record_type {
    sy_node_t *type;
    void *value;
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

const char *
sy_record_type_as_string(sy_record_t *record);

sy_record_t *
sy_record_create(uint64_t kind, void *value);

sy_record_t *
sy_record_copy(sy_record_t *record);

sy_record_t *
sy_record_make_null();

sy_record_t *
sy_record_make_undefined();

sy_record_t *
sy_record_make_nan();

sy_record_t *
sy_record_make_type(sy_node_t *type, void *value);

sy_record_t *
sy_record_make_struct(sy_node_t *type, sy_strip_t *value);

sy_record_tuple_t *
sy_record_make_tuple(sy_record_t *value, sy_record_tuple_t *next);

sy_record_object_t *
sy_record_make_object(sy_node_t *key, sy_record_t *value, sy_record_object_t *next);

sy_record_t *
sy_record_make_string(char *value);

sy_record_t *
sy_record_make_char(char value);

sy_record_t *
sy_record_make_float_from_ui(uint64_t value);

sy_record_t *
sy_record_make_float_from_si(int64_t value);

sy_record_t *
sy_record_make_float_from_f(mpf_t value);

sy_record_t *
sy_record_make_float_from_z(mpz_t value);

sy_record_t *
sy_record_make_float_from_d(double value);

sy_record_t *
sy_record_make_float(const char *value);

sy_record_t *
sy_record_make_float64(double value);

sy_record_t *
sy_record_make_float32(float value);

sy_record_t *
sy_record_make_int_from_ui(uint64_t value);

sy_record_t *
sy_record_make_int_from_si(int64_t value);

sy_record_t *
sy_record_make_int_from_z(mpz_t value);

sy_record_t *
sy_record_make_int_from_f(mpf_t value);

sy_record_t *
sy_record_make_int(const char *value);

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

void
sy_record_link_inc(sy_record_t *record);

void
sy_record_link_dec(sy_record_t *record);

int32_t
sy_record_destroy(sy_record_t *record);

sy_record_tuple_t *
sy_record_tuple_copy(sy_record_tuple_t *tuple);

sy_record_object_t *
sy_record_object_copy(sy_record_object_t *object);

int32_t
sy_record_tuple_destroy(sy_record_tuple_t *tuple);

int32_t
sy_record_object_destroy(sy_record_object_t *object);

sy_record_type_t *
sy_record_type_copy(sy_record_type_t *type);

sy_record_struct_t *
sy_record_struct_copy(sy_record_struct_t *struct1);

int32_t
sy_record_type_destroy(sy_record_type_t *type);

int32_t
sy_record_struct_destroy(sy_record_struct_t *struct1);

#endif