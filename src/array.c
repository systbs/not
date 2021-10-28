#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "array.h"
#include "memory.h"

array_t *
array_apply(array_t *arr)
{
    iarray_t *it;

    if(!(it = (iarray_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->next = it->previous = it;
    arr->end = arr->begin = it;

    return arr;
}

array_t *
array_create()
{
    array_t *arr;

    if(!(arr = (array_t *)qalam_malloc(sizeof(*arr)))) {
        return 0;
    }

    return array_apply(arr);
}

value_t
array_isempty(array_t *arr)
{
    return (arr->begin == arr->end);
}

value_t
array_content(iarray_t *current){
    return current ? current->value : 0;
}

iarray_t*
array_next(iarray_t *current)
{
    return current->next;
}

iarray_t*
array_previous(iarray_t *current)
{
    return current->previous;
}

value_t
array_count(array_t *arr)
{
    value_t cnt = 0;
    iarray_t *b;
    for(b = arr->begin; b && (b != arr->end); b = b->next){
        cnt++;
    }
    return cnt;
}

value_t
array_clear(array_t *arr, value_t (*f)(iarray_t*))
{
    if (array_isempty(arr))
        return 0;

    iarray_t *b, *n;
    for(b = arr->begin; b && (b != arr->end); b = n){
        n = b->next;
        if(!(*f)(b)){
          return 0;
        }
    }

    return 1;
}

void
array_destroy(array_t *arr, value_t (*f)(iarray_t*))
{
    array_clear(arr, *f);
    qalam_free (arr);
}

iarray_t*
array_link(array_t *arr, iarray_t *current, iarray_t *it)
{
    it->next = current;
    it->previous = current->previous;
    current->previous->next = it;
    current->previous = it;

    if(arr->begin == current){
        arr->begin = it;
    }

    return it;
}

iarray_t*
array_unlink(array_t *arr, iarray_t* it)
{
    if (it == arr->end){
        return 0;
    }

    if (it == arr->begin){
        arr->begin = it->next;
    }

    it->next->previous = it->previous;
    it->previous->next = it->next;

    return it;
}

iarray_t*
array_remove(array_t *arr, value_t (*f)(value_t))
{
    iarray_t *b, *n;
    for(b = arr->begin; b != arr->end; b = n){
        n = b->next;
        if((*f)(b->value)){
            return array_unlink(arr, b);
        }
    }
    return 0;
}

iarray_t*
array_rpop(array_t *arr)
{
    return array_unlink(arr, arr->end->previous);
}

iarray_t *
array_rpush(array_t *arr, value_t value)
{
    iarray_t *it;
    if(!(it = (iarray_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return array_link(arr, arr->end, it);
}

iarray_t*
array_lpop(array_t *arr)
{
    return array_unlink(arr, arr->begin);
}

iarray_t *
array_lpush(array_t *arr, value_t value)
{
    iarray_t *it;

    if(!(it = (iarray_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return array_link(arr, arr->begin, it);
}

iarray_t *
array_insert(array_t *arr, value_t n, value_t value)
{
    iarray_t *current = arr->begin;
    for (value_t i = 0; i < n; i++)
    {
        if (current == arr->end) {
            return 0;
        }
        current = current->next;
    }

    iarray_t *it;

    if(!(it = (iarray_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = value;

    return array_link(arr, current, it);
}

value_t
array_null(array_t *arr)
{
    if(arr == 0){
        return 0;
    }
    return (arr->begin == arr->end);
}

iarray_t *
array_at(array_t *arr, value_t key)
{
    iarray_t *b, *n;
    for(b = arr->begin; b && (b != arr->end); b = n){
        n = b->next;
        if (key-- <= 0){
            return b;
        }
    }

    if(b == arr->end){
        return 0;
    }

    iarray_t *it;
    if(!(it = (iarray_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = 0;

    if (arr->begin == arr->end)
    {
        it->next = arr->end;
        it->previous = arr->end;
        arr->begin = it;
        arr->end->next = it;
        arr->end->previous = it;
    }
    else
    {
        it->next = arr->end;
        it->previous = arr->end->previous;
        arr->end->previous->next = it;
        arr->end->previous = it;
    }

    return it;
}

iarray_t *
array_first(array_t *arr)
{
    if(arr->begin != 0)
        return arr->begin;
    return 0;
}

iarray_t *
array_last(array_t *arr)
{
    if(arr->end->previous != 0)
        return arr->end->previous;
    return 0;
}

iarray_t *
array_first_or_default(array_t *arr, value_t (*f)(value_t))
{
    iarray_t *b, *n;
    for(b = arr->begin; b && (b != arr->end); b = n){
        n = b->next;
        if ((*f)(b->value)){
            return b;
        }
    }
    return 0;
}

iarray_t *
array_last_or_default(array_t *arr, value_t (*f)(value_t))
{
    iarray_t *b, *p;
    for(b = arr->end->previous; b && (b != arr->end); b = p){
        p = b->previous;
        if ((*f)(b->value)){
            return b;
        }
    }
    return 0;
}

value_t
array_aggregate(array_t *arr, value_t(*f)(value_t, value_t))
{
    if (array_isempty(arr))
        return 0;

    value_t result = 0;

    iarray_t *b, *n;
    for(b = arr->begin; b && (b != arr->end); b = n){
        n = b->next;
        result = (*f)(b->value, result);
    }

    return result;
}


value_t 
array_ateq(array_t *arr, value_t key, value_t value)
{
    iarray_t *b, *n;
    for(b = arr->begin; b && (b != arr->end); b = n){
        n = b->next;
        if (key-- <= 0){
            return b->value == value;
        }
    }

    if(b == arr->end){
        return 0;
    }

    iarray_t *it;
    if(!(it = (iarray_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }

    it->value = 0;

    if (arr->begin == arr->end)
    {
        it->next = arr->end;
        it->previous = arr->end;
        arr->begin = it;
        arr->end->next = it;
        arr->end->previous = it;
    }
    else
    {
        it->next = arr->end;
        it->previous = arr->end->previous;
        arr->end->previous->next = it;
        arr->end->previous = it;
    }

    return it->value == value;
}


value_t 
array_lasteq(array_t *arr, value_t value)
{
    if(arr->end->previous != 0){
        iarray_t *it = arr->end->previous;
        return (it && it->value == value);
    }
    return 0;
}