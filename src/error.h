#pragma once
#ifndef __ERRORS_H__
#define __ERRORS_H__ 1

typedef struct not_node not_node_t;
typedef struct not_position not_position_t;

int32_t
not_error_init();

void not_error_no_memory();

void not_error_system(const char *format, ...);

void not_error_lexer_by_position(not_position_t position, const char *format, ...);

void not_error_syntax_by_position(not_position_t position, const char *format, ...);

void not_error_semantic_by_node(not_node_t *node, const char *format, ...);

void not_error_runtime_by_node(not_node_t *node, const char *format, ...);

void not_error_type_by_node(not_node_t *node, const char *format, ...);

#endif