#pragma once
#ifndef __SYMBOL_H__
#define __SYMBOL_H__

typedef enum symbol_flag {
    SYMBOL_FLAG_NONE						= 0,
    SYMBOL_FLAG_VARIABLE	                = 1 << 0,
	SYMBOL_FLAG_MODULE						= 1 << 1,
	SYMBOL_FLAG_CONST						= 1 << 2,
    SYMBOL_FLAG_PROPERTY					= 1 << 3,
    SYMBOL_FLAG_ENUM_MEMBER					= 1 << 4,
    SYMBOL_FLAG_FUNCTION					= 1 << 5,
    SYMBOL_FLAG_CLASS						= 1 << 6,
    SYMBOL_FLAG_ENUM						= 1 << 7,
    SYMBOL_FLAG_NAMESPACE					= 1 << 8,
    SYMBOL_FLAG_TYPE						= 1 << 9,
    SYMBOL_FLAG_METHOD						= 1 << 10,
    SYMBOL_FLAG_CONSTRUCTOR					= 1 << 11,
    SYMBOL_FLAG_SIGNATURE					= 1 << 12,
    SYMBOL_FLAG_TYPE_PARAMETER				= 1 << 13,
    SYMBOL_FLAG_PARAMETER				    = 1 << 14,
    SYMBOL_FLAG_FIELD				        = 1 << 15,
    SYMBOL_FLAG_EXPORT						= 1 << 16,
    SYMBOL_FLAG_IMPORT						= 1 << 17
} symbol_flag_t;

typedef struct symbol {
    uint64_t id;
    uint64_t flags;
	
    char *escaped_name;

	node_t *declaration;
} symbol_t;

symbol_t *
symbol_create(char *name, uint64_t flags, node_t *declaration);

int32_t
symbol_check_flag(symbol_t *symbol, uint64_t flag);

#endif