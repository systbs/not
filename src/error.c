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

void sy_error_fatal_format_arg(const char *format, va_list arg)
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

void sy_error_fatal_format(const char *format, ...)
{
    va_list arg;
    if (format)
    {
        va_start(arg, format);
        sy_error_fatal_format_arg(format, arg);
        va_end(arg);
    }
}

void sy_error_no_memory()
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        sy_record_t *code = sy_record_make_int_from_si(ERR_LEXER_TYPE);
        sy_record_t *message = sy_record_make_string("Out of memory");
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_code = sy_record_make_object("code", code, NULL);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    sy_error_fatal_format("Out of memory");
}

void sy_error_system(const char *format, ...)
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        char message_str[4096];
        memset(message_str, 0, 4096);
        if (format)
        {
            va_list arg;
            va_start(arg, format);
            vsprintf(message_str, format, arg);
            va_end(arg);
        }

        sy_record_t *code = sy_record_make_int_from_si(ERR_LEXER_TYPE);
        sy_record_t *message = sy_record_make_string(message_str);
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_code = sy_record_make_object("code", code, NULL);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    va_list arg;
    va_start(arg, format);
    sy_error_fatal_format_arg(format, arg);
    va_end(arg);
}

void sy_error_lexer_by_position(sy_position_t position, const char *format, ...)
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        sy_record_t *line = sy_record_make_int_from_ui(position.line);
        sy_record_t *column = sy_record_make_int_from_ui(position.column);
        sy_record_t *offset = sy_record_make_int_from_ui(position.offset);
        sy_record_t *path = sy_record_make_string(position.path);
        if ((line == ERROR) || (column == ERROR) || (offset == ERROR) || (path == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_line = sy_record_make_object("line", line, NULL);
        if (object_line == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_column = sy_record_make_object("column", column, object_line);
        if (object_column == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_offset = sy_record_make_object("offset", offset, object_column);
        if (object_offset == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_path = sy_record_make_object("path", path, object_offset);
        if (object_path == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_position = sy_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == ERROR)
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

        sy_record_t *code = sy_record_make_int_from_si(ERR_LEXER_TYPE);
        sy_record_t *message = sy_record_make_string(message_str);
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_position = sy_record_make_object("position", record_position, NULL);
        if (object_position == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_code = sy_record_make_object("code", code, object_position);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    char base_path[MAX_PATH];
    sy_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    sy_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %lld, Column %lld\n",
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

    if (fp == NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NULL)
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
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

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
                    fprintf(stderr, "  %lld\t|%s", line, buf);
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

void sy_error_syntax_by_position(sy_position_t position, const char *format, ...)
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        sy_record_t *line = sy_record_make_int_from_ui(position.line);
        sy_record_t *column = sy_record_make_int_from_ui(position.column);
        sy_record_t *offset = sy_record_make_int_from_ui(position.offset);
        sy_record_t *path = sy_record_make_string(position.path);
        if ((line == ERROR) || (column == ERROR) || (offset == ERROR) || (path == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_line = sy_record_make_object("line", line, NULL);
        if (object_line == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_column = sy_record_make_object("column", column, object_line);
        if (object_column == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_offset = sy_record_make_object("offset", offset, object_column);
        if (object_offset == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_path = sy_record_make_object("path", path, object_offset);
        if (object_path == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_position = sy_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == ERROR)
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

        sy_record_t *code = sy_record_make_int_from_si(ERR_SYNTAX_TYPE);
        sy_record_t *message = sy_record_make_string(message_str);
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_position = sy_record_make_object("position", record_position, NULL);
        if (object_position == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_code = sy_record_make_object("code", code, object_position);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    char base_path[MAX_PATH];
    sy_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    sy_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %lld, Column %lld\n",
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

    if (fp == NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NULL)
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
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

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
                    fprintf(stderr, "  %lld\t|%s", line, buf);
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

void sy_error_semantic_by_node(sy_node_t *node, const char *format, ...)
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        sy_position_t position = node->position;

        sy_record_t *line = sy_record_make_int_from_ui(position.line);
        sy_record_t *column = sy_record_make_int_from_ui(position.column);
        sy_record_t *offset = sy_record_make_int_from_ui(position.offset);
        sy_record_t *path = sy_record_make_string(position.path);
        if ((line == ERROR) || (column == ERROR) || (offset == ERROR) || (path == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_line = sy_record_make_object("line", line, NULL);
        if (object_line == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_column = sy_record_make_object("column", column, object_line);
        if (object_column == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_offset = sy_record_make_object("offset", offset, object_column);
        if (object_offset == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_path = sy_record_make_object("path", path, object_offset);
        if (object_path == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_position = sy_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == ERROR)
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

        sy_record_t *code = sy_record_make_int_from_si(ERR_SEMANTIC_TYPE);
        sy_record_t *message = sy_record_make_string(message_str);
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_position = sy_record_make_object("position", record_position, NULL);
        if (object_position == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_code = sy_record_make_object("code", code, object_position);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    sy_position_t position = node->position;

    char base_path[MAX_PATH];
    sy_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    sy_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %lld, Column %lld",
            relative_path,
            position.line,
            position.column);

    sy_node_t *node1 = node;
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr,
                    ", <Module>");
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;
            sy_node_t *key1 = class1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", Class <%s>", keSy_string1->value);
            break;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;
            sy_node_t *key1 = fun1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

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

    if (fp == NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NULL)
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
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

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
                    fprintf(stderr, "  %lld\t|%s", line, buf);
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

void sy_error_runtime_by_node(sy_node_t *node, const char *format, ...)
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        sy_position_t position = node->position;

        sy_record_t *line = sy_record_make_int_from_ui(position.line);
        sy_record_t *column = sy_record_make_int_from_ui(position.column);
        sy_record_t *offset = sy_record_make_int_from_ui(position.offset);
        sy_record_t *path = sy_record_make_string(position.path);
        if ((line == ERROR) || (column == ERROR) || (offset == ERROR) || (path == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_line = sy_record_make_object("line", line, NULL);
        if (object_line == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_column = sy_record_make_object("column", column, object_line);
        if (object_column == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_offset = sy_record_make_object("offset", offset, object_column);
        if (object_offset == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_path = sy_record_make_object("path", path, object_offset);
        if (object_path == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_position = sy_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == ERROR)
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

        sy_record_t *code = sy_record_make_int_from_si(ERR_RUNTIME_TYPE);
        sy_record_t *message = sy_record_make_string(message_str);
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_position = sy_record_make_object("position", record_position, NULL);
        if (object_position == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_code = sy_record_make_object("code", code, object_position);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    sy_position_t position = node->position;

    char base_path[MAX_PATH];
    sy_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    sy_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %lld, Column %lld",
            relative_path,
            position.line,
            position.column);

    sy_node_t *node1 = node;
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr,
                    ", <Module>");
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;
            sy_node_t *key1 = class1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", Class <%s>", keSy_string1->value);
            break;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;
            sy_node_t *key1 = fun1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

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

    if (fp == NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NULL)
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
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

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
                    fprintf(stderr, "  %lld\t|%s", line, buf);
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

void sy_error_type_by_node(sy_node_t *node, const char *format, ...)
{
    if (sy_config_expection_is_enable())
    {
        sy_config_expection_set(0);

        sy_position_t position = node->position;

        sy_record_t *line = sy_record_make_int_from_ui(position.line);
        sy_record_t *column = sy_record_make_int_from_ui(position.column);
        sy_record_t *offset = sy_record_make_int_from_ui(position.offset);
        sy_record_t *path = sy_record_make_string(position.path);
        if ((line == ERROR) || (column == ERROR) || (offset == ERROR) || (path == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_line = sy_record_make_object("line", line, NULL);
        if (object_line == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_column = sy_record_make_object("column", column, object_line);
        if (object_column == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_offset = sy_record_make_object("offset", offset, object_column);
        if (object_offset == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_path = sy_record_make_object("path", path, object_offset);
        if (object_path == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_position = sy_record_create(RECORD_KIND_OBJECT, object_path);
        if (record_position == ERROR)
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

        sy_record_t *code = sy_record_make_int_from_si(ERR_TYPE_TYPE);
        sy_record_t *message = sy_record_make_string(message_str);
        if ((code == ERROR) || (message == ERROR))
        {
            goto region_fatal;
        }

        sy_record_object_t *object_position = sy_record_make_object("position", record_position, NULL);
        if (object_position == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_code = sy_record_make_object("code", code, object_position);
        if (object_code == ERROR)
        {
            goto region_fatal;
        }
        sy_record_object_t *object_message = sy_record_make_object("message", message, object_code);
        if (object_message == ERROR)
        {
            goto region_fatal;
        }

        sy_record_t *record_error = sy_record_create(RECORD_KIND_OBJECT, object_message);
        if (object_message == ERROR)
        {
            sy_record_object_destroy(object_message);
            goto region_fatal;
        }

        sy_thread_t *t = sy_thread_get_current();
        if (!t)
        {
            goto region_fatal;
        }

        if (ERROR == sy_queue_right_push(t->interpreter->expections, record_error))
        {
            goto region_fatal;
        }

        sy_config_expection_set(1);
        return;
    }

region_fatal:
    fputs("Traceback:\n", stderr);

    sy_position_t position = node->position;

    char base_path[MAX_PATH];
    sy_path_get_current_directory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    sy_path_get_relative(base_path, position.path, relative_path, sizeof(relative_path));

    fprintf(stderr,
            "File \"%s\", Line %lld, Column %lld",
            relative_path,
            position.line,
            position.column);

    sy_node_t *node1 = node;
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr,
                    ", <Module>");
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;
            sy_node_t *key1 = class1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr,
                    ", Class <%s>", keSy_string1->value);
            break;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;
            sy_node_t *key1 = fun1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

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

    if (fp == NULL)
    {
        fprintf(stderr, "%s-%u:could not open(%s)\n",
                __FILE__, __LINE__, position.path);
        return;
    }

    uint64_t line = 0;

    char chunk[128];
    size_t len = sizeof(chunk);

    char *buf = calloc(1, len);
    if (buf == NULL)
    {
        fprintf(stderr, "%s-%u:Unable to allocted a block of %zu bytes\n",
                __FILE__, __LINE__, sizeof(chunk));
        return;
    }

    buf[0] = '\0';
    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if (len - len_used < chunk_used)
        {
            len *= 2;
            if ((buf = realloc(buf, len)) == NULL)
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
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

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
                    fprintf(stderr, "  %lld\t|%s", line, buf);
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
