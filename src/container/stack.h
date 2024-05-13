#pragma once
#ifndef __STACK_H__
#define __STACK_H__ 1

typedef struct istack {
    struct istack *previous;
    struct istack *next;
    int64_t value;
} istack_t;

typedef struct stack {
    istack_t *begin;
    istack_t *end;
} stack_t;

stack_t *
stack_apply(stack_t *res);

stack_t *
stack_create();

int
stack_isempty(stack_t *res);

istack_t*
stack_next(istack_t *current);

istack_t*
stack_previous(istack_t *current);

uint64_t
stack_count(stack_t *res);

int32_t
stack_query(stack_t *res, int (*f)(istack_t*));

void
stack_destroy(stack_t *res);

void
stack_link(stack_t *res, istack_t *current, istack_t *it);

void
stack_unlink(stack_t *res, istack_t* it);

void
stack_sort(stack_t *res, int (*f)(istack_t *, istack_t *));

void
stack_unlinkby(stack_t *res, int (*f)(istack_t *));

void
stack_clear(stack_t *res);

istack_t*
stack_rpop(stack_t *res);

istack_t *
stack_rpush(stack_t *res, int64_t value);

istack_t*
stack_lpop(stack_t *res);

istack_t *
stack_lpush(stack_t *res, int64_t value);

istack_t *
stack_insert(stack_t *res, istack_t *current, int64_t value);

istack_t *
stack_first(stack_t *res);

istack_t *
stack_last(stack_t *res);

#endif