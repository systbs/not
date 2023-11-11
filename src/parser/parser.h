#pragma once
#ifndef __PARSER_H__
#define __PARSER_H__

typedef enum parser_modifier 
{
	PARSER_MODIFIER_NONE 			= 1 << 0,
	PARSER_MODIFIER_EXPORT 		= 1 << 1,
	PARSER_MODIFIER_READONLY 	= 1 << 2,
	PARSER_MODIFIER_STATIC 		= 1 << 3
} Parser_modifier_t;

typedef struct parser 
{
	file_source_t *file_source;
	scanner_t *scanner;
	list_t *errors;
	list_t *states;
	program_t *program;
	token_t *token;
	int32_t loop_depth;
	int32_t nouse_composite;
} parser_t;

typedef struct parser_state
{
	int32_t loop_depth;
	int32_t nouse_composite;
	int32_t ch;
	uint64_t offset;
	uint64_t reading_offset;
	uint64_t line;
	uint64_t column;
	token_t token;
} parser_state_t;

parser_t *
parser_create(program_t *program, char *path, list_t *errors);

node_t *
parser_module(parser_t *parser);

int32_t
parser_save_state(parser_t *parser);

int32_t
parser_restore_state(parser_t *parser);

int32_t
parser_release_state(parser_t *parser);

#endif //__PARSER_H__
