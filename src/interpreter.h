#pragma once
#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__ 1

typedef struct sy_record sy_record_t;

typedef struct sy_interpreter
{
    sy_record_t *rax;
    sy_queue_t *expections;
} sy_interpreter_t;

sy_interpreter_t *
sy_interpreter_create();

void sy_interpreter_destroy(sy_interpreter_t *it);

#endif