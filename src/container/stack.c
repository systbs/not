#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "stack.h"

stack_t *
stack_apply(stack_t *res)
{
  istack_t *it = (istack_t *)malloc(sizeof(istack_t));
  if(it == NULL) {
    return NULL;
  }

  memset(it, 0, sizeof(istack_t));

  it->next = it->previous = it;
  res->end = res->begin = it;

  return res;
}

stack_t *
stack_create()
{
  stack_t *res = (stack_t *)malloc(sizeof(*res));
  if(!res) {
    return NULL;
  }
  memset(res, 0, sizeof(stack_t));

  return stack_apply(res);
}

int32_t
stack_isempty(stack_t *res)
{
  return (res->begin == res->end);
}

istack_t*
stack_next(istack_t *current)
{
  return current->next;
}

istack_t*
stack_previous(istack_t *current)
{
  return current->previous;
}

uint64_t
stack_count(stack_t *res)
{
  uint64_t cnt = 0;
  istack_t *b;
  for(b = res->begin; b && (b != res->end); b = b->next){
    cnt++;
  }
  return cnt;
}

int32_t
stack_query(stack_t *res, int (*f)(istack_t*))
{
  if (stack_isempty(res))
  {
    return 0;
  }

  istack_t *b, *n;
  for(b = res->begin; b && (b != res->end); b = n){
    n = b->next;
    if(!(*f)(b)){
      return 0;
    }
  }

  return 1;
}

void
stack_destroy(stack_t *res)
{
  stack_clear(res);
  free (res);
}

void
stack_link(stack_t *res, istack_t *current, istack_t *it)
{
  it->next = current;
  it->previous = current->previous;
  current->previous->next = it;
  current->previous = it;

  if(res->begin == current)
  {
      res->begin = it;
  }
}

void
stack_unlink(stack_t *res, istack_t* it)
{
  if (it == res->end)
  {
    return;
  }

  if (it == res->begin)
  {
    res->begin = it->next;
  }

  it->next->previous = it->previous;
  it->previous->next = it->next;
}

void
stack_sort(stack_t *res, int (*f)(istack_t *, istack_t *))
{
  istack_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    if(n != res->end){
      if((*f)(b, n)){
        stack_unlink(res, b);
        stack_link(res, n, b);
      }
    }
  }
}

void
stack_unlinkby(stack_t *res, int (*f)(istack_t *))
{
  istack_t *b, *n;
  for(b = res->begin; b != res->end; b = n) {
    n = b->next;
    if ((*f)(b)) {
      stack_unlink(res, b);
      break;
    }
  }
}

void
stack_clear(stack_t *res)
{
  istack_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    stack_unlink(res, b);
    free(b);
  }
}

istack_t *
stack_rpop(stack_t *res)
{
	if(stack_isempty(res)){
		return NULL;
	}
  istack_t *it = res->end->previous;
  stack_unlink(res, it);
  return it;
}

istack_t *
stack_rpush(stack_t *res, int64_t value)
{
  istack_t *it = (istack_t *)malloc(sizeof(*it));
  if(it == NULL) {
      return NULL;
  }
  memset(it, 0, sizeof(istack_t));

  it->value = value;

  stack_link(res, res->end, it);
  return it;
}

istack_t *
stack_lpop(stack_t *res)
{
  istack_t *it = res->begin;
  stack_unlink(res, it);
  return it;
}

istack_t *
stack_lpush(stack_t *res, int64_t value)
{
  istack_t *it = (istack_t *)malloc(sizeof(*it));
  if(it == NULL) {
    return NULL;
  }
  memset(it, 0, sizeof(istack_t));

  it->value = value;

  stack_link(res, res->begin, it);
  return it;
}

istack_t *
stack_insert(stack_t *res, istack_t *current, int64_t value)
{
  istack_t *it = (istack_t *)malloc(sizeof(*it));
  if(it == NULL) {
      return NULL;
  }
  memset(it, 0, sizeof(istack_t));

  it->value = value;

  stack_link(res, current, it);
  return it;
}


istack_t *
stack_first(stack_t *res)
{
  if(res->begin != 0)
    return res->begin;
  return NULL;
}

istack_t *
stack_last(stack_t *res)
{
  if(res->end->previous != 0 && res->end->previous != res->end)
    return res->end->previous;
  return NULL;
}
