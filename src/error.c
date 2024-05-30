#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "types/types.h"
#include "container/queue.h"
#include "token/position.h"
#include "ast/node.h"
#include "utils/path.h"
#include "mutex.h"
#include "memory.h"
#include "config.h"
#include "error.h"

void
sy_error_fatal_format_arg(const char *format, va_list arg)
{
	if (format)
	{
        fputs("Fatal error: ", stderr);
		vfprintf(stderr, format, arg);
        fputs("\n", stderr);
	}
    
    fflush(stderr);
}

void
sy_error_fatal_format(const char *format, ...)
{
    va_list arg;
	if (format)
	{
		va_start(arg, format);
		sy_error_fatal_format_arg(format, arg);
		va_end(arg);
	}
}

void
sy_error_no_memory()
{
    sy_error_fatal_format("Out of memory");
}

void
sy_error_system(const char *format, ...)
{
    /*
    if (!SyException_Runtime)
    {
        va_list arg;
        va_start(arg, format);
        sy_record_t *sy_record_arg = SyExecute_StringArgFromFormat(format, arg)
        if (!sy_record_arg)
        {
            sy_error_fatal_format("unhandled error, unable to create argument");
            return;
        }
        va_end(arg);

        sy_record_t *result = SyExecute_CallByOneArg(SyException_Runtime, sy_record_arg);
        if (!result)
        {
            sy_error_fatal_format("unhandled error, unable to call");
            return;
        }
        SyException_SetRise(result);
        return;
    }
    */

    va_list arg;
    va_start(arg, format);
    sy_error_fatal_format_arg(format, arg);
    va_end(arg);
}

void
sy_error_lexer_by_position(sy_position_t position, const char *format, ...)
{
    fputs("Traceback:\n", stderr);

    char base_path[MAX_PATH];
    SyPath_GetCurrentDirectory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    SyPath_GetRelative(base_path, position.path, relative_path, sizeof(relative_path));
    
    fprintf(stderr, 
            "File \"%s\", Line %lld, Column %lld\n", 
            relative_path, 
            position.line, 
            position.column
    );

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
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if(len - len_used < chunk_used) {
            len *= 2;
            if((buf = realloc(buf, len)) == NULL) {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n", 
                    __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if(buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0;i < strlen(buf);i++)
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

            if(buf[len_used - 1] != '\n')
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

void
sy_error_syntax_by_position(sy_position_t position, const char *format, ...)
{
    fputs("Traceback:\n", stderr);

    char base_path[MAX_PATH];
    SyPath_GetCurrentDirectory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    SyPath_GetRelative(base_path, position.path, relative_path, sizeof(relative_path));
    
    fprintf(stderr, 
            "File \"%s\", Line %lld, Column %lld\n", 
            relative_path, 
            position.line, 
            position.column
    );

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
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if(len - len_used < chunk_used) {
            len *= 2;
            if((buf = realloc(buf, len)) == NULL) {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n", 
                    __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if(buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0;i < strlen(buf);i++)
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

            if(buf[len_used - 1] != '\n')
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

void
sy_error_semantic_by_node(sy_node_t *node, const char *format, ...)
{
    fputs("Traceback:\n", stderr);

    sy_position_t position = node->position;

    char base_path[MAX_PATH];
    SyPath_GetCurrentDirectory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    SyPath_GetRelative(base_path, position.path, relative_path, sizeof(relative_path));
    
    fprintf(stderr, 
            "File \"%s\", Line %lld, Column %lld", 
            relative_path, 
            position.line, 
            position.column
    );

    sy_node_t *node1 = node;
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr, 
                ", <Module>"
            );
            break;
        }
        else
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;
            sy_node_t *key1 = class1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr, 
                ", Class <%s>", keSy_string1->value
            );
            break;
        }
        else
        if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;
            sy_node_t *key1 = fun1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr, 
                ", fun <%s>", keSy_string1->value
            );
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
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if(len - len_used < chunk_used) {
            len *= 2;
            if((buf = realloc(buf, len)) == NULL) {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n", 
                    __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if(buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0;i < strlen(buf);i++)
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

            if(buf[len_used - 1] != '\n')
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

void
sy_error_runtime_by_node(sy_node_t *node, const char *format, ...)
{
    fputs("Traceback:\n", stderr);

    sy_position_t position = node->position;

    char base_path[MAX_PATH];
    SyPath_GetCurrentDirectory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    SyPath_GetRelative(base_path, position.path, relative_path, sizeof(relative_path));
    
    fprintf(stderr, 
            "File \"%s\", Line %lld, Column %lld", 
            relative_path, 
            position.line, 
            position.column
    );

    sy_node_t *node1 = node;
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr, 
                ", <Module>"
            );
            break;
        }
        else
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;
            sy_node_t *key1 = class1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr, 
                ", Class <%s>", keSy_string1->value
            );
            break;
        }
        else
        if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;
            sy_node_t *key1 = fun1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr, 
                ", fun <%s>", keSy_string1->value
            );
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
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if(len - len_used < chunk_used) {
            len *= 2;
            if((buf = realloc(buf, len)) == NULL) {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n", 
                    __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if(buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0;i < strlen(buf);i++)
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

            if(buf[len_used - 1] != '\n')
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

void
sy_error_type_by_node(sy_node_t *node, const char *format, ...)
{
    fputs("Traceback:\n", stderr);

    sy_position_t position = node->position;

    char base_path[MAX_PATH];
    SyPath_GetCurrentDirectory(base_path, MAX_PATH);

    char relative_path[MAX_PATH];
    SyPath_GetRelative(base_path, position.path, relative_path, sizeof(relative_path));
    
    fprintf(stderr, 
            "File \"%s\", Line %lld, Column %lld", 
            relative_path, 
            position.line, 
            position.column
    );

    sy_node_t *node1 = node;
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            fprintf(stderr, 
                ", <Module>"
            );
            break;
        }
        else
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;
            sy_node_t *key1 = class1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr, 
                ", Class <%s>", keSy_string1->value
            );
            break;
        }
        else
        if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;
            sy_node_t *key1 = fun1->key;
            sy_node_basic_t *keSy_string1 = key1->value;

            fprintf(stderr, 
                ", fun <%s>", keSy_string1->value
            );
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
    while(fgets(chunk, sizeof(chunk), fp) != NULL) {
        size_t len_used = strlen(buf);
        size_t chunk_used = strlen(chunk);

        if(len - len_used < chunk_used) {
            len *= 2;
            if((buf = realloc(buf, len)) == NULL) {
                fprintf(stderr, "%s-%u:Unable to reallocate a block of %zu bytes\n", 
                    __FILE__, __LINE__, sizeof(len));
                free(buf);
                return;
            }
        }

        strncpy(buf + len_used, chunk, len - len_used);
        len_used += chunk_used;

        if(buf[len_used - 1])
        {
            line += 1;

            if ((line > position.line - 2) && (line < position.line + 2))
            {
                if (line == position.line)
                {
                    fprintf(stderr, "  \033[31m%lld\033[m\t|", line);

                    uint64_t j = 0;
                    for (uint64_t i = 0;i < strlen(buf);i++)
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

            if(buf[len_used - 1] != '\n')
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
