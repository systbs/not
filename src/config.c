#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

#include "types/types.h"
#include "utils/path.h"
#include "token/position.h"
#include "error.h"
#include "mutex.h"
#include "config.h"

#if defined(_WIN32) || defined(_WIN64)
#define setenv(name, value, overwrite) _putenv_s(name, value)
#define unsetenv(name) _putenv(name "=")
#endif

not_config_t base_config;

not_config_t *
not_config_get()
{
    return &base_config;
}

int32_t
not_config_init()
{
    not_config_t *config = not_config_get();
    config->expection = 0;

    char *env = getenv(ENV_LIBRARY_KEY);
    if (env == NOT_PTR_NULL)
    {
#if defined(_WIN32) || defined(_WIN64)
        char path[MAX_PATH];
        if (GetModuleFileName(NOT_PTR_NULL, path, MAX_PATH) != 0)
        {
            char dir_path[MAX_PATH];
            not_path_get_directory_path(path, dir_path, MAX_PATH);
            strcpy(config->library_path, dir_path);
            setenv(ENV_LIBRARY_KEY, dir_path, 1);
        }
        else
        {
            setenv(ENV_LIBRARY_KEY, DEFAULT_LIBRARY_PATH, 1);
        }
#elif defined(__linux__)
        char path[MAX_PATH];
        ssize_t count = readlink("/proc/self/exe", path, MAX_PATH);
        if (count != -1)
        {
            path[count] = '\0';

            char dir_path[MAX_PATH];
            not_path_get_directory_path(path, dir_path, MAX_PATH);

            strcpy(config->library_path, dir_path);
            setenv(ENV_LIBRARY_KEY, dir_path, 1);
        }
        else
        {
            setenv(ENV_LIBRARY_KEY, DEFAULT_LIBRARY_PATH, 1);
        }
#elif defined(__APPLE__) && defined(__MACH__)
        uint32_t size = MAX_PATH;
        if (_NSGetExecutablePath(path, &size) == 0)
        {
            char dir_path[MAX_PATH];
            not_path_get_directory_path(path, dir_path, MAX_PATH);

            strcpy(config->library_path, dir_path);
            setenv(ENV_LIBRARY_KEY, dir_path, 1);
        }
        else
        {
            setenv(ENV_LIBRARY_KEY, DEFAULT_LIBRARY_PATH, 1);
        }
#endif
    }
    else
    {
        strcpy(config->library_path, env);
    }

    return 0;
}

char *
not_config_get_library_path()
{
    not_config_t *config = not_config_get();
    return config->library_path;
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