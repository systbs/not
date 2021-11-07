#pragma once

// eval operators
typedef enum operator {
	NUL,   	// null function
	IMM,   	// immidiate value to eax
	VAR,
	DATA,
	SUPER, 	// parent
	THIS,  	// this

	ENT,   	// enter to body region 3
	LEV,   	// leave body region 4
	PUSH,  	// push eax to stack_frame
	POP,   	// pop stack_frame to eax
	CONTINUE,  // continue
	BREAK, 	// break
	DOT,  	// change schema path
	CALL, 	// call a function 12
	CGT,

	JMP,   	// jump
	JZ,  	// jump if sp is not zero
	JNZ,   	// jump if sp is zero
	LD,  	// load sp to eax 21
	SD,  	// save eax to sp
	HASH,
	RET,
	SIM,
	REL,
	EXTND,
	FN,
	AT, // @
	DEF, // prototype
	COMMA,

	OR,
	LOR,
	XOR,
	AND,
	LAND,
	EQ,
	NE,
	LT,
	GT,
	LE,
	GE,
	LSHIFT,
	RSHIFT,

	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	EPISODE,

	PRTF,  
	SIZEOF,
	TYPEOF,

	EXIT,
	BLP = 66271,
	ELP = 27166,
} operator_type;

typedef enum imm_type {
	TP_IMM,
	TP_VAR,
	TP_NUMBER,
	TP_SCHEMA,
	TP_NULL,
	TP_ARRAY
} imm_type_t;

typedef struct parser {
    schema_t *schema; 
    table_t *schemas; 
	table_t *pool; 
    table_t *tokens; 
    itable_t *c;
	long_t ub;
} parser_t;

void
expression(parser_t *prs, array_t *code);

parser_t *
parse(table_t *tokens, array_t *code);

const char *
code_typeAsString(int tp);