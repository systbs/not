#pragma once
#ifndef __REPOSITORY_H__
#define __REPOSITORY_H__ 1

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct not_module
{
    char path[MAX_PATH];

#if defined(_WIN32) || defined(_WIN64)
    HMODULE handle;
#else
    void *handle;
#endif

    not_node_t *root;
    json_t *json;
} not_module_t;

typedef struct not_repository
{
    not_queue_t *queue;
} not_repository_t;

int32_t
not_repository_init();

not_repository_t *
not_repository_get();

not_module_t *
not_repository_load(char *path);

void not_repository_destroy();

#endif