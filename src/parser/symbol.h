#pragma once
#ifndef __SYMBOL_H__
#define __SYMBOL_H__

typedef enum symbol_type {
    SYMBOL_TYPE_NONE						= 1,
    SYMBOL_TYPE_VAR,
	SYMBOL_TYPE_MODULE,
    SYMBOL_TYPE_PROPERTY,
    SYMBOL_TYPE_FUNCTION,
    SYMBOL_TYPE_LAMBDA,
    SYMBOL_TYPE_CLASS,
    SYMBOL_TYPE_ENUM,
    SYMBOL_TYPE_MEMBER,
    SYMBOL_TYPE_MEMBERS,
    SYMBOL_TYPE_TYPE,
    SYMBOL_TYPE_METHOD,
    SYMBOL_TYPE_GENERIC,
    SYMBOL_TYPE_GENERICS,
    SYMBOL_TYPE_PARAMETER,
    SYMBOL_TYPE_PARAMETERS,
    SYMBOL_TYPE_FIELD,
    SYMBOL_TYPE_FIELDS,
    SYMBOL_TYPE_TYPE_EXPRESSION,
    SYMBOL_TYPE_EXPORT,
    SYMBOL_TYPE_IMPORT,
    SYMBOL_TYPE_PATH,
    SYMBOL_TYPE_HERITAGE,
    SYMBOL_TYPE_HERITAGES,
    SYMBOL_TYPE_THROW,
    SYMBOL_TYPE_RETURN,
    SYMBOL_TYPE_CONTINUE,
    SYMBOL_TYPE_BREAK,
    SYMBOL_TYPE_FORIN ,
    SYMBOL_TYPE_EXPRESSION,
    SYMBOL_TYPE_FOR,
    SYMBOL_TYPE_INITIALIZER,
    SYMBOL_TYPE_INCREMENTOR,
    SYMBOL_TYPE_BLOCK,
    SYMBOL_TYPE_CATCH,
    SYMBOL_TYPE_CATCHS,
    SYMBOL_TYPE_TRY,
    SYMBOL_TYPE_IF,
    SYMBOL_TYPE_CONDITION,
    SYMBOL_TYPE_ELSE,

    SYMBOL_TYPE_DEFINE,
    SYMBOL_TYPE_ASSIGN,
    SYMBOL_TYPE_ADD_ASSIGN,
    SYMBOL_TYPE_SUB_ASSIGN,
    SYMBOL_TYPE_DIV_ASSIGN,
    SYMBOL_TYPE_MUL_ASSIGN,
    SYMBOL_TYPE_MOD_ASSIGN,
    SYMBOL_TYPE_AND_ASSIGN,
    SYMBOL_TYPE_OR_ASSIGN,
    SYMBOL_TYPE_SHL_ASSIGN,
    SYMBOL_TYPE_SHR_ASSIGN,

    SYMBOL_TYPE_CONDITIONAL,
    SYMBOL_TYPE_TRUE,
    SYMBOL_TYPE_FALSE,
    SYMBOL_TYPE_LEFT,
    SYMBOL_TYPE_RIGHT,
    SYMBOL_TYPE_TILDE,
	SYMBOL_TYPE_NOT,
	SYMBOL_TYPE_NEG,
	SYMBOL_TYPE_POS,
	SYMBOL_TYPE_AWAIT,
	SYMBOL_TYPE_SIZEOF,
	SYMBOL_TYPE_TYPEOF,
	SYMBOL_TYPE_ELLIPSIS,
    SYMBOL_TYPE_SHL,
    SYMBOL_TYPE_SHR,
    SYMBOL_TYPE_LOR,
    SYMBOL_TYPE_LAND,
    SYMBOL_TYPE_OR,
    SYMBOL_TYPE_XOR,
    SYMBOL_TYPE_AND,
    SYMBOL_TYPE_EQ,
    SYMBOL_TYPE_IN,
    SYMBOL_TYPE_EXTENDS,
    SYMBOL_TYPE_NEQ,
    SYMBOL_TYPE_LT,
    SYMBOL_TYPE_GT,
    SYMBOL_TYPE_LE,
    SYMBOL_TYPE_GE,
    SYMBOL_TYPE_POW,
    SYMBOL_TYPE_PLUS,
    SYMBOL_TYPE_MINUS,
    SYMBOL_TYPE_MUL,
    SYMBOL_TYPE_DIV,
    SYMBOL_TYPE_MOD,
    SYMBOL_TYPE_EPI,
    SYMBOL_TYPE_COMPOSITE,
    SYMBOL_TYPE_ATTR,
    SYMBOL_TYPE_ITEM,
    SYMBOL_TYPE_NAME,
    SYMBOL_TYPE_CALL,
    SYMBOL_TYPE_ARGUMENT,
    SYMBOL_TYPE_ARGUMENTS,
    SYMBOL_TYPE_OBJECT,
    SYMBOL_TYPE_OBJECT_PROPERTY,
    SYMBOL_TYPE_VALUE,
    SYMBOL_TYPE_PARENTHESIS,
    SYMBOL_TYPE_ARRAY,
    SYMBOL_TYPE_NULL,
    SYMBOL_TYPE_CHAR,
    SYMBOL_TYPE_STRING,
    SYMBOL_TYPE_NUMBER,
    SYMBOL_TYPE_BOOLEAN,
    SYMBOL_TYPE_ID
} symbol_type_t;

typedef enum symbol_flag {
    SYMBOL_FLAG_NONE            = 0 << 0,
    SYMBOL_FLAG_CHECKING        = 1 << 0,
    SYMBOL_FLAG_SYNTAX          = 1 << 1,
    SYMBOL_FLAG_REFERENCE       = 1 << 2
} symbol_flag_t;

typedef struct symbol {
    uint64_t id;
    uint32_t type;
    uint32_t flag;
    
    // statistics
    uint64_t reference;

    // declaration
	node_t *declaration;
    
    // list
    struct symbol *previous;
    struct symbol *next;

    // branche
    struct symbol *begin;
    struct symbol *end;

    struct symbol *parent;
} symbol_t;

symbol_t *
symbol_create(uint32_t type, node_t *declaration);

int
symbol_isempty(symbol_t *sym);

symbol_t*
symbol_next(symbol_t *current);

symbol_t*
symbol_previous(symbol_t *current);

uint64_t
symbol_count(symbol_t *sym);

int
symbol_query(symbol_t *sym, int (*f)(symbol_t*));

void
symbol_destroy(symbol_t *sym);

symbol_t*
symbol_link(symbol_t *sym, symbol_t *current, symbol_t *it);

symbol_t*
symbol_unlink(symbol_t *sym, symbol_t* it);

symbol_t*
symbol_sort(symbol_t *sym, int (*f)(symbol_t *, symbol_t *));

symbol_t*
symbol_remove(symbol_t *sym, int (*f)(symbol_t *));

symbol_t*
symbol_clear(symbol_t *sym);

symbol_t*
symbol_rpop(symbol_t *sym);

symbol_t *
symbol_rpush(symbol_t *sym, uint32_t type, node_t *declaration);

symbol_t*
symbol_lpop(symbol_t *sym);

symbol_t *
symbol_lpush(symbol_t *sym, uint32_t type, node_t *declaration);

symbol_t *
symbol_at(symbol_t *sym, uint64_t key);

symbol_t *
symbol_first(symbol_t *sym);

symbol_t *
symbol_last(symbol_t *sym);

int32_t
symbol_check_type(symbol_t *sym, uint32_t type);

void
symbol_clear_flag(symbol_t *sym, uint32_t flag);

void
symbol_set_flag(symbol_t *sym, uint32_t flag);

void
symbol_toggle_flag(symbol_t *sym, uint32_t flag);

int32_t
symbol_check_flag(symbol_t *sym, uint32_t flag);

#endif