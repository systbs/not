#pragma once
#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#define MAX_URI (_MAX_DIR + _MAX_FNAME + _MAX_EXT)

typedef struct node sy_node_t;
typedef struct pack pack_t;

typedef struct module {
    char *key;
    void *value;
} module_t;

typedef struct program {
    char *base_path;
    char *base_file;

    char *out_file;
    FILE *out;

    sy_node_t *main;

    // using in execute
    sy_queue_t *repository;
    pack_t *rax;

    sy_queue_t *errors;
    sy_queue_t *modules;
} program_t;

void_t
Sy_programReport(program_t *program);

void_t
Sy_programPutPath(program_t *program, char *path);

void_t
Sy_programOutfile(program_t *program, char *path);

program_t *
Sy_programCreate();


#endif