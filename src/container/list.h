#pragma once
#ifndef __LIST_H__
#define __LIST_H__

typedef struct ilist {
    struct ilist *previous;
    struct ilist *next;
    void *value;
} ilist_t;

typedef struct list {
    ilist_t *begin;
    ilist_t *end;
} list_t;

list_t *
list_create();

int
list_isempty(list_t *res);

ilist_t*
list_next(ilist_t *current);

ilist_t*
list_previous(ilist_t *current);

uint64_t
list_count(list_t *res);

int32_t
list_query(list_t *res, int (*f)(ilist_t*));

void
list_destroy(list_t *res);

void
list_link(list_t *res, ilist_t *current, ilist_t *it);

void
list_unlink(list_t *res, ilist_t* it);

void
list_sort(list_t *res, int (*f)(ilist_t *, ilist_t *));

void
list_unlink_by(list_t *res, int (*f)(ilist_t *));

void
list_clear(list_t *res);

ilist_t*
list_rpop(list_t *res);

ilist_t *
list_rpush(list_t *res, void *value);

ilist_t*
list_lpop(list_t *res);

ilist_t *
list_lpush(list_t *res, void *value);

ilist_t *
list_first(list_t *res);

ilist_t *
list_last(list_t *res);

#endif