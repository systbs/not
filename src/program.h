#pragma once
#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#define MAX_URI (_MAX_DIR + _MAX_FNAME + _MAX_EXT)

typedef struct pair {
    uint64_t key;
    void *value;
} pair_t;

typedef struct program {
    char *base_path;
    char *base_file;

    list_t *errors;
    list_t *modules;
    list_t *repository;
} program_t;

void
program_report(program_t *program);

void_t
program_resolve(program_t *program, char *path);

program_t *
program_create();

#endif