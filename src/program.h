#pragma once
#ifndef __PROGRAM_H__

typedef struct program_pair {
    uint64_t key;
    void *value;
} program_pair_t;

typedef struct program {
    list_t *errors;
    list_t *imports;
    list_t *heritages;
} program_t;

#endif