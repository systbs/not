#pragma once
#ifndef __SYNTAX_H__
#define __SYNTAX_H__

typedef enum syntax_modifier 
{
	SYNTAX_MODIFIER_NONE 				= 1 << 0,
	SYNTAX_MODIFIER_EXPORT 			= 1 << 1,
	SYNTAX_MODIFIER_READONLY 		= 1 << 2,
	SYNTAX_MODIFIER_REFERENCE 	= 1 << 3,
	SYNTAX_MODIFIER_PROTECT 	= 1 << 4,
	SYNTAX_MODIFIER_STATIC 			= 1 << 5,
	SYNTAX_MODIFIER_ASYNC 			= 1 << 6
} Parser_modifier_t;

typedef struct syntax 
{
	file_source_t *file_source;
	scanner_t *scanner;
	list_t *states;
	program_t *program;
	token_t *token;
	int32_t loop_depth;
	int32_t nouse_composite;
} syntax_t;

typedef struct syntax_state
{
	int32_t loop_depth;
	int32_t nouse_composite;
	int32_t ch;
	uint64_t offset;
	uint64_t reading_offset;
	uint64_t line;
	uint64_t column;
	uint64_t count_error;
	token_t token;
} syntax_state_t;

syntax_t *
syntax_create(program_t *program, char *path);

node_t *
syntax_module(program_t *program, syntax_t *syntax);

#endif //__SYNTAX_H__
