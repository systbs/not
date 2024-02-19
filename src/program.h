#pragma once
#ifndef __PROGRAM_H__

typedef struct program {
    list_t *errors;
    list_t *imports;
} program_t;

#endif