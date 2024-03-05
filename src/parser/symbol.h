#pragma once
#ifndef __SYMBOL_H__
#define __SYMBOL_H__

typedef enum symbol_type {
    SYMBOL_ROOT						= 1,
    SYMBOL_VAR,
	SYMBOL_MODULE,
    SYMBOL_PROPERTY,
    SYMBOL_FUNCTION,
    SYMBOL_LAMBDA,
    SYMBOL_CLASS,
    SYMBOL_ENUM,
    SYMBOL_MEMBER,
    SYMBOL_MEMBERS,
    SYMBOL_TYPE,
    SYMBOL_METHOD,
    SYMBOL_GENERIC,
    SYMBOL_GENERICS,
    SYMBOL_PARAMETER,
    SYMBOL_PARAMETERS,
    SYMBOL_FIELD,
    SYMBOL_FIELDS,
    SYMBOL_TYPE_EXPRESSION,
    SYMBOL_EXPORT,
    SYMBOL_IMPORT,
    SYMBOL_PATH,
    SYMBOL_HERITAGE,
    SYMBOL_HERITAGES,
    SYMBOL_THROW,
    SYMBOL_RETURN,
    SYMBOL_RETURN_TYPE,
    SYMBOL_CONTINUE,
    SYMBOL_BREAK,
    SYMBOL_FORIN ,
    SYMBOL_EXPRESSION,
    SYMBOL_FOR,
    SYMBOL_INITIALIZER,
    SYMBOL_INCREMENTOR,
    SYMBOL_BLOCK,
    SYMBOL_CATCH,
    SYMBOL_CATCHS,
    SYMBOL_TRY,
    SYMBOL_IF,
    SYMBOL_CONDITION,
    SYMBOL_ELSE,

    SYMBOL_DEFINE,
    SYMBOL_ASSIGN,
    SYMBOL_ADD_ASSIGN,
    SYMBOL_SUB_ASSIGN,
    SYMBOL_DIV_ASSIGN,
    SYMBOL_MUL_ASSIGN,
    SYMBOL_MOD_ASSIGN,
    SYMBOL_AND_ASSIGN,
    SYMBOL_OR_ASSIGN,
    SYMBOL_SHL_ASSIGN,
    SYMBOL_SHR_ASSIGN,

    SYMBOL_CONDITIONAL,
    SYMBOL_TRUE,
    SYMBOL_FALSE,
    SYMBOL_LEFT,
    SYMBOL_RIGHT,
    SYMBOL_TILDE,
	SYMBOL_NOT,
	SYMBOL_NEG,
	SYMBOL_POS,
	SYMBOL_AWAIT,
	SYMBOL_SIZEOF,
	SYMBOL_TYPEOF,
	SYMBOL_ELLIPSIS,
    SYMBOL_SHL,
    SYMBOL_SHR,
    SYMBOL_LOR,
    SYMBOL_LAND,
    SYMBOL_OR,
    SYMBOL_XOR,
    SYMBOL_AND,
    SYMBOL_EQ,
    SYMBOL_IN,
    SYMBOL_EXTENDS,
    SYMBOL_NEQ,
    SYMBOL_LT,
    SYMBOL_GT,
    SYMBOL_LE,
    SYMBOL_GE,
    SYMBOL_POW,
    SYMBOL_PLUS,
    SYMBOL_MINUS,
    SYMBOL_MUL,
    SYMBOL_DIV,
    SYMBOL_MOD,
    SYMBOL_EPI,
    SYMBOL_COMPOSITE,
    SYMBOL_ATTR,
    SYMBOL_ITEM,
    SYMBOL_NAME,
    SYMBOL_CALL,
    SYMBOL_ARGUMENT,
    SYMBOL_ARGUMENTS,
    SYMBOL_OBJECT,
    SYMBOL_OBJECT_PROPERTY,
    SYMBOL_KEY,
    SYMBOL_VALUE,
    SYMBOL_PARENTHESIS,
    SYMBOL_ARRAY,
    SYMBOL_NULL,
    SYMBOL_CHAR,
    SYMBOL_STRING,
    SYMBOL_NUMBER,
    SYMBOL_BOOLEAN,
    SYMBOL_ID
} symbol_type_t;

typedef enum symbol_flag {
    SYMBOL_FLAG_NONE            = 0 << 0,
    SYMBOL_FLAG_INITIALIZE      = 1 << 0,
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

    // branch
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