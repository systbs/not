#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define ENV_LIBRARY_KEY "SYNTAX_LIBRARY_PATH"
#define DEFAULT_LIBRARY_PATH "/home/qalam/"
#define CONSTRUCTOR_STR "constructor"

typedef struct sy_config
{
    char input_file[MAX_PATH];
    int32_t expection;
} sy_config_t;

int32_t
sy_config_set_input_file(const char *path);

char *
sy_config_get_input_file();

int32_t
sy_config_init();

int32_t
sy_config_expection_is_enable();

void sy_config_expection_set(int32_t expection);

#endif