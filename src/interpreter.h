#pragma once
#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__ 1

typedef struct not_record not_record_t;

typedef struct not_interpreter
{
    not_record_t *rax;
    not_queue_t *expections;
} not_interpreter_t;

not_interpreter_t *
not_interpreter_create();

void not_interpreter_destroy(not_interpreter_t *it);

#endif