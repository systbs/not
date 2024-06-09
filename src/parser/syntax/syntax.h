#pragma once
#ifndef __SYNTAX_H__
#define __SYNTAX_H__ 1

typedef enum sy_syntax_modifier 
{
	SYNTAX_MODIFIER_NONE 			= 1 << 0,
	SYNTAX_MODIFIER_EXPORT 			= 1 << 1,
	SYNTAX_MODIFIER_READONLY 		= 1 << 2,
	SYNTAX_MODIFIER_REFERENCE 		= 1 << 3,
	SYNTAX_MODIFIER_KARG 			= 1 << 4,
	SYNTAX_MODIFIER_KWARG 			= 1 << 5,
	SYNTAX_MODIFIER_STATIC 			= 1 << 6
} sy_syntax_modifier_t;

typedef struct sy_syntax 
{
	SyScanner_t *scanner;
	sy_queue_t *states;
	sy_token_t *token;
	int32_t loop_depth;
	int32_t fun_depth;
} sy_syntax_t;

typedef struct sy_syntax_state
{
	int32_t fun_depth;
	int32_t loop_depth;
	int32_t ch;
	uint64_t offset;
	uint64_t reading_offset;
	uint64_t line;
	uint64_t column;
	uint64_t count_error;
	sy_token_t token;
} sy_syntax_state_t;

sy_syntax_t *
sy_syntax_create(char *path);

sy_node_t *
sy_syntax_module(sy_syntax_t *syntax);

#endif //__SYNTAX_H__
