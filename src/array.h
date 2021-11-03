#pragma once

typedef struct iarray {
    struct iarray *previous;
    struct iarray *next;
    arval_t value;
} iarray_t;

typedef struct array {
    iarray_t *begin;
    iarray_t *end;
} array_t;


array_t *
array_apply(array_t *que);

array_t *
array_create();

arval_t
array_isempty(array_t *que);

arval_t
array_content(iarray_t *current);

iarray_t*
array_next(iarray_t *current);

iarray_t*
array_previous(iarray_t *current);

arval_t
array_count(array_t *que);

arval_t
array_clear(array_t *que, arval_t (*f)(iarray_t*));

arval_t
array_free(iarray_t *it);

void
array_destroy(array_t *que, arval_t (*f)(iarray_t*));

iarray_t*
array_link(array_t *arr, iarray_t *current, iarray_t *it);

iarray_t*
array_unlink(array_t *que, iarray_t* it);

iarray_t*
array_remove(array_t *que, arval_t (*f)(arval_t));

iarray_t*
array_rpop(array_t *que);

iarray_t *
array_rpush(array_t *que, arval_t value);

iarray_t*
array_lpop(array_t *que);

iarray_t *
array_lpush(array_t *que, arval_t value);

iarray_t *
array_insert(array_t *que, arval_t n, arval_t value);

arval_t
array_null(array_t *que);

iarray_t *
array_at(array_t *que, arval_t key);

iarray_t *
array_first(array_t *que);

iarray_t *
array_last(array_t *que);

iarray_t *
array_first_or_default(array_t *que, arval_t (*f)(arval_t));

iarray_t *
array_last_or_default(array_t *que, arval_t (*f)(arval_t));

arval_t
array_aggregate(array_t *que, arval_t(*f)(arval_t, arval_t));

arval_t 
array_lasteq(array_t *arr, arval_t value);

iarray_t *
array_find(array_t *arr, arval_t value);

iarray_t *
array_findBefore(array_t *arr, iarray_t *n, arval_t value);

iarray_t *
array_findAfter(array_t *arr, iarray_t *n, arval_t value);