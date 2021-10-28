#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "table.h"
#include "memory.h"

table_t *
table_apply(table_t *tbl)
{
    itable_t *it;

    if(!(it = (itable_t *)qalam_malloc(sizeof(itable_t)))) {
        return 0;
    }

    it->next = it->previous = it;
    tbl->end = tbl->begin = it;

    return tbl;
}

table_t *
table_create()
{
    table_t *tbl;

    if(!(tbl = (table_t *)qalam_malloc(sizeof(*tbl)))) {
        return 0;
    }

    return table_apply(tbl);
}

value_t
table_isempty(table_t *tbl)
{
    return (tbl->begin == tbl->end);
}

value_p
table_content(itable_t *current){
    return current ? current->value : 0;
}

itable_t*
table_next(itable_t *current)
{
    return current->next;
}

itable_t*
table_previous(itable_t *current)
{
    return current->previous;
}

value_t
table_count(table_t *tbl)
{
    value_t cnt = 0;
    itable_t *b;
    for(b = tbl->begin; b && (b != tbl->end); b = b->next){
        cnt++;
    }
    return cnt;
}

value_t
table_query(table_t *tbl, value_t (*f)(itable_t*))
{
    if (table_isempty(tbl))
        return 0;

    itable_t *b, *n;
    for(b = tbl->begin; b && (b != tbl->end); b = n){
        n = b->next;
        if(!(*f)(b)){
          return 0;
        }
    }

    return 1;
}

void
table_destroy(table_t *tbl)
{
    table_clear(tbl);
    qalam_free (tbl);
}

itable_t*
table_link(table_t *tbl, itable_t *current, itable_t *it)
{
    it->next = current;
    it->previous = current->previous;
    current->previous->next = it;
    current->previous = it;

    if(tbl->begin == current){
        tbl->begin = it;
    }

    return it;
}

itable_t*
table_unlink(table_t *tbl, itable_t* it)
{
    if (it == tbl->end){
        return 0;
    }

    if (it == tbl->begin){
        tbl->begin = it->next;
    }

    it->next->previous = it->previous;
    it->previous->next = it->next;

    return it;
}

itable_t*
table_sort(table_t *tbl, value_t (*f)(value_p, value_p))
{
    itable_t *b, *n;
    for(b = tbl->begin; b != tbl->end; b = n){
        n = b->next;
        if(n != tbl->end){
            if((*f)(b->value, n->value)){
                table_unlink(tbl, b);
                table_link(tbl, n, b);
            }
        }
    }
    return 0;
}

itable_t*
table_remove(table_t *tbl, value_t (*f)(value_p))
{
    itable_t *b, *n;
    for(b = tbl->begin; b != tbl->end; b = n){
        n = b->next;
        if((*f)(b->value)){
            return table_unlink(tbl, b);
        }
    }
    return 0;
}

itable_t*
table_clear(table_t *tbl)
{
    itable_t *b, *n;
    for(b = tbl->begin; b != tbl->end; b = n){
        n = b->next;
        return table_unlink(tbl, b);
    }
    return 0;
}

itable_t*
table_rpop(table_t *tbl)
{
    return table_unlink(tbl, tbl->end->previous);
}

itable_t *
table_rpush(table_t *tbl, value_p value)
{
    itable_t *it;
    if(!(it = (itable_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return table_link(tbl, tbl->end, it);
}

itable_t*
table_lpop(table_t *tbl)
{
    return table_unlink(tbl, tbl->begin);
}

itable_t *
table_lpush(table_t *tbl, value_p value)
{
    itable_t *it;

    if(!(it = (itable_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return table_link(tbl, tbl->begin, it);
}

itable_t *
table_insert(table_t *tbl, itable_t *current, value_p value){
    itable_t *it;

    if(!(it = (itable_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return table_link(tbl, current, it);
}

value_t
table_null(table_t *tbl)
{
    if(tbl == 0){
        return 0;
    }
    return (tbl->begin == tbl->end);
}

itable_t *
table_at(table_t *tbl, value_t key)
{
    itable_t *b, *n;
    for(b = tbl->begin; b && (b != tbl->end); b = n){
        n = b->next;
        if (key-- <= 0){
            return b;
        }
    }

    if(b == tbl->end){
        return 0;
    }

    itable_t *it;
    if(!(it = (itable_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = 0;

    if (tbl->begin == tbl->end)
    {
        it->next = tbl->end;
        it->previous = tbl->end;
        tbl->begin = it;
        tbl->end->next = it;
        tbl->end->previous = it;
    }
    else
    {
        it->next = tbl->end;
        it->previous = tbl->end->previous;
        tbl->end->previous->next = it;
        tbl->end->previous = it;
    }

    return it;
}

itable_t *
table_first(table_t *tbl)
{
    if(tbl->begin != 0)
        return tbl->begin;
    return 0;
}

itable_t *
table_last(table_t *tbl)
{
    if(tbl->end->previous != 0 && tbl->end->previous != tbl->end)
        return tbl->end->previous;
    return 0;
}

itable_t *
table_first_or_default(table_t *tbl, value_t (*f)(value_p))
{
    itable_t *b, *n;
    for(b = tbl->begin; b && (b != tbl->end); b = n){
        n = b->next;
        if ((*f)(b->value)){
            return b;
        }
    }
    return 0;
}

itable_t *
table_last_or_default(table_t *tbl, value_t (*f)(value_p))
{
    itable_t *b, *p;
    for(b = tbl->end->previous; b && (b != tbl->end); b = p){
        p = b->previous;
        if ((*f)(b->value)){
            return b;
        }
    }
    return 0;
}

value_p
table_aggregate(table_t *tbl, value_p(*f)(value_p, value_p))
{
    if (table_isempty(tbl))
        return 0;

    value_p result = 0;

    itable_t *b, *n;
    for(b = tbl->begin; b && (b != tbl->end); b = n){
        n = b->next;
        result = (*f)(b->value, result);
    }

    return result;
}
