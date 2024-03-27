#pragma once
#ifndef __PROGRAM_H__

typedef struct pair {
    uint64_t key;
    void *value;
} pair_t;

typedef struct program {
    list_t *errors;
    list_t *imports;
    list_t *repository;
} program_t;

#endif