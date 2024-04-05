#pragma once
#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

int32_t
semantic_module(program_t *program, node_t *node);

int32_t
semantic_run(program_t *program, node_t *root);

#endif