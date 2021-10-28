#pragma once

typedef struct iarray {
    struct iarray *previous;
    struct iarray *next;
    value_t value;
} iarray_t;

typedef struct array {
    iarray_t *begin;
    iarray_t *end;
} array_t;


array_t *
array_apply(array_t *que);

array_t *
array_create();

value_t
array_isempty(array_t *que);

value_t
array_content(iarray_t *current);

iarray_t*
array_next(iarray_t *current);

iarray_t*
array_previous(iarray_t *current);

value_t
array_count(array_t *que);

value_t
array_clear(array_t *que, value_t (*f)(iarray_t*));

void
array_destroy(array_t *que, value_t (*f)(iarray_t*));

iarray_t*
array_unlink(array_t *que, iarray_t* it);

iarray_t*
array_remove(array_t *que, value_t (*f)(value_t));

iarray_t*
array_rpop(array_t *que);

iarray_t *
array_rpush(array_t *que, value_t value);

iarray_t*
array_lpop(array_t *que);

iarray_t *
array_lpush(array_t *que, value_t value);

iarray_t *
array_insert(array_t *que, value_t n, value_t value);

value_t
array_null(array_t *que);

iarray_t *
array_at(array_t *que, value_t key);

iarray_t *
array_first(array_t *que);

iarray_t *
array_last(array_t *que);

iarray_t *
array_first_or_default(array_t *que, value_t (*f)(value_t));

iarray_t *
array_last_or_default(array_t *que, value_t (*f)(value_t));

value_t
array_aggregate(array_t *que, value_t(*f)(value_t, value_t));

value_t 
array_lasteq(array_t *arr, value_t value);
