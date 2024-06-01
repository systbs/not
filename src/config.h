#pragma once
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_PATH (_MAX_PATH + _MAX_DIR + _MAX_FNAME + _MAX_EXT)
#define ENV_LIBRARY_KEY "SYNTAX_LIBRARY_PATH"
#define DEFAULT_LIBRARY_PATH "/home/qalam/"

typedef struct sy_config {
    char input_file[MAX_PATH];
    sy_mutex_t lock;
} sy_config_t;

int32_t
sy_config_init();

int32_t
sy_config_destroy();

int32_t
sy_config_set_input_file(const char *path);

char *
sy_config_get_input_file();

#endif