#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "list.h"

list_t *
list_apply(list_t *lst)
{
    ilist_t *it;

    if(!(it = (ilist_t *)malloc(sizeof(ilist_t)))) {
        return NULL;
    }
    memset(it, 0, sizeof(ilist_t));

    it->next = it->previous = it;
    lst->end = lst->begin = it;

    return lst;
}

list_t *
list_create()
{
    list_t *lst;

    if(!(lst = (list_t *)malloc(sizeof(*lst)))) {
        return NULL;
    }
    memset(lst, 0, sizeof(list_t));

    return list_apply(lst);
}

int
list_isempty(list_t *lst)
{
    return (lst->begin == lst->end);
}

uint64_t
list_content(ilist_t *current){
    return current ? current->value : 0;
}

ilist_t*
list_next(ilist_t *current)
{
    return current->next;
}

ilist_t*
list_previous(ilist_t *current)
{
    return current->previous;
}

uint64_t
list_count(list_t *lst)
{
    uint64_t cnt = 0;
    ilist_t *b;
    for(b = lst->begin; b && (b != lst->end); b = b->next){
        cnt++;
    }
    return cnt;
}

int
list_query(list_t *lst, int (*f)(ilist_t*))
{
    if (list_isempty(lst))
        return 0;

    ilist_t *b, *n;
    for(b = lst->begin; b && (b != lst->end); b = n){
        n = b->next;
        if(!(*f)(b)){
          return 0;
        }
    }

    return 1;
}

void
list_destroy(list_t *lst)
{
    list_clear(lst);
    free (lst);
}

void
list_free_it(ilist_t *it)
{
    free (it);
}

ilist_t *
list_create_iterior(uint64_t value){
	ilist_t *it;
    if(!(it = (ilist_t *)malloc(sizeof(*it)))) {
        return NULL;
    }
    memset(it, 0, sizeof(ilist_t));

    it->value = value;
    
    return it;
}

ilist_t*
list_link(list_t *lst, ilist_t *current, ilist_t *it)
{
    it->next = current;
    it->previous = current->previous;
    current->previous->next = it;
    current->previous = it;

    if(lst->begin == current){
        lst->begin = it;
    }

    return it;
}

ilist_t*
list_unlink(list_t *lst, ilist_t* it)
{
    if (it == lst->end){
        return 0;
    }

    if (it == lst->begin){
        lst->begin = it->next;
    }

    it->next->previous = it->previous;
    it->previous->next = it->next;

    return it;
}

ilist_t*
list_sort(list_t *lst, int (*f)(ilist_t *, ilist_t *))
{
    ilist_t *b, *n;
    for(b = lst->begin; b != lst->end; b = n){
        n = b->next;
        if(n != lst->end){
            if((*f)(b, n)){
                list_unlink(lst, b);
                list_link(lst, n, b);
            }
        }
    }
    return 0;
}

ilist_t*
list_remove(list_t *lst, int (*f)(ilist_t *))
{
    ilist_t *b, *n;
    for(b = lst->begin; b != lst->end; b = n){
        n = b->next;
        if((*f)(b)){
            return list_unlink(lst, b);
        }
    }
    return 0;
}

list_t*
list_clear(list_t *lst)
{
    ilist_t *b, *n;
    for(b = lst->begin; b != lst->end; b = n){
        n = b->next;
        list_unlink(lst, b);
        free(b);
    }
    return lst;
}

ilist_t*
list_rpop(list_t *lst)
{
	if(list_isempty(lst)){
		return NULL;
	}
    return list_unlink(lst, lst->end->previous);
}

ilist_t *
list_rpush(list_t *lst, uint64_t value)
{
    ilist_t *it;
    if(!(it = (ilist_t *)malloc(sizeof(*it)))) {
        return NULL;
    }
    memset(it, 0, sizeof(ilist_t));

    it->value = value;

    return list_link(lst, lst->end, it);
}

ilist_t*
list_lpop(list_t *lst)
{
    return list_unlink(lst, lst->begin);
}

ilist_t *
list_lpush(list_t *lst, uint64_t value)
{
    ilist_t *it;

    if(!(it = (ilist_t *)malloc(sizeof(*it)))) {
        return NULL;
    }
    memset(it, 0, sizeof(ilist_t));

    it->value = value;

    return list_link(lst, lst->begin, it);
}

ilist_t *
list_at(list_t *lst, uint64_t key)
{
    ilist_t *b, *n;
    for(b = lst->begin; b && (b != lst->end); b = n){
        n = b->next;
        if (key-- <= 0){
            return b;
        }
    }

    if(b == lst->end){
        return NULL;
    }

    ilist_t *it;
    if(!(it = (ilist_t *)malloc(sizeof(*it)))) {
        return NULL;
    }

    it->value = 0;

    if (lst->begin == lst->end)
    {
        it->next = lst->end;
        it->previous = lst->end;
        lst->begin = it;
        lst->end->next = it;
        lst->end->previous = it;
    }
    else
    {
        it->next = lst->end;
        it->previous = lst->end->previous;
        lst->end->previous->next = it;
        lst->end->previous = it;
    }

    return it;
}

ilist_t *
list_first(list_t *lst)
{
    if(lst->begin != 0)
        return lst->begin;
    return NULL;
}

ilist_t *
list_last(list_t *lst)
{
    if(lst->end->previous != 0 && lst->end->previous != lst->end)
        return lst->end->previous;
    return NULL;
}
