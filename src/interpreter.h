#pragma once
#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__ 1

typedef struct record sy_record_t;

typedef struct sy_interpreter {
    sy_record_t *rax;
} sy_interpreter_t;

sy_interpreter_t *
sy_interpreter_create();

int32_t
sy_interpreter_destroy(sy_interpreter_t *it);

#endif