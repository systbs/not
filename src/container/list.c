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
#include "../parser/symbol.h"
#include "list.h"

list_t *
list_apply(list_t *res)
{
  ilist_t *it = (ilist_t *)malloc(sizeof(ilist_t));
  if(it == NULL) {
    return NULL;
  }

  memset(it, 0, sizeof(ilist_t));

  it->next = it->previous = it;
  res->end = res->begin = it;

  return res;
}

list_t *
list_create()
{
  list_t *res = (list_t *)malloc(sizeof(*res));
  if(!res) {
    return NULL;
  }
  memset(res, 0, sizeof(list_t));

  return list_apply(res);
}

int32_t
list_isempty(list_t *res)
{
  return (res->begin == res->end);
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
list_count(list_t *res)
{
  uint64_t cnt = 0;
  ilist_t *b;
  for(b = res->begin; b && (b != res->end); b = b->next){
    cnt++;
  }
  return cnt;
}

int32_t
list_query(list_t *res, int (*f)(ilist_t*))
{
  if (list_isempty(res))
  {
    return 0;
  }

  ilist_t *b, *n;
  for(b = res->begin; b && (b != res->end); b = n){
    n = b->next;
    if(!(*f)(b)){
      return 0;
    }
  }

  return 1;
}

void
list_destroy(list_t *res)
{
  list_clear(res);
  free (res);
}

void
list_link(list_t *res, ilist_t *current, ilist_t *it)
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
list_unlink(list_t *res, ilist_t* it)
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
list_sort(list_t *res, int (*f)(ilist_t *, ilist_t *))
{
  ilist_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    if(n != res->end){
      if((*f)(b, n)){
        list_unlink(res, b);
        list_link(res, n, b);
      }
    }
  }
}

void
list_unlink_by(list_t *res, int (*f)(ilist_t *))
{
  ilist_t *b, *n;
  for(b = res->begin; b != res->end; b = n) {
    n = b->next;
    if ((*f)(b)) {
      list_unlink(res, b);
      break;
    }
  }
}

void
list_clear(list_t *res)
{
  ilist_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    list_unlink(res, b);
    free(b);
  }
}

ilist_t *
list_rpop(list_t *res)
{
	if(list_isempty(res)){
		return NULL;
	}
  ilist_t *it = res->end->previous;
  list_unlink(res, it);
  return it;
}

ilist_t *
list_rpush(list_t *res, void *value)
{
  ilist_t *it = (ilist_t *)malloc(sizeof(*it));
  if(it == NULL) {
      return NULL;
  }
  memset(it, 0, sizeof(ilist_t));

  it->value = value;

  list_link(res, res->end, it);
  return it;
}

ilist_t *
list_lpop(list_t *res)
{
  ilist_t *it = res->begin;
  list_unlink(res, it);
  return it;
}

ilist_t *
list_lpush(list_t *res, void *value)
{
  ilist_t *it = (ilist_t *)malloc(sizeof(*it));
  if(it == NULL) {
    return NULL;
  }
  memset(it, 0, sizeof(ilist_t));

  it->value = value;

  list_link(res, res->begin, it);
  return it;
}

ilist_t *
list_first(list_t *res)
{
  if(res->begin != 0)
    return res->begin;
  return NULL;
}

ilist_t *
list_last(list_t *res)
{
  if(res->end->previous != 0 && res->end->previous != res->end)
    return res->end->previous;
  return NULL;
}
