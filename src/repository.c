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
#include "interpreter/garbage.h"
#include "interpreter/symbol_table.h"
#include "interpreter/strip.h"
#include "interpreter/execute/execute.h"

not_repository_t base_repository;

int32_t
not_repository_init()
{
    not_repository_t *module = not_repository_get();

    module->begin = NULL;

    return 0;
}

not_repository_t *
not_repository_get()
{
    return &base_repository;
}

static void
not_repository_link(not_module_t *entry)
{
    not_repository_t *repo = not_repository_get();
    entry->next = repo->begin;
    if (repo->begin)
    {
        repo->begin->previous = entry;
    }
    entry->previous = NULL;
    repo->begin = entry;
}

static not_module_t *
not_repository_push_normal_module(const char *path, not_node_t *root)
{
    not_repository_t *repository = not_repository_get();

    for (not_module_t *item = repository->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, path) == 0)
        {
            return NULL;
        }
    }

    not_module_t *entry = (not_module_t *)not_memory_calloc(1, sizeof(not_module_t));
    if (!entry)
    {
        not_error_no_memory();
        return ERROR;
    }

    strcpy(entry->path, path);
    entry->root = root;

    not_repository_link(entry);

    return entry;
}

static not_module_t *
not_repository_push_json_module(const char *path, json_t *root,
#ifdef _WIN32
                                HMODULE handle
#else
                                void *handle
#endif
)
{
    not_repository_t *repository = not_repository_get();

    for (not_module_t *item = repository->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, path) == 0)
        {
            return NULL;
        }
    }

    not_module_t *entry = (not_module_t *)not_memory_calloc(1, sizeof(not_module_t));
    if (!entry)
    {
        not_error_no_memory();
        return ERROR;
    }

    strcpy(entry->path, path);
    entry->root = NULL;
    entry->json = root;
    entry->handle = handle;

    not_repository_link(entry);

    return entry;
}

not_module_t *
not_repository_load(char *path)
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
        char base_path[MAX_PATH];
        not_path_get_current_directory(base_path, MAX_PATH);
        if (not_path_is_relative(path))
        {
            not_path_join(base_path, path, base_file, MAX_PATH);
        }
        else
        {
            not_path_normalize(path, base_file, MAX_PATH);
        }
    }

    not_repository_t *repository = not_repository_get();

    for (not_module_t *item = repository->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, base_file) == 0)
        {
            return item;
        }
    }

    FILE *file = fopen(base_file, "r");
    if (!file)
    {
        not_error_system("error opening file:%s", base_file);
        return ERROR;
    }

    fseek(file, 0, SEEK_END);
    int64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)not_memory_calloc(length + 1, sizeof(char));
    if (!data)
    {
        fclose(file);
        not_error_no_memory();
        return ERROR;
    }

    int64_t i;
    if ((i = fread(data, 1, length, file)) < length)
    {
        not_error_system("%s: read returned %ld", base_file, i);
        return ERROR;
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

    json_t *json_path = json_object_get(root, "path");
    if (!json_is_string(json_path))
    {
        not_error_system("'json' path is not of string type in: %s", base_file);
        return ERROR;
    }

    char directory_path[MAX_PATH];
    not_path_get_directory_path(base_file, directory_path, MAX_PATH);

    char module_path[MAX_PATH];
    not_path_join(directory_path, json_string_value(json_path), module_path, MAX_PATH);

#ifdef _WIN32
    HMODULE handle = LoadLibrary(module_path);
    if (!handle)
    {
        not_error_system("%s", "failed to load library");
        return ERROR;
    }
#else
    void *handle = dlopen(module_path, RTLD_LAZY);
    if (!handle)
    {
        not_error_system("%s", dlerror());
        return ERROR;
    }
    dlerror();
#endif

    return not_repository_push_json_module(base_file, root, handle);

region_normal_module:
    not_syntax_t *syntax = not_syntax_create(base_file);
    if (syntax == ERROR)
    {
        return ERROR;
    }

    not_node_t *root_node = not_syntax_module(syntax);
    if (root_node == ERROR)
    {
        return ERROR;
    }

    not_module_t *module = not_repository_push_normal_module(base_file, root_node);
    if (module == ERROR)
    {
        return ERROR;
    }

    if (not_semantic_module(module->root) < 0)
    {
        return ERROR;
    }

    if (not_execute_run(module->root) < 0)
    {
        return ERROR;
    }

    return module;
}
