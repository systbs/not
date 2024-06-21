#pragma once
#ifndef __RECORD_H__
#define __RECORD_H__ 1

typedef struct not_strip not_strip_t;
typedef struct not_entry not_entry_t;

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

typedef struct not_record_struct
{
    not_node_t *type;
    not_strip_t *value;
} not_record_struct_t;

typedef struct not_record_type
{
    not_node_t *type;
    void *value;
} not_record_type_t;

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

typedef struct not_record_proc
{
    void *handle;
    json_t *map;
} not_record_proc_t;

typedef struct not_record_builtin
{
    not_record_t *source;
    void *handle;
} not_record_builtin_t;

void not_record_link_increase(not_record_t *record);

int32_t
not_record_link_decrease(not_record_t *record);

const char *
not_record_type_as_string(not_record_t *record);

not_record_t *
not_record_create(uint64_t kind, void *value);

not_record_t *
not_record_copy(not_record_t *record);

not_record_t *
not_record_make_null();

not_record_t *
not_record_make_undefined();

not_record_t *
not_record_make_nan();

not_record_t *
not_record_make_type(not_node_t *type, void *value);

not_record_t *
not_record_make_struct(not_node_t *type, not_strip_t *value);

not_record_tuple_t *
not_record_make_tuple(not_record_t *value, not_record_tuple_t *next);

not_record_object_t *
not_record_make_object(char *key, not_record_t *value, not_record_object_t *next);

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

not_record_tuple_t *
not_record_tuple_copy(not_record_tuple_t *tuple);

not_record_object_t *
not_record_object_copy(not_record_object_t *object);

int32_t
not_record_tuple_destroy(not_record_tuple_t *tuple);

int32_t
not_record_object_destroy(not_record_object_t *object);

not_record_type_t *
not_record_type_copy(not_record_type_t *type);

not_record_struct_t *
not_record_struct_copy(not_record_struct_t *struct1);

int32_t
not_record_type_destroy(not_record_type_t *type);

int32_t
not_record_struct_destroy(not_record_struct_t *struct1);

not_record_t *
not_record_make_proc(void *handle, json_t *map);

char *
not_record_to_string(not_record_t *record, char *previous_buf);

not_record_t *
not_record_make_builtin(not_record_t *source, void *handle);

#endif