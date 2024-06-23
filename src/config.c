#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <jansson.h>

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
#include "memory.h"

#if defined(_WIN32) || defined(_WIN64)
int setenv(const char *name, const char *value, int overwrite)
{
    if (!overwrite)
    {
        char *existing_value = getenv(name);
        if (existing_value != NULL)
        {
            return 0;
        }
    }

    size_t name_len = strlen(name);
    size_t value_len = strlen(value);
    char *env_string = (char *)malloc(name_len + value_len + 2);
    if (env_string == NULL)
    {
        return -1;
    }

    strcpy(env_string, name);
    env_string[name_len] = '=';
    strcpy(env_string + name_len + 1, value);

    int result = _putenv(env_string);

    free(env_string);

    return result;
}

#define unsetenv(name) _putenv(name "=")

int set_system_environment_variable(const char *name, const char *value)
{
    HKEY hKey;
    LONG result;

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return -1;
    }

    result = RegSetValueEx(hKey, name, 0, REG_SZ, (const BYTE *)value, strlen(value) + 1);
    if (result != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return -1;
    }

    RegCloseKey(hKey);

    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, NULL);

    setenv(name, value, 1);

    return 0;
}

int get_system_environment_variable(const char *name, char *buffer)
{
    char value[1024];
    DWORD size = sizeof(value);

    if (GetEnvironmentVariable(name, value, size))
    {
        strcpy(buffer, value);
        return 0;
    }

    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS)
    {
        DWORD type;
        result = RegQueryValueEx(hKey, name, NULL, &type, (LPBYTE)value, &size);
        if (result == ERROR_SUCCESS && type == REG_SZ)
        {
            strcpy(buffer, value);
            return 0;
        }
        else
        {
            return -1;
        }
        RegCloseKey(hKey);
    }
    return -1;
}

#else
#define MAX_LINE_LENGTH 1024

int check_variable_in_file(const char *filename, const char *name)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n\r")] = '\0';
        if (strncmp(line, name, strlen(name)) == 0 && line[strlen(name)] == '=')
        {
            found = 1;
            break;
        }
    }

    fclose(file);
    return found;
}

int replace_variable_in_file(const char *filename, const char *name, const char *new_value)
{
    FILE *file = fopen(filename, "r+");
    if (file == NULL)
    {
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    long int pos;
    int found = 0;

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n\r")] = '\0';

        if (strncmp(line, name, strlen(name)) == 0 && line[strlen(name)] == '=')
        {
            pos = ftell(file);
            found = 1;
            break;
        }
    }

    if (found)
    {
        fseek(file, pos, SEEK_SET);
        fprintf(file, "%s=%s\n", name, new_value);
    }

    fclose(file);
    return found;
}

int set_system_environment_variable(const char *name, const char *value)
{
    int found = check_variable_in_file("/etc/environment", name);
    if (found == 1)
    {
        int r = replace_variable_in_file("/etc/environment", name, value);
        if (r == -1)
        {
            return -1;
        }
        setenv(name, value, 1);
        return 0;
    }
    else if (found == 0)
    {
        FILE *file = fopen("/etc/environment", "a");
        if (file == NULL)
        {
            return -1;
        }

        fprintf(file, "%s=%s\n", name, value);
        fclose(file);

        setenv(name, value, 1);

        return 0;
    }

    return -1;
}

int get_system_environment_variable(const char *name, char *buffer)
{
    char *value = getenv(name);
    if (value)
    {
        strcpy(buffer, value);
        return 0;
    }
    return -1;
}

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

    FILE *file = fopen(CONFIG_PATH, "r");
    if (!file)
    {
        not_error_system("error opening file:%s", CONFIG_PATH);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    int64_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = (char *)not_memory_calloc(length + 1, sizeof(char));
    if (!data)
    {
        fclose(file);
        not_error_no_memory();
        return -1;
    }

    int64_t i;
    if ((i = fread(data, 1, length, file)) < length)
    {
        not_error_system("%s: read returned %ld", CONFIG_PATH, i);
        fclose(file);
        return -1;
    }
    fclose(file);
    data[i] = '\0';

    json_error_t error;
    json_t *root = json_loads(data, 0, &error);
    if (!root)
    {
        not_memory_free(data);
        return -1;
    }

    json_t *packages_path = json_object_get(root, "packages_path");
    if (!packages_path || !json_is_string(packages_path))
    {
        not_error_system("'%s': missing 'packages_path'", CONFIG_PATH);
        return -1;
    }

    strcpy(config->library_path, json_string_value(packages_path));

    json_decref(root);

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