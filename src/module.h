#pragma once
#ifndef __MODULE_H__
#define __MODULE_H__ 1

typedef struct sy_module_entry_t {
    struct sy_module_entry_t *previous;
    struct sy_module_entry_t *next;

    char path[MAX_PATH];
    sy_node_t *root;
} sy_module_entry_t_t;

typedef struct SyModule {
    sy_module_entry_t_t *begin;
    sy_mutex_t lock;
} SyModule_t;

int32_t 
sy_module_init();

SyModule_t *
sy_module_get();

sy_module_entry_t_t *
sy_module_get_entry_by_path(const char *path);

sy_module_entry_t_t *
sy_module_load(char *path);

#endif