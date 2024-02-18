#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__

typedef struct graph {
    symbol_t *symbol;
} graph_t;

int32_t
graph_run(program_t *program, symbol_t *root, node_t *node);

#endif //__GRAPH_H__
