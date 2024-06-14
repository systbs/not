#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <gmp.h>
#include <jansson.h>

#ifdef _WIN32
#include <windows.h>
#else
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

sy_repository_t base_repository;

int32_t
sy_repository_init()
{
    sy_repository_t *module = sy_repository_get();
    if (sy_mutex_init(&module->lock) < 0)
    {
        sy_error_system("'%s' could not initialize the lock", "sy_repository.lock");
        return -1;
    }

    module->begin = NULL;

    return 0;
}

sy_repository_t *
sy_repository_get()
{
    return &base_repository;
}

static void
sy_repository_link(sy_module_t *entry)
{
    sy_repository_t *repo = sy_repository_get();
    entry->next = repo->begin;
    if (repo->begin)
    {
        repo->begin->previous = entry;
    }
    entry->previous = NULL;
    repo->begin = entry;
}

static sy_module_t *
sy_repository_push_normal_module(const char *path, sy_node_t *root)
{
    sy_repository_t *repository = sy_repository_get();

    if (sy_mutex_lock(&repository->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_repository.lock");
        return ERROR;
    }

    for (sy_module_t *item = repository->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, path) == 0)
        {
            if (sy_mutex_unlock(&repository->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_repository.lock");
                return ERROR;
            }
            return NULL;
        }
    }

    sy_module_t *entry = (sy_module_t *)sy_memory_calloc(1, sizeof(sy_module_t));
    if (!entry)
    {
        if (sy_mutex_unlock(&repository->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_repository.lock");
            return ERROR;
        }

        sy_error_no_memory();
        return ERROR;
    }

    strcpy(entry->path, path);
    entry->root = root;

    sy_repository_link(entry);

    if (sy_mutex_unlock(&repository->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not unlock", "sy_repository.lock");
        return ERROR;
    }

    return entry;
}

static sy_module_t *
sy_repository_push_json_module(const char *path, json_t *root,
#ifdef _WIN32
                               HMODULE handle
#else
                               void *handle
#endif
)
{
    sy_repository_t *repository = sy_repository_get();

    if (sy_mutex_lock(&repository->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_repository.lock");
        return ERROR;
    }

    for (sy_module_t *item = repository->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, path) == 0)
        {
            if (sy_mutex_unlock(&repository->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_repository.lock");
                return ERROR;
            }
            return NULL;
        }
    }

    sy_module_t *entry = (sy_module_t *)sy_memory_calloc(1, sizeof(sy_module_t));
    if (!entry)
    {
        if (sy_mutex_unlock(&repository->lock) < 0)
        {
            sy_error_system("'%s' could not unlock", "sy_repository.lock");
            return ERROR;
        }

        sy_error_no_memory();
        return ERROR;
    }

    strcpy(entry->path, path);
    entry->root = NULL;
    entry->json = root;
    entry->handle = handle;

    sy_repository_link(entry);

    if (sy_mutex_unlock(&repository->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not unlock", "sy_repository.lock");
        return ERROR;
    }

    return entry;
}

sy_module_t *
sy_repository_load(char *path)
{
    char base_file[MAX_PATH];

    if (sy_path_is_root(path))
    {
        char base_path[MAX_PATH];
        sy_path_normalize(getenv(ENV_LIBRARY_KEY), base_path, MAX_PATH);
        sy_path_join(base_path, path + 2, base_file, MAX_PATH);
    }
    else
    {
        char base_path[MAX_PATH];
        sy_path_get_current_directory(base_path, MAX_PATH);
        if (sy_path_is_relative(path))
        {
            sy_path_join(base_path, path, base_file, MAX_PATH);
        }
        else
        {
            sy_path_normalize(path, base_file, MAX_PATH);
        }
    }

    sy_repository_t *repository = sy_repository_get();

    if (sy_mutex_lock(&repository->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_repository.lock");
        return ERROR;
    }

    for (sy_module_t *item = repository->begin; item != NULL; item = item->next)
    {
        if (strcmp(item->path, base_file) == 0)
        {
            if (sy_mutex_unlock(&repository->lock) < 0)
            {
                sy_error_system("'%s' could not unlock", "sy_repository.lock");
                return ERROR;
            }
            return item;
        }
    }

    if (sy_mutex_unlock(&repository->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_repository.lock");
        return ERROR;
    }

    FILE *file = fopen(base_file, "r");
    if (!file)
    {
        sy_error_system("error opening file:%s", base_file);
        return ERROR;
    }

    fseek(file, 0, SEEK_END);
    int64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)sy_memory_calloc(length + 1, sizeof(char));
    if (!data)
    {
        fclose(file);
        sy_error_no_memory();
        return ERROR;
    }

    int64_t i;
    if ((i = fread(data, 1, length, file)) < length)
    {
        sy_error_system("%s: read returned %ld", base_file, i);
        return ERROR;
    }
    fclose(file);
    data[i] = '\0';

    json_error_t error;
    json_t *root = json_loads(data, 0, &error);
    if (!root)
    {
        sy_memory_free(data);
        goto region_normal_module;
    }

    sy_memory_free(data);

    json_t *json_path = json_object_get(root, "path");
    if (!json_is_string(json_path))
    {
        sy_error_system("'json' path is not of string type in: %s", base_file);
        return ERROR;
    }

    char directory_path[MAX_PATH];
    sy_path_get_directory_path(base_file, directory_path, MAX_PATH);

    char module_path[MAX_PATH];
    sy_path_join(directory_path, json_string_value(json_path), module_path, MAX_PATH);

#ifdef _WIN32
    HMODULE handle = LoadLibrary(module_path);
    if (!handle)
    {
        sy_error_system("%s", "failed to load library");
        return ERROR;
    }
#else
    void *handle = dlopen(module_path, RTLD_LAZY);
    if (!handle)
    {
        sy_error_system("%s", dlerror());
        return ERROR;
    }
    dlerror();
#endif

    return sy_repository_push_json_module(base_file, root, handle);

region_normal_module:
    sy_syntax_t *syntax = sy_syntax_create(base_file);
    if (syntax == ERROR)
    {
        return ERROR;
    }

    sy_node_t *root_node = sy_syntax_module(syntax);
    if (root_node == ERROR)
    {
        return ERROR;
    }

    sy_module_t *module = sy_repository_push_normal_module(base_file, root_node);
    if (module == ERROR)
    {
        return ERROR;
    }

    if (sy_semantic_module(module->root) < 0)
    {
        return ERROR;
    }

    if (sy_execute_run(module->root) < 0)
    {
        return ERROR;
    }

    return module;
}
