#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <gmp.h>
#include <jansson.h>

#include "types/types.h"
#include "container/queue.h"
#include "token/position.h"
#include "ast/node.h"
#include "utils/path.h"
#include "mutex.h"
#include "memory.h"
#include "config.h"
#include "interpreter.h"
#include "thread.h"
#include "error.h"
#include "interpreter/record.h"

enum
{
    ERR_MEMORY_TYPE = 0,
    ERR_SYSTEM_TYPE,
    ERR_LEXER_TYPE,
    ERR_SYNTAX_TYPE,
    ERR_SEMANTIC_TYPE,
    ERR_RUNTIME_TYPE,
    ERR_TYPE_TYPE
};

void not_error_fatal_format_arg(const char *format, va_list arg)
{
    if (format)
    {
        fputs("Fatal error: ", stderr);
        vfprintf(stderr, format, arg);
        fputs("\n", stderr);
    }

    fflush(stderr);
    exit(-1);
}

void not_error_fatal_format(const char *format, ...)
{
    va_list arg;
    if (format)
    {
        va_start(arg, format);
        not_error_fatal_format_arg(format, arg);
        va_end(arg);
    }
}

void not_error_no_memory()
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        not_record_t *code = not_record_make_int_from_si(ERR_LEXER_TYPE);
        not_record_t *message = not_record_make_string("Out of memory");
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_code = not_record_make_object("code", code, NOT_PTR_NULL);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    not_error_fatal_format("Out of memory");
}

void not_error_system(const char *format, ...)
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        not_record_t *code = not_record_make_int_from_si(ERR_LEXER_TYPE);
        not_record_t *message = not_record_make_string(message_str);
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_code = not_record_make_object("code", code, NOT_PTR_NULL);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    va_list arg;
    va_start(arg, format);
    not_error_fatal_format_arg(format, arg);
    va_end(arg);
}

void not_error_lexer_by_position(not_position_t position, const char *format, ...)
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        not_record_t *line = not_record_make_int_from_ui(position.line);
        not_record_t *column = not_record_make_int_from_ui(position.column);
        not_record_t *offset = not_record_make_int_from_ui(position.offset);
        not_record_t *path = not_record_make_string(position.path);
        if ((line == NOT_PTR_ERROR) || (column == NOT_PTR_ERROR) || (offset == NOT_PTR_ERROR) || (path == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_line = not_record_make_object("line", line, NOT_PTR_NULL);
        if (object_line == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_column = not_record_make_object("column", column, object_line);
        if (object_column == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_offset = not_record_make_object("offset", offset, object_column);
        if (object_offset == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_path = not_record_make_object("path", path, object_offset);
        if (object_path == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_position = not_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        not_record_t *code = not_record_make_int_from_si(ERR_LEXER_TYPE);
        not_record_t *message = not_record_make_string(message_str);
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_position = not_record_make_object("position", record_position, NOT_PTR_NULL);
        if (object_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_code = not_record_make_object("code", code, object_position);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    char base_path[MAX_PATH];
    not_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    not_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %ld, Column %ld\n",
            relative_path,
            position.line,
            position.column);

    fputs("Lexer error: ", stderr);

    va_list arg;
    if (format)
    {
        va_start(arg, format);
        vfprintf(stderr, format, arg);
        va_end(arg);
    }

    fputs("\n", stderr);

    FILE *fp = fopen(position.path, "rb");

    if (fp == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NOT_PTR_NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NOT_PTR_NULL)
            {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n",
                        __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if (buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%ld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0; i < strlen(buf); i++)
                    {
                        j += 1;
                        if (buf[i] == '\t')
                        {
                            j += 3;
                        }

                        if ((j >= position.column) && (j < position.column + position.length))
                        {
                            fprintf(stderr, "\033[1;31m%c\033[m", buf[i]);
                        }
                        else
                        {
                            fprintf(stderr, "%c", buf[i]);
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "  %ld\t|%s", line, buf);
                }
            }

            if (buf[len_used - 1] != '\n')
            {
                fputs("\n", stderr);
            }

            buf[0] = '\0';
        }
    }

    fclose(fp);
    free(buf);
    fflush(stderr);
    exit(-1);
}

void not_error_syntax_by_position(not_position_t position, const char *format, ...)
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        not_record_t *line = not_record_make_int_from_ui(position.line);
        not_record_t *column = not_record_make_int_from_ui(position.column);
        not_record_t *offset = not_record_make_int_from_ui(position.offset);
        not_record_t *path = not_record_make_string(position.path);
        if ((line == NOT_PTR_ERROR) || (column == NOT_PTR_ERROR) || (offset == NOT_PTR_ERROR) || (path == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_line = not_record_make_object("line", line, NOT_PTR_NULL);
        if (object_line == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_column = not_record_make_object("column", column, object_line);
        if (object_column == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_offset = not_record_make_object("offset", offset, object_column);
        if (object_offset == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_path = not_record_make_object("path", path, object_offset);
        if (object_path == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_position = not_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        not_record_t *code = not_record_make_int_from_si(ERR_SYNTAX_TYPE);
        not_record_t *message = not_record_make_string(message_str);
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_position = not_record_make_object("position", record_position, NOT_PTR_NULL);
        if (object_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_code = not_record_make_object("code", code, object_position);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    char base_path[MAX_PATH];
    not_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    not_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %ld, Column %ld\n",
            relative_path,
            position.line,
            position.column);

    fputs("Syntax error: ", stderr);

    va_list arg;
    if (format)
    {
        va_start(arg, format);
        vfprintf(stderr, format, arg);
        va_end(arg);
    }

    fputs("\n", stderr);

    FILE *fp = fopen(position.path, "rb");

    if (fp == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NOT_PTR_NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NOT_PTR_NULL)
            {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n",
                        __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if (buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%ld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0; i < strlen(buf); i++)
                    {
                        j += 1;
                        if (buf[i] == '\t')
                        {
                            j += 3;
                        }

                        if ((j >= position.column) && (j < position.column + position.length))
                        {
                            fprintf(stderr, "\033[1;31m%c\033[m", buf[i]);
                        }
                        else
                        {
                            fprintf(stderr, "%c", buf[i]);
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "  %ld\t|%s", line, buf);
                }
            }

            if (buf[len_used - 1] != '\n')
            {
                fputs("\n", stderr);
            }

            buf[0] = '\0';
        }
    }

    fclose(fp);
    free(buf);
    fflush(stderr);
}

void not_error_semantic_by_node(not_node_t *node, const char *format, ...)
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        not_position_t position = node->position;

        not_record_t *line = not_record_make_int_from_ui(position.line);
        not_record_t *column = not_record_make_int_from_ui(position.column);
        not_record_t *offset = not_record_make_int_from_ui(position.offset);
        not_record_t *path = not_record_make_string(position.path);
        if ((line == NOT_PTR_ERROR) || (column == NOT_PTR_ERROR) || (offset == NOT_PTR_ERROR) || (path == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_line = not_record_make_object("line", line, NOT_PTR_NULL);
        if (object_line == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_column = not_record_make_object("column", column, object_line);
        if (object_column == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_offset = not_record_make_object("offset", offset, object_column);
        if (object_offset == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_path = not_record_make_object("path", path, object_offset);
        if (object_path == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_position = not_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        not_record_t *code = not_record_make_int_from_si(ERR_SEMANTIC_TYPE);
        not_record_t *message = not_record_make_string(message_str);
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_position = not_record_make_object("position", record_position, NOT_PTR_NULL);
        if (object_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_code = not_record_make_object("code", code, object_position);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    not_position_t position = node->position;

    char base_path[MAX_PATH];
    not_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    not_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %ld, Column %ld",
            relative_path,
            position.line,
            position.column);

    not_node_t *node1 = node;
    while (node1 != NOT_PTR_NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr,
                    ", <Module>");
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class1 = (not_node_class_t *)node1->value;
            not_node_t *key1 = class1->key;
            not_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", Class <%s>", keSy_string1->value);
            break;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)node1->value;
            not_node_t *key1 = fun1->key;
            not_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", fun <%s>", keSy_string1->value);
        }
        node1 = node1->parent;
    }

    fputs("\n", stderr);
    fputs("Semantic error: ", stderr);

    va_list arg;
    if (format)
    {
        va_start(arg, format);
        vfprintf(stderr, format, arg);
        va_end(arg);
    }

    fputs("\n", stderr);

    FILE *fp = fopen(position.path, "rb");

    if (fp == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NOT_PTR_NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NOT_PTR_NULL)
            {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n",
                        __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if (buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%ld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0; i < strlen(buf); i++)
                    {
                        j += 1;
                        if (buf[i] == '\t')
                        {
                            j += 3;
                        }

                        if ((j >= position.column) && (j < position.column + position.length))
                        {
                            fprintf(stderr, "\033[1;31m%c\033[m", buf[i]);
                        }
                        else
                        {
                            fprintf(stderr, "%c", buf[i]);
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "  %ld\t|%s", line, buf);
                }
            }

            if (buf[len_used - 1] != '\n')
            {
                fputs("\n", stderr);
            }

            buf[0] = '\0';
        }
    }

    fclose(fp);
    free(buf);
    fflush(stderr);
}

void not_error_runtime_by_node(not_node_t *node, const char *format, ...)
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        not_position_t position = node->position;

        not_record_t *line = not_record_make_int_from_ui(position.line);
        not_record_t *column = not_record_make_int_from_ui(position.column);
        not_record_t *offset = not_record_make_int_from_ui(position.offset);
        not_record_t *path = not_record_make_string(position.path);
        if ((line == NOT_PTR_ERROR) || (column == NOT_PTR_ERROR) || (offset == NOT_PTR_ERROR) || (path == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_line = not_record_make_object("line", line, NOT_PTR_NULL);
        if (object_line == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_column = not_record_make_object("column", column, object_line);
        if (object_column == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_offset = not_record_make_object("offset", offset, object_column);
        if (object_offset == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_path = not_record_make_object("path", path, object_offset);
        if (object_path == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_position = not_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        not_record_t *code = not_record_make_int_from_si(ERR_RUNTIME_TYPE);
        not_record_t *message = not_record_make_string(message_str);
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_position = not_record_make_object("position", record_position, NOT_PTR_NULL);
        if (object_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_code = not_record_make_object("code", code, object_position);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    not_position_t position = node->position;

    char base_path[MAX_PATH];
    not_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    not_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %ld, Column %ld",
            relative_path,
            position.line,
            position.column);

    not_node_t *node1 = node;
    while (node1 != NOT_PTR_NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr,
                    ", <Module>");
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class1 = (not_node_class_t *)node1->value;
            not_node_t *key1 = class1->key;
            not_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", Class <%s>", keSy_string1->value);
            break;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)node1->value;
            not_node_t *key1 = fun1->key;
            not_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", fun <%s>", keSy_string1->value);
        }
        node1 = node1->parent;
    }

    fputs("\n", stderr);
    fputs("Runtime error: ", stderr);

    va_list arg;
    if (format)
    {
        va_start(arg, format);
        vfprintf(stderr, format, arg);
        va_end(arg);
    }

    fputs("\n", stderr);

    FILE *fp = fopen(position.path, "rb");

    if (fp == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NOT_PTR_NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NOT_PTR_NULL)
            {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n",
                        __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if (buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%ld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0; i < strlen(buf); i++)
                    {
                        j += 1;
                        if (buf[i] == '\t')
                        {
                            j += 3;
                        }

                        if ((j >= position.column) && (j < position.column + position.length))
                        {
                            fprintf(stderr, "\033[1;31m%c\033[m", buf[i]);
                        }
                        else
                        {
                            fprintf(stderr, "%c", buf[i]);
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "  %ld\t|%s", line, buf);
                }
            }

            if (buf[len_used - 1] != '\n')
            {
                fputs("\n", stderr);
            }

            buf[0] = '\0';
        }
    }

    fclose(fp);
    free(buf);
    fflush(stderr);
}

void not_error_type_by_node(not_node_t *node, const char *format, ...)
{
    if (not_config_expection_is_enable())
    {
        not_config_expection_set(0);

        not_position_t position = node->position;

        not_record_t *line = not_record_make_int_from_ui(position.line);
        not_record_t *column = not_record_make_int_from_ui(position.column);
        not_record_t *offset = not_record_make_int_from_ui(position.offset);
        not_record_t *path = not_record_make_string(position.path);
        if ((line == NOT_PTR_ERROR) || (column == NOT_PTR_ERROR) || (offset == NOT_PTR_ERROR) || (path == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_line = not_record_make_object("line", line, NOT_PTR_NULL);
        if (object_line == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_column = not_record_make_object("column", column, object_line);
        if (object_column == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_offset = not_record_make_object("offset", offset, object_column);
        if (object_offset == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_path = not_record_make_object("path", path, object_offset);
        if (object_path == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_position = not_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        not_record_t *code = not_record_make_int_from_si(ERR_TYPE_TYPE);
        not_record_t *message = not_record_make_string(message_str);
        if ((code == NOT_PTR_ERROR) || (message == NOT_PTR_ERROR))
        {
            goto region_fatal;
        }

        not_record_object_t *object_position = not_record_make_object("position", record_position, NOT_PTR_NULL);
        if (object_position == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_code = not_record_make_object("code", code, object_position);
        if (object_code == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }
        not_record_object_t *object_message = not_record_make_object("message", message, object_code);
        if (object_message == NOT_PTR_ERROR)
        {
            goto region_fatal;
        }

        not_record_t *record_error = not_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == NOT_PTR_ERROR)
        {
            not_record_object_destroy(object_message);
            goto region_fatal;
        }

        not_thread_t *t = not_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (NOT_PTR_ERROR == not_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        not_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    not_position_t position = node->position;

    char base_path[MAX_PATH];
    not_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    not_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %ld, Column %ld",
            relative_path,
            position.line,
            position.column);

    not_node_t *node1 = node;
    while (node1 != NOT_PTR_NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr,
                    ", <Module>");
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class1 = (not_node_class_t *)node1->value;
            not_node_t *key1 = class1->key;
            not_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", Class <%s>", keSy_string1->value);
            break;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)node1->value;
            not_node_t *key1 = fun1->key;
            not_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", fun <%s>", keSy_string1->value);
        }
        node1 = node1->parent;
    }

    fputs("\n", stderr);
    fputs("Type error: ", stderr);

    va_list arg;
    if (format)
    {
        va_start(arg, format);
        vfprintf(stderr, format, arg);
        va_end(arg);
    }

    fputs("\n", stderr);

    FILE *fp = fopen(position.path, "rb");

    if (fp == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NOT_PTR_NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NOT_PTR_NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NOT_PTR_NULL)
            {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n",
                        __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if (buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%ld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0; i < strlen(buf); i++)
                    {
                        j += 1;
                        if (buf[i] == '\t')
                        {
                            j += 3;
                        }

                        if ((j >= position.column) && (j < position.column + position.length))
                        {
                            fprintf(stderr, "\033[1;31m%c\033[m", buf[i]);
                        }
                        else
                        {
                            fprintf(stderr, "%c", buf[i]);
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "  %ld\t|%s", line, buf);
                }
            }

            if (buf[len_used - 1] != '\n')
            {
                fputs("\n", stderr);
            }

            buf[0] = '\0';
        }
    }

    fclose(fp);
    free(buf);
    fflush(stderr);
}
