#pragma once
#ifndef __PARSER_H__
#define __PARSER_H__

typedef enum parser_scope_type {
	PARSER_SCOPE_MODULE,
	PARSER_SCOPE_CLASS,
	PARSER_SCOPE_FN,
	PARSER_SCOPE_ENUM,
	PARSER_SCOPE_WHILE,
	PARSER_SCOPE_IF
} parser_scope_type_t;

typedef struct parser {
	lexer_t *lexer;
	
	int32_t loop_depth;
	
	int32_t scope_type;
	list_t *scope_list;
} parser_t;

parser_t *
parser_create(lexer_t *lexer);

node_t *
parser_module(parser_t *parser);

#endif //__PARSER_H__
