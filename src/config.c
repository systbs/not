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

sy_config_t base_config;

static sy_config_t *
sy_config_get()
{
    return &base_config;
}

int32_t
sy_config_init()
{
    sy_config_t *config = sy_config_get();
    config->expection = 0;
    return 0;
}

int32_t
sy_config_set_input_file(const char *path)
{
    sy_config_t *config = sy_config_get();

    if (sy_path_is_root(path))
    {
        char base_path[MAX_PATH];
        sy_path_normalize(getenv(ENV_LIBRARY_KEY), base_path, MAX_PATH);
        sy_path_join(base_path, path + 2, config->input_file, MAX_PATH);
    }
    else
    {
        char base_path[MAX_PATH];
        sy_path_get_current_directory(base_path, MAX_PATH);
        if (sy_path_is_relative(path))
        {
            sy_path_join(base_path, path, config->input_file, MAX_PATH);
        }
        else
        {
            sy_path_normalize(path, config->input_file, MAX_PATH);
        }
    }

    return 0;
}

char *
sy_config_get_input_file()
{
    sy_config_t *config = sy_config_get();
    return config->input_file;
}

int32_t
sy_config_expection_is_enable()
{
    sy_config_t *config = sy_config_get();
    return config->expection != 0;
}

void sy_config_expection_set(int32_t expection)
{
    sy_config_t *config = sy_config_get();
    config->expection = expection;
}