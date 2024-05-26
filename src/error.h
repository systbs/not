#pragma once
#ifndef __ERRORS_H__
#define __ERRORS_H__ 1

typedef struct sy_node sy_node_t;
typedef struct sy_position sy_position_t;

void
sy_error_no_memory();

void
sy_error_system(const char *format, ...);

void
sy_error_lexer_by_position(sy_position_t position, const char *format, ...);

void
sy_error_syntax_by_position(sy_position_t position, const char *format, ...);

void
sy_error_semantic_by_node(sy_node_t *node, const char *format, ...);

void
sy_error_runtime_by_node(sy_node_t *node, const char *format, ...);

#endif