#pragma once
#ifndef __REPOSITORY_H__
#define __REPOSITORY_H__ 1

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct not_module
{
    struct not_module *previous;
    struct not_module *next;

    char path[MAX_PATH];

#ifdef _WIN32
    HMODULE handle;
#else
    void *handle;
#endif

    not_node_t *root;
    json_t *json;
} not_module_t;

typedef struct not_repository
{
    not_module_t *begin;
    not_mutex_t lock;
} not_repository_t;

int32_t
not_repository_init();

not_repository_t *
not_repository_get();

not_module_t *
not_repository_load(char *path);

#endif