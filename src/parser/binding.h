#pragma once
#ifndef __BINDING_H__

typedef struct binding {
    program_t *program;
    list_t *errors;
    list_t *modules;
} binding_t;

int32_t
binding_run(binding_t *binding, node_t *node);

binding_t *
binding_create(program_t *program, list_t *errors);

#endif