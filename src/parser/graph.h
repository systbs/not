#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__

typedef struct graph {
    program_t *program;
    list_t *errors;
    symbol_t *symbol;
} graph_t;

int32_t
graph_run(graph_t *graph, syntax_t *syntax);

graph_t *
graph_create(program_t *program, list_t *errors);

int32_t
graph_analysis(graph_t *graph);

#endif //__GRAPH_H__
