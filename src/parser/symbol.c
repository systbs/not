#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "parser.h"
#include "forecast.h"
#include "error.h"
#include "binding.h"
#include "symbol.h"

uint64_t symbol_counter = 0;

symbol_t *
symbol_create(symbol_t *parent, node_t *declaration, char *name, uint64_t flags)
{
    symbol_t *symbol;
    symbol = (symbol_t *)malloc(sizeof(symbol_t));
    if(!symbol)
    {
        fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(symbol_t));
        return NULL;
    }
    symbol->escaped_name = name;
    symbol->flags = flags;
    symbol->parent = parent;
    symbol->declaration = declaration;
    symbol->id = symbol_counter++;
    
    symbol->members = list_create();
    if(!symbol->members)
    {
        return NULL;
    }

    return symbol;
}