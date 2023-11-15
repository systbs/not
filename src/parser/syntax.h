#pragma once
#ifndef __SYNTAX_H__

typedef struct syntax {
    program_t *program;
    list_t *errors;
    list_t *modules;
} syntax_t;

int32_t
syntax_run(syntax_t *syntax, node_t *node);

syntax_t *
syntax_create(program_t *program, list_t *errors);

#endif