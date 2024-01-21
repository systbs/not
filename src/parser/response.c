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
#include "symbol.h"
#include "response.h"

response_t *
response_apply(response_t *res)
{
  iresponse_t *it;

  if(!(it = (iresponse_t *)malloc(sizeof(iresponse_t)))) {
    return NULL;
  }
  memset(it, 0, sizeof(iresponse_t));

  it->next = it->previous = it;
  res->end = res->begin = it;

  return res;
}

response_t *
response_create()
{
  response_t *res;
  if(!(res = (response_t *)malloc(sizeof(*res)))) {
    return NULL;
  }
  memset(res, 0, sizeof(response_t));

  return response_apply(res);
}

int
response_isempty(response_t *res)
{
  return (res->begin == res->end);
}

iresponse_t*
response_next(iresponse_t *current)
{
  return current->next;
}

iresponse_t*
response_previous(iresponse_t *current)
{
  return current->previous;
}

uint64_t
response_count(response_t *res)
{
  uint64_t cnt = 0;
  iresponse_t *b;
  for(b = res->begin; b && (b != res->end); b = b->next){
    cnt++;
  }
  return cnt;
}

int
response_query(response_t *res, int (*f)(iresponse_t*))
{
  if (response_isempty(res))
    return 0;

  iresponse_t *b, *n;
  for(b = res->begin; b && (b != res->end); b = n){
    n = b->next;
    if(!(*f)(b)){
      return 0;
    }
  }

  return 1;
}

void
response_destroy(response_t *res)
{
  response_clear(res);
  free (res);
}

iresponse_t*
response_link(response_t *res, iresponse_t *current, iresponse_t *it)
{
  it->next = current;
  it->previous = current->previous;
  current->previous->next = it;
  current->previous = it;

  if(res->begin == current){
      res->begin = it;
  }

  return it;
}

iresponse_t*
response_unlink(response_t *res, iresponse_t* it)
{
  if (it == res->end) {
    return NULL;
  }

  if (it == res->begin) {
    res->begin = it->next;
  }

  it->next->previous = it->previous;
  it->previous->next = it->next;

  return it;
}

int
response_sort(response_t *res, int (*f)(iresponse_t *, iresponse_t *))
{
  iresponse_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    if(n != res->end){
      if((*f)(b, n)){
        iresponse_t *result;
        result = response_unlink(res, b);
        if (!result)
        {
          return 0;
        }
        result = response_link(res, n, b);
        if (!result)
        {
          return 0;
        }
      }
    }
  }
  return 1;
}

iresponse_t *
response_unlink_by(response_t *res, int (*f)(iresponse_t *))
{
  iresponse_t *b, *n;
  for(b = res->begin; b != res->end; b = n) {
    n = b->next;
    if ((*f)(b)) {
      return response_unlink(res, b);
    }
  }
  return NULL;
}

void
response_clear(response_t *res)
{
  iresponse_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    response_unlink(res, b);
    free(b);
  }
}

iresponse_t*
response_rpop(response_t *res)
{
	if(response_isempty(res)){
		return NULL;
	}
  return response_unlink(res, res->end->previous);
}

iresponse_t *
response_rpush(response_t *res, uint64_t type, void *value)
{
  iresponse_t *it;
  if(!(it = (iresponse_t *)malloc(sizeof(*it)))) {
      return NULL;
  }
  memset(it, 0, sizeof(iresponse_t));

  it->type = type;
  it->value = value;

  return response_link(res, res->end, it);
}

iresponse_t *
response_lpop(response_t *res)
{
  return response_unlink(res, res->begin);
}

iresponse_t *
response_lpush(response_t *res, uint64_t type, void *value)
{
  iresponse_t *it;
  if(!(it = (iresponse_t *)malloc(sizeof(*it)))) {
      return NULL;
  }
  memset(it, 0, sizeof(iresponse_t));

  it->type = type;
  it->value = value;

  return response_link(res, res->begin, it);
}

iresponse_t *
response_first(response_t *res)
{
  if(res->begin != 0)
    return res->begin;
  return NULL;
}

iresponse_t *
response_last(response_t *res)
{
  if(res->end->previous != 0 && res->end->previous != res->end)
    return res->end->previous;
  return NULL;
}
