#pragma once
#ifndef __SYMBOL_H__
#define __SYMBOL_H__

typedef enum symbol_flag {
    SYMBOL_FLAG_NONE						= 1,
    SYMBOL_FLAG_VARIABLE,
	SYMBOL_FLAG_MODULE,
	SYMBOL_FLAG_CONST,
    SYMBOL_FLAG_PROPERTY,
    SYMBOL_FLAG_PROPERTY_TYPE,
    SYMBOL_FLAG_PROPERTY_VALUE,
    SYMBOL_FLAG_FUNCTION,
    SYMBOL_FLAG_CLASS,
    SYMBOL_FLAG_ENUM,
    SYMBOL_FLAG_ENUM_MEMBER,
    SYMBOL_FLAG_TYPE,
    SYMBOL_FLAG_METHOD,
    SYMBOL_FLAG_CONSTRUCTOR,
    SYMBOL_FLAG_TYPE_PARAMETER,
    SYMBOL_FLAG_TYPE_PARAMETER_EXTENDS,
    SYMBOL_FLAG_TYPE_PARAMETER_VALUE,
    SYMBOL_FLAG_PARAMETER,
    SYMBOL_FLAG_PARAMETER_TYPE,
    SYMBOL_FLAG_PARAMETER_VALUE,
    SYMBOL_FLAG_FIELD,
    SYMBOL_FLAG_FIELD_TYPE,
    SYMBOL_FLAG_EXPORT,
    SYMBOL_FLAG_IMPORT,
    SYMBOL_FLAG_PATH,
    SYMBOL_FLAG_HERITAGE,
    SYMBOL_FLAG_HERITAGE_TYPE,
    SYMBOL_FLAG_THROW,
    SYMBOL_FLAG_RETURN,
    SYMBOL_FLAG_CONTINUE,
    SYMBOL_FLAG_BREAK,
    SYMBOL_FLAG_FORIN ,
    SYMBOL_FLAG_FORIN_INITIALIZER,
    SYMBOL_FLAG_FORIN_EXPRESSION,
    SYMBOL_FLAG_FOR,
    SYMBOL_FLAG_FOR_INITIALIZER,
    SYMBOL_FLAG_FOR_CONDITION,
    SYMBOL_FLAG_FOR_INCREMENTOR,
    SYMBOL_FLAG_FOR_STEP,
    SYMBOL_FLAG_FOR_INIT,
    SYMBOL_FLAG_BLOCK,
    SYMBOL_FLAG_TRY,
    SYMBOL_FLAG_IF,
    SYMBOL_FLAG_IF_CONDITION,
    SYMBOL_FLAG_ELSE,
    SYMBOL_FLAG_ASSIGN,
    SYMBOL_FLAG_ASSIGN_LEFT,
    SYMBOL_FLAG_ASSIGN_RIGHT,
    SYMBOL_FLAG_CONDITIONAL,
    SYMBOL_FLAG_CONDITION,
    SYMBOL_FLAG_TRUE,
    SYMBOL_FLAG_FALSE,
    SYMBOL_FLAG_LEFT,
    SYMBOL_FLAG_RIGHT,
    SYMBOL_FLAG_LOR,
    SYMBOL_FLAG_LAND,
    SYMBOL_FLAG_OR,
    SYMBOL_FLAG_XOR,
    SYMBOL_FLAG_AND,
    SYMBOL_FLAG_EQ,
    SYMBOL_FLAG_IN,
    SYMBOL_FLAG_EXTENDS,
    SYMBOL_FLAG_NEQ,
    SYMBOL_FLAG_LT,
    SYMBOL_FLAG_GT,
    SYMBOL_FLAG_LE,
    SYMBOL_FLAG_GE,
    SYMBOL_FLAG_ATTR,
    SYMBOL_FLAG_GET_ITEM,
    SYMBOL_FLAG_GET_SLICE,
    SYMBOL_FLAG_INDEX,
    SYMBOL_FLAG_NAME,
    SYMBOL_FLAG_GET_START,
    SYMBOL_FLAG_GET_STEP,
    SYMBOL_FLAG_GET_STOP,
    SYMBOL_FLAG_CALL,
    SYMBOL_FLAG_CALLABLE,
    SYMBOL_FLAG_BASE,
    SYMBOL_FLAG_TYPE_ARGUMENT,
    SYMBOL_FLAG_OBJECT,
    SYMBOL_FLAG_OBJECT_PROPERTY,
    SYMBOL_FLAG_VALUE,
    SYMBOL_FLAG_PARENTHESIS,
    SYMBOL_FLAG_ARRAY,
    SYMBOL_FLAG_NULL,
    SYMBOL_FLAG_CHAR,
    SYMBOL_FLAG_STRING,
    SYMBOL_FLAG_NUMBER,
    SYMBOL_FLAG_ID
} symbol_flag_t;

typedef struct symbol {
    uint64_t id;
    uint64_t flags;
	
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
symbol_create(uint64_t flags, node_t *declaration);

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

symbol_t *
symbol_create_iterior(uint64_t flags, node_t *declaration);

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
symbol_rpush(symbol_t *sym, uint64_t flags, node_t *declaration);

symbol_t*
symbol_lpop(symbol_t *sym);

symbol_t *
symbol_lpush(symbol_t *sym, uint64_t flags, node_t *declaration);

symbol_t *
symbol_at(symbol_t *sym, uint64_t key);

symbol_t *
symbol_first(symbol_t *sym);

symbol_t *
symbol_last(symbol_t *sym);

int32_t
symbol_check_flag(symbol_t *symbol, uint64_t flag);

void
symbol_clear_flag(symbol_t *symbol, uint64_t flag);

void
symbol_set_flag(symbol_t *symbol, uint64_t flag);

void
symbol_toggle_flag(symbol_t *symbol, uint64_t flag);

#endif