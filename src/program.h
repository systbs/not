#pragma once
#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#define MAX_URI (_MAX_DIR + _MAX_FNAME + _MAX_EXT)

typedef struct node node_t;

typedef struct pair {
    char *key;
    void *value;
} pair_t;

typedef struct program {
    char *base_path;
    char *base_file;

    char *out_file;
    FILE *out;

    node_t *main;

    stack_t *repository_nvars;
    int64_t n_variables;

    stack_t *repository;

    list_t *errors;
    list_t *modules;
} program_t;

void_t
program_report(program_t *program);

void_t
program_putpath(program_t *program, char *path);

node_t *
program_load(program_t *program, char *path);

void_t
program_outfile(program_t *program, char *path);

program_t *
program_create();


#endif