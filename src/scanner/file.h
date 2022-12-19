#pragma once
#ifndef __FILE_H__

typedef struct file_source {
    char name[_MAX_FNAME + _MAX_EXT];
    char path[_MAX_DIR];
    char *text;
} file_source_t;

file_source_t *
file_create_source(char *path);

void
file_destroy_source(file_source_t *file_source);

#endif