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
#include "garbage.h"
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
    record->link = 1;

    return record;
}

void sy_record_link_increase(sy_record_t *record)
{
    record->link += 1;
}

int32_t
sy_record_link_decrease(sy_record_t *record)
{
    record->link -= 1;
    if (record->link <= 0)
    {
        if (sy_record_destroy(record) < 0)
        {
            return -1;
        }
    }
    return 0;
}

char *
sy_record_to_string(sy_record_t *record, char *previous_buf)
{
    if (record->kind == RECORD_KIND_CHAR)
    {
        char str[50];
        snprintf(str, sizeof(str), "%c", (char)(*(int8_t *)record->value));
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_STRING)
    {
        char *str = (char *)record->value;
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_INT)
    {
        char *str = mpz_get_str(NULL, 10, *(mpz_t *)record->value);
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_FLOAT)
    {
        size_t buf_size = 64;
        char *str = (char *)sy_memory_calloc(buf_size, sizeof(char));

        if (!str)
        {
            sy_error_no_memory();
            return ERROR;
        }

        size_t required_size = gmp_snprintf(str, buf_size, "%s%.Ff", previous_buf, (*(mpf_t *)record->value));
        if (required_size >= buf_size)
        {
            buf_size = required_size + 1;
            str = (char *)sy_memory_realloc(str, buf_size);

            if (!str)
            {
                sy_error_no_memory();
                return ERROR;
            }
            gmp_snprintf(str, buf_size, "%s%.Ff", previous_buf, (*(mpf_t *)record->value));
        }

        return str;
    }
    else if (record->kind == RECORD_KIND_OBJECT)
    {
        size_t length = strlen(previous_buf) + 1;
        char *str = sy_memory_calloc(length + 1, sizeof(char));
        if (str == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '{');
        for (sy_record_object_t *item = (sy_record_object_t *)record->value; item != NULL; item = item->next)
        {
            sy_node_basic_t *basic = (sy_node_basic_t *)item->key->value;
            length = strlen(str) + strlen(basic->value) + 1;
            char *result = sy_memory_calloc(length + 1, sizeof(char));
            if (result == NULL)
            {
                sy_error_no_memory();
                return ERROR;
            }
            snprintf(result, length + 1, "%s%s:", str, basic->value);
            sy_memory_free(str);
            str = result;

            result = sy_record_to_string(item->value, str);
            if (result == ERROR)
            {
                sy_memory_free(str);
                return ERROR;
            }
            sy_memory_free(str);
            str = result;

            if (item->next)
            {
                length = strlen(str) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s,", str);
                sy_memory_free(str);
                str = result;
            }
        }
        length = strlen(str) + 1;
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            sy_memory_free(str);
            return ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, '}');
        sy_memory_free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        size_t length = strlen(previous_buf) + 1;
        char *str = sy_memory_calloc(length + 1, sizeof(char));
        if (str == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '[');

        for (sy_record_tuple_t *item = (sy_record_tuple_t *)record->value; item != NULL; item = item->next)
        {
            char *result = sy_record_to_string(item->value, str);
            if (result == ERROR)
            {
                sy_memory_free(str);
                return ERROR;
            }
            sy_memory_free(str);
            str = result;

            if (item->next)
            {
                length = strlen(str) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s,", str);
                sy_memory_free(str);
                str = result;
            }
        }

        length = strlen(str) + 1;
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            sy_memory_free(str);
            return ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, ']');
        sy_memory_free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_UNDEFINED)
    {
        char *str = "undefined";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_NAN)
    {
        char *str = "nan";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_NULL)
    {
        char *str = "null";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_TYPE)
    {
        char *str = "<type>";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *struct1 = (sy_record_struct_t *)record->value;
        sy_node_t *type = struct1->type;
        sy_strip_t *strip_class = struct1->value;

        sy_node_class_t *class1 = (sy_node_class_t *)type->value;

        size_t length = strlen(previous_buf) + 1;
        char *str = sy_memory_calloc(length + 1, sizeof(char));
        if (str == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(str, length + 1, "%s%c", previous_buf, '{');
        uint64_t i = 0;
        for (sy_node_t *item = class1->block; item != NULL; item = item->next)
        {
            if (item->kind == NODE_KIND_PROPERTY)
            {
                sy_node_property_t *property = (sy_node_property_t *)item->value;
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
                    char *result = sy_memory_calloc(length + 1, sizeof(char));
                    if (result == NULL)
                    {
                        sy_error_no_memory();
                        return ERROR;
                    }
                    snprintf(result, length + 1, "%s,", str);
                    sy_memory_free(str);
                    str = result;
                }

                sy_node_basic_t *basic = (sy_node_basic_t *)property->key->value;
                length = strlen(str) + strlen(basic->value) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s%s:", str, basic->value);
                sy_memory_free(str);
                str = result;

                sy_entry_t *entry = sy_strip_variable_find(strip_class, type, property->key);
                if (entry == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                if (entry == NULL)
                {
                    sy_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    sy_memory_free(str);
                    return ERROR;
                }

                result = sy_record_to_string(entry->value, str);

                if (sy_record_link_decrease(entry->value) < 0)
                {
                    return ERROR;
                }

                if (result == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                sy_memory_free(str);
                str = result;
                i += 1;
            }
        }

        if (class1->heritages)
        {
            sy_node_block_t *block = (sy_node_block_t *)class1->heritages->value;
            for (sy_node_t *item = block->items; item != NULL; item = item->next)
            {
                sy_node_heritage_t *heritage = (sy_node_heritage_t *)item->value;
                if (i > 0)
                {
                    length = strlen(str) + 1;
                    char *result = sy_memory_calloc(length + 1, sizeof(char));
                    if (result == NULL)
                    {
                        sy_error_no_memory();
                        return ERROR;
                    }
                    snprintf(result, length + 1, "%s,", str);
                    sy_memory_free(str);
                    str = result;
                }

                sy_node_basic_t *basic = (sy_node_basic_t *)heritage->key->value;
                length = strlen(str) + strlen(basic->value) + 1;
                char *result = sy_memory_calloc(length + 1, sizeof(char));
                if (result == NULL)
                {
                    sy_error_no_memory();
                    return ERROR;
                }
                snprintf(result, length + 1, "%s%s:", str, basic->value);
                sy_memory_free(str);
                str = result;

                sy_entry_t *entry = sy_strip_variable_find(strip_class, type, heritage->key);
                if (entry == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                if (entry == NULL)
                {
                    sy_error_runtime_by_node(item, "'%s' is not initialized", basic->value);
                    sy_memory_free(str);
                    return ERROR;
                }

                result = sy_record_to_string(entry->value, str);

                if (sy_record_link_decrease(entry->value) < 0)
                {
                    return ERROR;
                }

                if (result == ERROR)
                {
                    sy_memory_free(str);
                    return ERROR;
                }
                sy_memory_free(str);
                str = result;
                i += 1;
            }
        }

        length = strlen(str) + 1;
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            sy_memory_free(str);
            return ERROR;
        }
        snprintf(result, length + 1, "%s%c", str, '}');
        sy_memory_free(str);
        return result;
    }
    else if (record->kind == RECORD_KIND_UNDEFINED)
    {
        char *str = "undefined";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_NAN)
    {
        char *str = "nan";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else if (record->kind == RECORD_KIND_NULL)
    {
        char *str = "null";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
    else
    {
        char *str = "";
        size_t length = strlen(previous_buf) + strlen(str);
        char *result = sy_memory_calloc(length + 1, sizeof(char));
        if (result == NULL)
        {
            sy_error_no_memory();
            return ERROR;
        }
        snprintf(result, length + 1, "%s%s", previous_buf, str);
        return result;
    }
}

sy_record_t *
sy_record_make_proc(void *handle, json_t *map)
{
    sy_record_proc_t *basic = (sy_record_proc_t *)sy_memory_calloc(1, sizeof(sy_record_proc_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    basic->handle = handle;
    basic->map = map;

    sy_record_t *record = sy_record_create(RECORD_KIND_PROC, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int(const char *value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);

    mpz_set_str(*basic, value, 10);

    sy_record_t *record = sy_record_create(RECORD_KIND_INT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int_from_ui(uint64_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);

    mpz_set_ui(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_INT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int_from_si(int64_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);
    mpz_set_si(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_INT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int_from_z(mpz_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init_set(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_INT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_int_from_f(mpf_t value)
{
    mpz_t *basic = (mpz_t *)sy_memory_calloc(1, sizeof(mpz_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpz_init(*basic);
    mpz_set_f(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_INT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float(const char *value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init(*basic);
    mpf_set_str(*basic, value, 10);

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float_from_d(double value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init(*basic);
    mpf_set_d(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float_from_si(int64_t value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init(*basic);
    mpf_set_si(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float_from_ui(uint64_t value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init(*basic);
    mpf_set_ui(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float_from_f(mpf_t value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init_set(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_t *
sy_record_make_float_from_z(mpz_t value)
{
    mpf_t *basic = (mpf_t *)sy_memory_calloc(1, sizeof(mpf_t));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }

    mpf_init(*basic);
    mpf_set_z(*basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_FLOAT, basic);
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
    char *basic = (char *)sy_memory_calloc(1, strlen(value));
    if (basic == NULL)
    {
        sy_error_no_memory();
        return ERROR;
    }
    strcpy(basic, value);

    sy_record_t *record = sy_record_create(RECORD_KIND_STRING, basic);
    if (record == ERROR)
    {
        sy_memory_free(basic);
        sy_memory_free(basic);
        return ERROR;
    }
    return record;
}

sy_record_object_t *
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

    return basic;
}

sy_record_tuple_t *
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

    return basic;
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
sy_record_make_type(sy_node_t *type, void *value)
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

    if (sy_record_link_decrease(object->value) < 0)
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

    if (sy_record_link_decrease(tuple->value) < 0)
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
    else if (type->type->kind == NODE_KIND_ARRAY)
    {
        sy_record_t *value = (sy_record_t *)type->value;
        if (sy_record_link_decrease(value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_TUPLE)
    {
        if (sy_record_tuple_destroy((sy_record_tuple_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_CLASS)
    {
        if (sy_strip_destroy((sy_strip_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_FUN)
    {
        if (sy_strip_destroy((sy_strip_t *)type->value) < 0)
        {
            return -1;
        }
    }
    else if (type->type->kind == NODE_KIND_LAMBDA)
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
    else if (type->type->kind == NODE_KIND_ARRAY)
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
    else if (type->type->kind == NODE_KIND_TUPLE)
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
    else if (type->type->kind == NODE_KIND_CLASS)
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
    else if (type->type->kind == NODE_KIND_FUN)
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
    else if (type->type->kind == NODE_KIND_LAMBDA)
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
        else if (type->type->kind == NODE_KIND_ARRAY)
        {
            sy_record_t *rec = (sy_record_t *)value;
            if (sy_record_link_decrease(rec) < 0)
            {
                return ERROR;
            }
        }
        else if (type->type->kind == NODE_KIND_TUPLE)
        {
            if (sy_record_tuple_destroy((sy_record_tuple_t *)value) < 0)
            {
                return ERROR;
            }
        }
        else if (type->type->kind == NODE_KIND_CLASS)
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
    if (record->kind == RECORD_KIND_INT)
    {
        return sy_record_make_int_from_z(*(mpz_t *)(record->value));
    }
    else if (record->kind == RECORD_KIND_FLOAT)
    {
        return sy_record_make_float_from_f(*(mpf_t *)(record->value));
    }
    else if (record->kind == RECORD_KIND_CHAR)
    {
        return sy_record_make_char(*(char *)(record->value));
    }
    else if (record->kind == RECORD_KIND_STRING)
    {
        return sy_record_make_string((char *)(record->value));
    }
    else if (record->kind == RECORD_KIND_OBJECT)
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
    else if (record->kind == RECORD_KIND_TUPLE)
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
    else if (record->kind == RECORD_KIND_STRUCT)
    {
        sy_record_struct_t *basic = sy_record_struct_copy((sy_record_struct_t *)record->value);
        ;
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
    else if (record->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *basic = sy_record_type_copy((sy_record_type_t *)record->value);
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
    else if (record->kind == RECORD_KIND_NULL)
    {
        return sy_record_make_null();
    }
    else if (record->kind == RECORD_KIND_UNDEFINED)
    {
        return sy_record_make_undefined();
    }
    else if (record->kind == RECORD_KIND_NAN)
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

    if (record->kind == RECORD_KIND_OBJECT)
    {
        sy_record_object_t *object = (sy_record_object_t *)record->value;
        if (sy_record_object_destroy(object) < 0)
        {
            return -1;
        }
        sy_memory_free(record);
    }
    else if (record->kind == RECORD_KIND_TUPLE)
    {
        sy_record_tuple_t *tuple = (sy_record_tuple_t *)record->value;
        if (sy_record_tuple_destroy(tuple) < 0)
        {
            return -1;
        }
        sy_memory_free(record);
    }
    else if (record->kind == RECORD_KIND_TYPE)
    {
        sy_record_type_t *type = (sy_record_type_t *)record->value;
        if (sy_record_type_destroy(type) < 0)
        {
            return -1;
        }
        sy_memory_free(record);
    }
    else if (record->kind == RECORD_KIND_STRUCT)
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
            sy_memory_free(record->value);
        }
        sy_memory_free(record);
    }

    return 0;
}
