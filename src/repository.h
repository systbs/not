#pragma once
#ifndef __REPOSITORY_H__
#define __REPOSITORY_H__ 1

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct sy_module
{
    struct sy_module *previous;
    struct sy_module *next;

    char path[MAX_PATH];

#ifdef _WIN32
    HMODULE handle;
#else
    void *handle;
#endif

    sy_node_t *root;
    json_t *json;
} sy_module_t;

typedef struct sy_repository
{
    sy_module_t *begin;
    sy_mutex_t lock;
} sy_repository_t;

int32_t
sy_repository_init();

sy_repository_t *
sy_repository_get();

sy_module_t *
sy_repository_load(char *path);

#endif