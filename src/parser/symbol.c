#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "symbol.h"

uint64_t symbol_counter = 0;

symbol_t *
symbol_apply(symbol_t *sym)
{
    symbol_t *it;

    if(!(it = (symbol_t *)malloc(sizeof(symbol_t)))) {
        return NULL;
    }
    memset(it, 0, sizeof(symbol_t));

    it->next = it->previous = it;
    sym->end = sym->begin = it;

    return sym;
}

symbol_t *
symbol_create(uint64_t flags, node_t *declaration)
{
    symbol_t *sym;

    if(!(sym = (symbol_t *)malloc(sizeof(*sym)))) {
        return NULL;
    }
    memset(sym, 0, sizeof(symbol_t));

    sym->flags = flags;
    sym->declaration = declaration;
    sym->id = symbol_counter++;

    return symbol_apply(sym);
}

int
symbol_isempty(symbol_t *sym)
{
    return (sym->begin == sym->end);
}

symbol_t*
symbol_next(symbol_t *current)
{
    return current->next;
}

symbol_t*
symbol_previous(symbol_t *current)
{
    return current->previous;
}

uint64_t
symbol_count(symbol_t *sym)
{
    uint64_t cnt = 0;
    symbol_t *b;
    for(b = sym->begin; b && (b != sym->end); b = b->next){
        cnt++;
    }
    return cnt;
}

int
symbol_query(symbol_t *sym, int (*f)(symbol_t*))
{
    if (symbol_isempty(sym))
        return 0;

    symbol_t *b, *n;
    for(b = sym->begin; b && (b != sym->end); b = n){
        n = b->next;
        if(!(*f)(b)){
          return 0;
        }
    }

    return 1;
}

void
symbol_destroy(symbol_t *sym)
{
    symbol_clear(sym);
    free (sym);
}

symbol_t *
symbol_create_iterior(uint64_t flags, node_t *declaration)
{
	symbol_t *it;
    if(!(it = (symbol_t *)malloc(sizeof(*it)))) {
        return NULL;
    }
    memset(it, 0, sizeof(symbol_t));

    symbol_apply(it);

    it->flags = flags;
    it->declaration = declaration;
    it->id = symbol_counter++;
    
    return it;
}

symbol_t*
symbol_link(symbol_t *sym, symbol_t *current, symbol_t *it)
{
    it->next = current;
    it->previous = current->previous;
    current->previous->next = it;
    current->previous = it;

    if(sym->begin == current){
        sym->begin = it;
    }

    return it;
}

symbol_t*
symbol_unlink(symbol_t *sym, symbol_t* it)
{
    if (it == sym->end) {
        return 0;
    }

    if (it == sym->begin) {
        sym->begin = it->next;
    }

    it->next->previous = it->previous;
    it->previous->next = it->next;

    return it;
}

symbol_t*
symbol_sort(symbol_t *sym, int (*f)(symbol_t *, symbol_t *))
{
    symbol_t *b, *n;
    for(b = sym->begin; b != sym->end; b = n){
        n = b->next;
        if(n != sym->end){
            if((*f)(b, n)){
                symbol_unlink(sym, b);
                symbol_link(sym, n, b);
            }
        }
    }
    return 0;
}

symbol_t*
symbol_remove(symbol_t *sym, int (*f)(symbol_t *))
{
    symbol_t *b, *n;
    for(b = sym->begin; b != sym->end; b = n){
        n = b->next;
        if((*f)(b)){
            return symbol_unlink(sym, b);
        }
    }
    return 0;
}

symbol_t*
symbol_clear(symbol_t *sym)
{
    symbol_t *b, *n;
    for(b = sym->begin; b != sym->end; b = n){
        n = b->next;
        symbol_unlink(sym, b);
        free(b);
    }
    return sym;
}

symbol_t*
symbol_rpop(symbol_t *sym)
{
	if(symbol_isempty(sym)){
		return NULL;
	}
    return symbol_unlink(sym, sym->end->previous);
}

symbol_t *
symbol_rpush(symbol_t *sym, uint64_t flags, node_t *declaration)
{
    symbol_t *it;
    if(!(it = (symbol_t *)malloc(sizeof(*it)))) {
        return NULL;
    }
    memset(it, 0, sizeof(symbol_t));

    symbol_apply(it);

    it->flags = flags;
    it->declaration = declaration;
    it->id = symbol_counter++;

    return symbol_link(sym, sym->end, it);
}

symbol_t*
symbol_lpop(symbol_t *sym)
{
    return symbol_unlink(sym, sym->begin);
}

symbol_t *
symbol_lpush(symbol_t *sym, uint64_t flags, node_t *declaration)
{
    symbol_t *it;

    if(!(it = (symbol_t *)malloc(sizeof(*it)))) {
        return NULL;
    }
    memset(it, 0, sizeof(symbol_t));

    symbol_apply(it);
    
    it->flags = flags;
    it->declaration = declaration;
    it->id = symbol_counter++;

    return symbol_link(sym, sym->begin, it);
}

symbol_t *
symbol_at(symbol_t *sym, uint64_t key)
{
    symbol_t *b, *n;
    for(b = sym->begin; b && (b != sym->end); b = n){
        n = b->next;
        if (key-- <= 0){
            return b;
        }
    }

    if(b == sym->end){
        return NULL;
    }

    symbol_t *it;
    if(!(it = (symbol_t *)malloc(sizeof(*it)))) {
        return NULL;
    }

    it->id = symbol_counter++;

    symbol_apply(it);

    if (sym->begin == sym->end)
    {
        it->next = sym->end;
        it->previous = sym->end;
        sym->begin = it;
        sym->end->next = it;
        sym->end->previous = it;
    }
    else
    {
        it->next = sym->end;
        it->previous = sym->end->previous;
        sym->end->previous->next = it;
        sym->end->previous = it;
    }

    return it;
}

symbol_t *
symbol_first(symbol_t *sym)
{
    if(sym->begin != 0)
        return sym->begin;
    return NULL;
}

symbol_t *
symbol_last(symbol_t *sym)
{
    if(sym->end->previous != 0 && sym->end->previous != sym->end)
        return sym->end->previous;
    return NULL;
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

symbol_t *
symbol_find_up(symbol_t *sym, uint64_t flag)
{
    symbol_t *b, *n, *end = sym->previous;
    for(b = sym; b && (b != end); b = n){
        n = b->next;
        if(symbol_check_flag(b, flag))
        {
            return b;
        }
    }
    if(sym->parent)
    {
        return symbol_find_up(sym->parent, flag);
    }

    return NULL;
}

symbol_t *
symbol_find_down(symbol_t *sym, uint64_t flag)
{
    symbol_t *b, *n;
    for(b = sym->begin; b && (b != sym->end); b = n){
        n = b->next;
        if(symbol_check_flag(b, flag))
        {
            return b;
        }
    }
    return NULL;
}