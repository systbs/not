#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <gmp.h>
#include <jansson.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <dlfcn.h>
#endif

#include "types/types.h"
#include "utils/path.h"
#include "container/queue.h"
#include "token/position.h"
#include "token/token.h"
#include "scanner/scanner.h"
#include "ast/node.h"
#include "parser/syntax/syntax.h"
#include "parser/semantic/semantic.h"
#include "error.h"
#include "mutex.h"
#include "config.h"
#include "memory.h"
#include "repository.h"
#include "interpreter.h"
#include "thread.h"
#include "interpreter/record.h"
#include "interpreter/symbol_table.h"
#include "interpreter/strip.h"
#include "interpreter/execute.h"

not_repository_t base_repository;

int32_t
not_repository_init()
{
    not_repository_t *module = not_repository_get();

    module->queue = not_queue_create();
    if (module->queue == NOT_PTR_ERROR)
    {
        return -1;
    }

    return 0;
}

not_repository_t *
not_repository_get()
{
    return &base_repository;
}

not_module_t *
not_repository_load(char *base, char *path)
{
    char base_file[MAX_PATH];

    if (not_path_is_root(path))
    {
        char base_path[MAX_PATH];
        not_path_normalize(not_config_get_library_path(), base_path, MAX_PATH);
        not_path_join(base_path, path + 2, base_file, MAX_PATH);
    }
    else
    {
        char directory_path[MAX_PATH];
        not_path_get_directory_path(base, directory_path, MAX_PATH);
        if (not_path_is_relative(path))
        {
            not_path_join(directory_path, path, base_file, MAX_PATH);
        }
        else
        {
            not_path_normalize(path, base_file, MAX_PATH);
        }
    }

    if (not_path_is_directory(base_file))
    {
        char base_path[MAX_PATH];
        not_path_join(base_file, INDEX_FILE_STR, base_path, MAX_PATH);
        strcpy(base_file, base_path);
    }

    not_repository_t *repository = not_repository_get();

    for (not_queue_entry_t *item = repository->queue->begin; item != repository->queue->end; item = item->next)
    {
        not_module_t *module = (not_module_t *)item->value;
        if (strcmp(module->path, base_file) == 0)
        {
            return module;
        }
    }

    FILE *file = fopen(base_file, "r");
    if (!file)
    {
        not_error_system("error opening file:%s", base_file);
        return NOT_PTR_ERROR;
    }

    fseek(file, 0, SEEK_END);
    int64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)not_memory_calloc(length + 1, sizeof(char));
    if (!data)
    {
        fclose(file);
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    int64_t i;
    if ((i = fread(data, 1, length, file)) < length)
    {
        not_error_system("%s: read returned %ld", base_file, i);
        return NOT_PTR_ERROR;
    }
    fclose(file);
    data[i] = '\0';

    json_error_t error;
    json_t *root = json_loads(data, 0, &error);
    if (!root)
    {
        not_memory_free(data);
        goto region_normal_module;
    }

    not_memory_free(data);

#if defined(_WIN32) || defined(_WIN64)
    json_t *json_path = json_object_get(root, "windows_path");
#elif defined(__APPLE__) || defined(__MACH__)
    json_t *json_path = json_object_get(root, "macos_path");
#else
    json_t *json_path = json_object_get(root, "linux_path");
#endif

    if (!json_is_string(json_path))
    {
        not_error_system("'json' path is not of string type in: %s", base_file);
        return NOT_PTR_ERROR;
    }

    char directory_path[MAX_PATH];
    not_path_get_directory_path(base_file, directory_path, MAX_PATH);

    char module_path[MAX_PATH];
    not_path_join(directory_path, json_string_value(json_path), module_path, MAX_PATH);

#if defined(_WIN32) || defined(_WIN64)
    HMODULE handle = LoadLibrary(module_path);
    if (!handle)
    {
        not_error_system("%s: '%s'", "failed to load library", module_path);
        return NOT_PTR_ERROR;
    }
#else
    void *handle = dlopen(module_path, RTLD_LAZY);
    if (!handle)
    {
        not_error_system("%s", dlerror());
        return NOT_PTR_ERROR;
    }
    dlerror();
#endif

    not_module_t *entry = (not_module_t *)not_memory_calloc(1, sizeof(not_module_t));
    if (!entry)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    strcpy(entry->path, base_file);
    entry->root = NULL;
    entry->json = root;
    entry->handle = handle;

    if (NOT_PTR_ERROR == not_queue_right_push(repository->queue, entry))
    {
        not_memory_free(entry);
        return NOT_PTR_ERROR;
    }

    return entry;

region_normal_module:
    not_syntax_t *syntax = not_syntax_create(base_file);
    if (syntax == NOT_PTR_ERROR)
    {
        return NOT_PTR_ERROR;
    }

    not_node_t *root_node = not_syntax_module(syntax);
    if (root_node == NOT_PTR_ERROR)
    {
        return NOT_PTR_ERROR;
    }

    entry = (not_module_t *)not_memory_calloc(1, sizeof(not_module_t));
    if (!entry)
    {
        not_error_no_memory();
        return NOT_PTR_ERROR;
    }

    strcpy(entry->path, base_file);
    entry->root = root_node;

    if (NOT_PTR_ERROR == not_queue_right_push(repository->queue, entry))
    {
        not_memory_free(entry);
        return NOT_PTR_ERROR;
    }

    if (not_semantic_module(entry->root) < 0)
    {
        return NOT_PTR_ERROR;
    }

    if (not_execute_run(entry->root) < 0)
    {
        return NOT_PTR_ERROR;
    }

    return entry;
}

void not_repository_destroy()
{
    not_repository_t *repository = not_repository_get();

    for (not_queue_entry_t *n = repository->queue->begin, *b = NULL; n != repository->queue->end; n = b)
    {
        b = n->next;
        not_module_t *module = (not_module_t *)n->value;
        if (module->root)
        {
            not_node_destroy(module->root);
        }
        else
        {
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(module->handle);
#else
            dlclose(module->handle);
#endif
            json_decref(module->json);
        }
    }
}