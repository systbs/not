#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include "types/types.h"
#include "utils/path.h"
#include "token/position.h"
#include "error.h"
#include "mutex.h"
#include "config.h"

not_config_t base_config;

static not_config_t *
not_config_get()
{
    return &base_config;
}

int32_t
not_config_init()
{
    not_config_t *config = not_config_get();
    config->expection = 0;
    return 0;
}

int32_t
not_config_set_input_file(const char *path)
{
    not_config_t *config = not_config_get();

    if (not_path_is_root(path))
    {
        char base_path[MAX_PATH];
        not_path_normalize(getenv(ENV_LIBRARY_KEY), base_path, MAX_PATH);
        not_path_join(base_path, path + 2, config->input_file, MAX_PATH);
    }
    else
    {
        char base_path[MAX_PATH];
        not_path_get_current_directory(base_path, MAX_PATH);
        if (not_path_is_relative(path))
        {
            not_path_join(base_path, path, config->input_file, MAX_PATH);
        }
        else
        {
            not_path_normalize(path, config->input_file, MAX_PATH);
        }
    }

    return 0;
}

char *
not_config_get_input_file()
{
    not_config_t *config = not_config_get();
    return config->input_file;
}

int32_t
not_config_expection_is_enable()
{
    not_config_t *config = not_config_get();
    return config->expection != 0;
}

void not_config_expection_set(int32_t expection)
{
    not_config_t *config = not_config_get();
    config->expection = expection;
}