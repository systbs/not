#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>
#include <assert.h>
#include <jansson.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../memory.h"
#include "../mutex.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "strip.h"
#include "entry.h"

static const char *const symbols[] = {
    [RECORD_KIND_INT] = "instance int",
    [RECORD_KIND_FLOAT] = "instance float",
    [RECORD_KIND_CHAR] = "instance char",
    [RECORD_KIND_STRING] = "instance string",
    [RECORD_KIND_OBJECT] = "instance object",
    [RECORD_KIND_TUPLE] = "instance tuple",
    [RECORD_KIND_TYPE] = "type",
    [RECORD_KIND_STRUCT] = "struct",
    [RECORD_KIND_NULL] = "null",
    [RECORD_KIND_UNDEFINED] = "undefined",
    [RECORD_KIND_NAN] = "nan",
    [RECORD_KIND_PROC] = "proc"};

const char *
not_record_type_as_string(not_record_t *record)
{
    return symbols[record->kind];
}

not_record_t *
not_record_create(uint64_t kind, void *value)
{
    not_record_t *record = (not_record_t *)not_memory_calloc(1, sizeof(not_record_t));
    if (record == NOT_PTR_NULL)
    {
        not_error_no_memory();
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
    if (record->link <= 0)
    {
        if (not_record_destroy(record) < 0)
        {
            return -1;
        }
    }
    return 0;
}

char *
not_record_to_string(not_record_t *record, char *previous_buf)
{
    if (record->kind == RECORD_KIND_CHAR)
    {
        char str[50];
        snprintf(str, sizeof(str), "%c", (char)(*(int8_t *)record->value));
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_STRING)
    {
        char *str = (char *)record->value;
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_INT)
    {
        char *str = mpz_get_str(NOT_PTR_NULL, 10, *(mpz_t *)record->value);
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_FLOAT)
    {
        size_t buf_size = 64;
        char *str = (char *)not_memory_calloc(buf_size, sizeof(char));

        if (!str)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }

        size_t required_size = gmp_snprintf(str, buf_size, "%s%.Ff", previous_buf, (*(mpf_t *)record->value));
        if (required_size >= buf_size)
        {
            buf_size = required_size + 1;
            str = (char *)not_memory_realloc(str, buf_size);

            if (!str)
            {
                not_error_no_memory();
                return NOT_PTR_ERROR;
            }
            gmp_snprintf(str, buf_size, "%s%.Ff", previous_buf, (*(mpf_t *)record->value));
        }

        return str;
    }
    else if (record->kind == RECORD_KIND_OBJECT)
    {
        size_t length = strlen(previous_buf) + 1;
        char *str = not_memory_calloc(length + 1, sizeof(char));
        if (str == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '{');
        for (not_record_object_t *item = (not_record_object_t *)record->value; item != NOT_PTR_NULL; item = item->next)
        {
            length = strlen(str) + strlen(item->key) + 1;
            char *result = not_memory_calloc(length + 1, sizeof(char));
            if (result == NOT_PTR_NULL)
            {
                not_error_no_memory();
                return NOT_PTR_ERROR;
            }
            snprintf(result, length + 1, "%s%s:", str, item->key);
            not_memory_free(str);
            str = result;

            if (item->value->kind == RECORD_KIND_STRING)
            {
                length = strlen(str) + 1;
                result = not_memory_calloc(length + 1, sizeof(char));
                if (result == NOT_PTR_NULL)
                {
                    not_error_no_memory();
                    return NOT_PTR_ERROR;
                }
                snprintf(result, length + 1, "%s\"", str);
                not_memory_free(str);
                str = result;
            }

            result = not_record_to_string(item->value, str);
            if (result == NOT_PTR_ERROR)
            {
                not_memory_free(str);
                return NOT_PTR_ERROR;
            }
            not_memory_free(str);
            str = result;

            if (item->value->kind == RECORD_KIND_STRING)
            {
                length = strlen(str) + 1;
                result = not_memory_calloc(length + 1, sizeof(char));
                if (result == NOT_PTR_NULL)
                {
                    not_error_no_memory();
                    return NOT_PTR_ERROR;
                }
                snprintf(result, length + 1, "%s\"", str);
                not_memory_free(str);
                str = result;
            }

            if (item->next)
            {
                length = strlen(str) + 1;
                char *result = not_memory_calloc(length + 1, sizeof(char));
                if (result == NOT_PTR_NULL)
                {
                    not_error_no_memory();
                    return NOT_PTR_ERROR;
                }
                snprintf(result, length + 1, "%s,", str);
                not_memory_free(str);
                str = result;
            }
        }
        length = strlen(str) + 1;
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            not_memory_free(str);
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, '}');
        not_memory_free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        size_t length = strlen(previous_buf) + 1;
        char *str = not_memory_calloc(length + 1, sizeof(char));
        if (str == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '[');

        for (not_record_tuple_t *item = (not_record_tuple_t *)record->value; item != NOT_PTR_NULL; item = item->next)
        {
            char *result = not_record_to_string(item->value, str);
            if (result == NOT_PTR_ERROR)
            {
                not_memory_free(str);
                return NOT_PTR_ERROR;
            }
            not_memory_free(str);
            str = result;

            if (item->next)
            {
                length = strlen(str) + 1;
                char *result = not_memory_calloc(length + 1, sizeof(char));
                if (result == NOT_PTR_NULL)
                {
                    not_error_no_memory();
                    return NOT_PTR_ERROR;
                }
                snprintf(result, length + 1, "%s,", str);
                not_memory_free(str);
                str = result;
            }
        }

        length = strlen(str) + 1;
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            not_memory_free(str);
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, ']');
        not_memory_free(str);

        return result;
    }
    else if (record->kind == RECORD_KIND_TYPE)
    {
        char *str = "<type>";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_STRUCT)
    {
        not_record_struct_t *struct1 = (not_record_struct_t *)record->value;
        not_node_t *type = struct1->type;
        not_strip_t *strip_class = struct1->value;

        not_node_class_t *class1 = (not_node_class_t *)type->value;

        size_t length = strlen(previous_buf) + 1;
        char *str = not_memory_calloc(length + 1, sizeof(char));
        if (str == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '{');
        uint64_t i = 0;
        for (not_node_t *item = class1->block; item != NOT_PTR_NULL; item = item->next)
        {
            if (item->kind == NODE_KIND_PROPERTY)
            {
                not_node_property_t *property = (not_node_property_t *)item->value;
                if ((property->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
                {
                    continue;
                }

                if ((property->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    continue;
                }

                if (i > 0)
                {
                    length = strlen(str) + 1;
                    char *result = not_memory_calloc(length + 1, sizeof(char));
                    if (result == NOT_PTR_NULL)
                    {
                        not_error_no_memory();
                        return NOT_PTR_ERROR;
                    }
                    snprintf(result, length + 1, "%s,", str);
                    not_memory_free(str);
                    str = result;
                }

                not_node_basic_t *basic = (not_node_basic_t *)property->key->value;
                length = strlen(str) + strlen(basic->value) + 1;
                char *result = not_memory_calloc(length + 1, sizeof(char));
                if (result == NOT_PTR_NULL)
                {
                    not_error_no_memory();
                    return NOT_PTR_ERROR;
                }
                snprintf(result, length + 1, "%s%s:", str, basic->value);
                not_memory_free(str);
                str = result;

                not_entry_t *entry = not_strip_variable_find(strip_class, type, property->key);
                if (entry == NOT_PTR_ERROR)
                {
                    not_memory_free(str);
                    return NOT_PTR_ERROR;
                }
                if (entry == NOT_PTR_NULL)
                {
                    not_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    not_memory_free(str);
                    return NOT_PTR_ERROR;
                }

                result = not_record_to_string(entry->value, str);

                if (not_record_link_decrease(entry->value) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                if (result == NOT_PTR_ERROR)
                {
                    not_memory_free(str);
                    return NOT_PTR_ERROR;
                }
                not_memory_free(str);
                str = result;
                i += 1;
            }
        }

        if (class1->heritages)
        {
            not_node_block_t *block = (not_node_block_t *)class1->heritages->value;
            for (not_node_t *item = block->items; item != NOT_PTR_NULL; item = item->next)
            {
                not_node_heritage_t *heritage = (not_node_heritage_t *)item->value;
                if (i > 0)
                {
                    length = strlen(str) + 1;
                    char *result = not_memory_calloc(length + 1, sizeof(char));
                    if (result == NOT_PTR_NULL)
                    {
                        not_error_no_memory();
                        return NOT_PTR_ERROR;
                    }
                    snprintf(result, length + 1, "%s,", str);
                    not_memory_free(str);
                    str = result;
                }

                not_node_basic_t *basic = (not_node_basic_t *)heritage->key->value;
                length = strlen(str) + strlen(basic->value) + 1;
                char *result = not_memory_calloc(length + 1, sizeof(char));
                if (result == NOT_PTR_NULL)
                {
                    not_error_no_memory();
                    return NOT_PTR_ERROR;
                }
                snprintf(result, length + 1, "%s%s:", str, basic->value);
                not_memory_free(str);
                str = result;

                not_entry_t *entry = not_strip_variable_find(strip_class, type, heritage->key);
                if (entry == NOT_PTR_ERROR)
                {
                    not_memory_free(str);
                    return NOT_PTR_ERROR;
                }
                if (entry == NOT_PTR_NULL)
                {
                    not_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    not_memory_free(str);
                    return NOT_PTR_ERROR;
                }

                result = not_record_to_string(entry->value, str);

                if (not_record_link_decrease(entry->value) < 0)
                {
                    return NOT_PTR_ERROR;
                }

                if (result == NOT_PTR_ERROR)
                {
                    not_memory_free(str);
                    return NOT_PTR_ERROR;
                }
                not_memory_free(str);
                str = result;
                i += 1;
            }
        }

        length = strlen(str) + 1;
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            not_memory_free(str);
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, '}');
        not_memory_free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_UNDEFINED)
    {
        char *str = "undefined";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_NAN)
    {
        char *str = "nan";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_NULL)
    {
        char *str = "null";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    {
        char *str = "";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = not_memory_calloc(length + 1, sizeof(char));
        if (result == NOT_PTR_NULL)
        {
            not_error_no_memory();
            return NOT_PTR_ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
}

not_record_t *
not_record_make_proc(void *handle, json_t *map)
{
    not_record_proc_t *basic = (not_record_proc_t *)not_memory_calloc(1, sizeof(not_record_proc_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    basic->handle = handle;
    basic->map = map;

    not_record_t *record = not_record_create(RECORD_KIND_PROC, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int(const char *value)
{
    mpz_t *basic = (mpz_t *)not_memory_calloc(1, sizeof(mpz_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);

    mpz_set_str(*basic, value, 10);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_ui(uint64_t value)
{
    mpz_t *basic = (mpz_t *)not_memory_calloc(1, sizeof(mpz_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);

    mpz_set_ui(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_si(int64_t value)
{
    mpz_t *basic = (mpz_t *)not_memory_calloc(1, sizeof(mpz_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);
    mpz_set_si(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_z(mpz_t value)
{
    mpz_t *basic = (mpz_t *)not_memory_calloc(1, sizeof(mpz_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpz_init_set(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_int_from_f(mpf_t value)
{
    mpz_t *basic = (mpz_t *)not_memory_calloc(1, sizeof(mpz_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpz_init(*basic);
    mpz_set_f(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_INT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float(const char *value)
{
    mpf_t *basic = (mpf_t *)not_memory_calloc(1, sizeof(mpf_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_str(*basic, value, 10);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_d(double value)
{
    mpf_t *basic = (mpf_t *)not_memory_calloc(1, sizeof(mpf_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_d(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_si(int64_t value)
{
    mpf_t *basic = (mpf_t *)not_memory_calloc(1, sizeof(mpf_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_si(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_ui(uint64_t value)
{
    mpf_t *basic = (mpf_t *)not_memory_calloc(1, sizeof(mpf_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_ui(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_f(mpf_t value)
{
    mpf_t *basic = (mpf_t *)not_memory_calloc(1, sizeof(mpf_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpf_init_set(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_float_from_z(mpz_t value)
{
    mpf_t *basic = (mpf_t *)not_memory_calloc(1, sizeof(mpf_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    mpf_init(*basic);
    mpf_set_z(*basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_FLOAT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_char(char value)
{
    char *basic = (char *)not_memory_calloc(1, sizeof(char));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    *basic = value;

    not_record_t *record = not_record_create(RECORD_KIND_CHAR, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_string(char *value)
{
    char *basic = (char *)not_memory_calloc(1, strlen(value) + 1);
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }
    strcpy(basic, value);

    not_record_t *record = not_record_create(RECORD_KIND_STRING, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_object_t *
not_record_make_object(char *key, not_record_t *value, not_record_object_t *next)
{
    not_record_object_t *basic = (not_record_object_t *)not_memory_calloc(1, sizeof(not_record_object_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    size_t length = strlen(key);
    basic->key = not_memory_calloc(length, sizeof(char) + 1);
    if (basic->key == NOT_PTR_ERROR)
    {
        not_error_no_memory();
        not_memory_free(basic);
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
    not_record_tuple_t *basic = (not_record_tuple_t *)not_memory_calloc(1, sizeof(not_record_tuple_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    basic->value = value;
    basic->next = next;

    return basic;
}

not_record_t *
not_record_make_struct(not_node_t *type, not_strip_t *value)
{
    not_record_struct_t *basic = (not_record_struct_t *)not_memory_calloc(1, sizeof(not_record_struct_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    basic->type = type;
    basic->value = value;

    not_record_t *record = not_record_create(RECORD_KIND_STRUCT, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
}

not_record_t *
not_record_make_type(not_node_t *type, void *value)
{
    not_record_type_t *basic = (not_record_type_t *)not_memory_calloc(1, sizeof(not_record_type_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    basic->type = type;
    basic->value = value;

    not_record_t *record = not_record_create(RECORD_KIND_TYPE, basic);
    if (record == NOT_PTR_ERROR)
    {
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    return record;
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

int32_t
not_record_object_destroy(not_record_object_t *object)
{
    if (object->next)
    {
        if (not_record_object_destroy(object->next) < 0)
        {
            return -1;
        }
        object->next = NOT_PTR_NULL;
    }

    if (not_record_link_decrease(object->value) < 0)
    {
        return -1;
    }

    not_memory_free(object->key);
    not_memory_free(object);

    return 0;
}

int32_t
not_record_tuple_destroy(not_record_tuple_t *tuple)
{
    if (tuple->next)
    {
        if (not_record_tuple_destroy(tuple->next) < 0)
        {
            return -1;
        }
        tuple->next = NOT_PTR_NULL;
    }

    if (not_record_link_decrease(tuple->value) < 0)
    {
        return -1;
    }

    not_memory_free(tuple);

    return 0;
}

int32_t
not_record_struct_destroy(not_record_struct_t *struct1)
{
    if (struct1->value)
    {
        if (not_strip_destroy(struct1->value) < 0)
        {
            return -1;
        }
    }

    not_memory_free(struct1);

    return 0;
}

int32_t
not_record_type_destroy(not_record_type_t *type)
{
    if (type->type->kind == NODE_KIND_OBJECT)
    {
        if (not_record_object_destroy((not_record_object_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_ARRAY)
    {
        not_record_t *value = (not_record_t *)type->value;
        if (not_record_link_decrease(value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_TUPLE)
    {
        if (not_record_tuple_destroy((not_record_tuple_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_CLASS)
    {
        if (not_strip_destroy((not_strip_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_FUN)
    {
        if (not_strip_destroy((not_strip_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_LAMBDA)
    {
        if (not_strip_destroy((not_strip_t *)type->value) < 0)
        {
            return -1;
        }
    }

    not_memory_free(type);

    return 0;
}

not_record_object_t *
not_record_object_copy(not_record_object_t *object)
{
    not_record_object_t *next = NOT_PTR_NULL;
    if (object->next)
    {
        next = not_record_object_copy(object->next);
        if (next == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }
    }

    not_record_object_t *basic = (not_record_object_t *)not_memory_calloc(1, sizeof(not_record_object_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        if (next)
        {
            if (not_record_object_destroy(next) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        return NOT_PTR_ERROR;
    }

    not_record_t *record_copy = not_record_copy(object->value);
    if (record_copy == NOT_PTR_ERROR)
    {
        if (next)
        {
            if (not_record_object_destroy(next) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }

    size_t length = strlen(object->key);
    basic->key = not_memory_calloc(length, sizeof(char) + 1);
    if (basic->key == NOT_PTR_ERROR)
    {
        not_error_no_memory();
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }
    strcpy(basic->key, object->key);

    basic->value = record_copy;
    basic->next = next;

    return basic;
}

not_record_tuple_t *
not_record_tuple_copy(not_record_tuple_t *tuple)
{
    not_record_tuple_t *next = NOT_PTR_NULL;
    if (tuple->next)
    {
        next = not_record_tuple_copy(tuple->next);
        if (next == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }
    }

    not_record_tuple_t *basic = (not_record_tuple_t *)not_memory_calloc(1, sizeof(not_record_tuple_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        if (next)
        {
            if (not_record_tuple_destroy(next) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        return NOT_PTR_ERROR;
    }

    not_record_t *record_copy = not_record_copy(tuple->value);
    if (record_copy == NOT_PTR_ERROR)
    {
        if (next)
        {
            if (not_record_tuple_destroy(next) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        not_memory_free(basic);
        return NOT_PTR_ERROR;
    }

    basic->value = record_copy;
    basic->next = next;

    return basic;
}

not_record_struct_t *
not_record_struct_copy(not_record_struct_t *struct1)
{
    not_strip_t *value = NOT_PTR_NULL;

    if (struct1->value)
    {
        value = not_strip_copy(struct1->value);
        if (value == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }
    }

    not_record_struct_t *basic = (not_record_struct_t *)not_memory_calloc(1, sizeof(not_record_struct_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        if (value)
        {
            if (not_strip_destroy(value) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }

        return NOT_PTR_ERROR;
    }

    basic->type = struct1->type;
    basic->value = value;

    return basic;
}

not_record_type_t *
not_record_type_copy(not_record_type_t *type)
{
    void *value = NOT_PTR_NULL;

    if (type->type->kind == NODE_KIND_OBJECT)
    {
        if (type->value)
        {
            value = not_record_object_copy((not_record_object_t *)type->value);
            if (value == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
        }
    }
    else if (type->type->kind == NODE_KIND_ARRAY)
    {
        if (type->value)
        {
            value = not_record_copy((not_record_t *)type->value);
            if (value == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
        }
    }
    else if (type->type->kind == NODE_KIND_TUPLE)
    {
        if (type->value)
        {
            value = not_record_tuple_copy((not_record_tuple_t *)type->value);
            if (value == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
        }
    }
    else if (type->type->kind == NODE_KIND_CLASS)
    {
        if (type->value)
        {
            value = not_strip_copy((not_strip_t *)type->value);
            if (value == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
        }
    }
    else if (type->type->kind == NODE_KIND_FUN)
    {
        if (type->value)
        {
            value = not_strip_copy((not_strip_t *)type->value);
            if (value == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
        }
    }
    else if (type->type->kind == NODE_KIND_LAMBDA)
    {
        if (type->value)
        {
            value = not_strip_copy((not_strip_t *)type->value);
            if (value == NOT_PTR_ERROR)
            {
                return NOT_PTR_ERROR;
            }
        }
    }

    not_record_type_t *basic = (not_record_type_t *)not_memory_calloc(1, sizeof(not_record_type_t));
    if (basic == NOT_PTR_NULL)
    {
        not_error_no_memory();
        if (type->type->kind == NODE_KIND_OBJECT)
        {
            if (not_record_object_destroy((not_record_object_t *)value) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        else if (type->type->kind == NODE_KIND_ARRAY)
        {
            not_record_t *rec = (not_record_t *)value;
            if (not_record_link_decrease(rec) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        else if (type->type->kind == NODE_KIND_TUPLE)
        {
            if (not_record_tuple_destroy((not_record_tuple_t *)value) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        else if (type->type->kind == NODE_KIND_CLASS)
        {
            if (not_strip_destroy((not_strip_t *)value) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }

        return NOT_PTR_ERROR;
    }

    basic->type = type->type;
    basic->value = value;

    return basic;
}

not_record_t *
not_record_copy(not_record_t *record)
{
    if (record->kind == RECORD_KIND_INT)
    {
        return not_record_make_int_from_z(*(mpz_t *)(record->value));
    }
    else if (record->kind == RECORD_KIND_FLOAT)
    {
        return not_record_make_float_from_f(*(mpf_t *)(record->value));
    }
    else if (record->kind == RECORD_KIND_CHAR)
    {
        return not_record_make_char(*(char *)(record->value));
    }
    else if (record->kind == RECORD_KIND_STRING)
    {
        return not_record_make_string((char *)(record->value));
    }
    else if (record->kind == RECORD_KIND_OBJECT)
    {
        not_record_object_t *basic = not_record_object_copy((not_record_object_t *)record->value);
        if (basic == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record_copy = not_record_create(RECORD_KIND_OBJECT, basic);
        if (record_copy == NOT_PTR_ERROR)
        {
            if (not_record_object_destroy(basic) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }
        return record_copy;
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        not_record_tuple_t *basic = not_record_tuple_copy((not_record_tuple_t *)record->value);
        if (basic == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record_copy = not_record_create(RECORD_KIND_TUPLE, basic);
        if (record_copy == NOT_PTR_ERROR)
        {
            if (not_record_tuple_destroy(basic) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }
        return record_copy;
    }
    else if (record->kind == RECORD_KIND_STRUCT)
    {
        not_record_struct_t *basic = not_record_struct_copy((not_record_struct_t *)record->value);
        ;
        if (basic == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record_copy = not_record_create(RECORD_KIND_STRUCT, basic);
        if (record_copy == NOT_PTR_ERROR)
        {
            if (not_record_struct_destroy(basic) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }

        return record_copy;
    }
    else if (record->kind == RECORD_KIND_TYPE)
    {
        not_record_type_t *basic = not_record_type_copy((not_record_type_t *)record->value);
        if (basic == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *record_copy = not_record_create(RECORD_KIND_TYPE, basic);
        if (record_copy == NOT_PTR_ERROR)
        {
            if (not_record_type_destroy(basic) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return NOT_PTR_ERROR;
        }

        return record_copy;
    }
    else if (record->kind == RECORD_KIND_NULL)
    {
        return not_record_make_null();
    }
    else if (record->kind == RECORD_KIND_UNDEFINED)
    {
        return not_record_make_undefined();
    }
    else if (record->kind == RECORD_KIND_NAN)
    {
        return not_record_make_nan();
    }

    return NOT_PTR_NULL;
}

int32_t
not_record_destroy(not_record_t *record)
{
    if (!record)
    {
        return 0;
    }

    if (record->kind == RECORD_KIND_OBJECT)
    {
        not_record_object_t *object = (not_record_object_t *)record->value;
        if (not_record_object_destroy(object) < 0)
        {
            return -1;
        }
        not_memory_free(record);
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        not_record_tuple_t *tuple = (not_record_tuple_t *)record->value;
        if (not_record_tuple_destroy(tuple) < 0)
        {
            return -1;
        }
        not_memory_free(record);
    }
    else if (record->kind == RECORD_KIND_TYPE)
    {
        not_record_type_t *type = (not_record_type_t *)record->value;
        if (not_record_type_destroy(type) < 0)
        {
            return -1;
        }
        not_memory_free(record);
    }
    else if (record->kind == RECORD_KIND_STRUCT)
    {
        not_record_struct_t *struct1 = (not_record_struct_t *)record->value;
        if (not_record_struct_destroy(struct1) < 0)
        {
            return -1;
        }
        not_memory_free(record);
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
            not_memory_free(record->value);
        }
        not_memory_free(record);
    }

    return 0;
}
