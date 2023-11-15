#pragma once
#ifndef __LIST_H__
#define __LIST_H__

typedef unsigned long long list_key_t;
typedef intptr_t list_value_t;

typedef struct ilist {
    struct ilist *previous;
    struct ilist *next;
    list_value_t value;
} ilist_t;

typedef struct list {
    ilist_t *begin;
    ilist_t *end;
} list_t;

list_t *
list_apply(list_t *lst);

list_t *
list_create();

int
list_isempty(list_t *lst);

list_value_t
list_content(ilist_t *current);

ilist_t*
list_next(ilist_t *current);

ilist_t*
list_previous(ilist_t *current);

list_key_t
list_count(list_t *lst);

int
list_query(list_t *lst, int (*f)(ilist_t*));

void
list_destroy(list_t *lst);

ilist_t *
list_create_iterior(list_value_t value);

ilist_t*
list_link(list_t *lst, ilist_t *current, ilist_t *it);

ilist_t*
list_unlink(list_t *lst, ilist_t* it);

ilist_t*
list_sort(list_t *lst, int (*f)(ilist_t *, ilist_t *));

ilist_t*
list_remove(list_t *lst, int (*f)(ilist_t *));

list_t*
list_clear(list_t *lst);

ilist_t*
list_rpop(list_t *lst);

ilist_t *
list_rpush(list_t *lst, list_value_t value);

ilist_t*
list_lpop(list_t *lst);

ilist_t *
list_lpush(list_t *lst, list_value_t value);

ilist_t *
list_at(list_t *lst, list_key_t key);

ilist_t *
list_first(list_t *lst);

ilist_t *
list_last(list_t *lst);

#endif //__LIST_H__