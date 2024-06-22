#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define ENV_LIBRARY_KEY "NOT_LIBRARY_PATH"
#define DEFAULT_LIBRARY_PATH "/home/not/lib"
#define CONSTRUCTOR_STR "constructor"
#define INDEX_FILE_STR "index.not"

typedef struct not_config
{
    char input_file[MAX_PATH];
    char library_path[MAX_PATH];
    int32_t expection;
} not_config_t;

int32_t
not_config_set_input_file(const char *path);

char *
not_config_get_input_file();

int32_t
not_config_init();

int32_t
not_config_expection_is_enable();

void not_config_expection_set(int32_t expection);

not_config_t *
not_config_get();

char *
not_config_get_library_path();

#endif