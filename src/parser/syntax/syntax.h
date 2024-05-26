#pragma once
#ifndef __SYNTAX_H__
#define __SYNTAX_H__ 1

typedef enum SySyntaxModifier 
{
	SYNTAX_MODIFIER_NONE 			= 1 << 0,
	SYNTAX_MODIFIER_EXPORT 			= 1 << 1,
	SYNTAX_MODIFIER_READONLY 		= 1 << 2,
	SYNTAX_MODIFIER_REFERENCE 		= 1 << 3,
	SYNTAX_MODIFIER_KARG 			= 1 << 4,
	SYNTAX_MODIFIER_KWARG 			= 1 << 5,
	SYNTAX_MODIFIER_STATIC 			= 1 << 6
} SySyntaxModifier_t;

typedef struct SySyntax 
{
	SyScanner_t *scanner;
	sy_queue_t *states;
	SyToken_t *token;
	int32_t loop_depth;
} SySyntax_t;

typedef struct SySyntaxState
{
	int32_t loop_depth;
	int32_t ch;
	uint64_t offset;
	uint64_t reading_offset;
	uint64_t line;
	uint64_t column;
	uint64_t count_error;
	SyToken_t token;
} SySyntaxState_t;

SySyntax_t *
sy_syntax_create(char *path);

sy_node_t *
sy_syntax_module(SySyntax_t *syntax);

#endif //__SYNTAX_H__
