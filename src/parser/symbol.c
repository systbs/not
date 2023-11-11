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
symbol_create(char *name, uint64_t flags, node_t *declaration)
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
    symbol->declaration = declaration;
    symbol->id = symbol_counter++;

    return symbol;
}

int32_t
symbol_check_flag(symbol_t *symbol, uint64_t flag)
{
    return ((symbol->flags & flag) == flag);
}

void
symbol_clear_flag(symbol_t *symbol, uint64_t flag)
{
    symbol->flags &= ~flag;
}

void
symbol_set_flag(symbol_t *symbol, uint64_t flag)
{
    symbol->flags |= flag;
}

void
symbol_toggle_flag(symbol_t *symbol, uint64_t flag)
{
    symbol->flags ^= flag;
}