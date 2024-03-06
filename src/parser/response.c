#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "response.h"

response_t *
response_apply(response_t *res)
{
  iresponse_t *it = (iresponse_t *)malloc(sizeof(iresponse_t));
  if(it == NULL) {
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
  response_t *res = (response_t *)malloc(sizeof(*res));
  if(!res) {
    return NULL;
  }
  memset(res, 0, sizeof(response_t));

  return response_apply(res);
}

int32_t
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

int32_t
response_query(response_t *res, int (*f)(iresponse_t*))
{
  if (response_isempty(res))
  {
    return 0;
  }

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

void
response_link(response_t *res, iresponse_t *current, iresponse_t *it)
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
response_unlink(response_t *res, iresponse_t* it)
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
response_sort(response_t *res, int (*f)(iresponse_t *, iresponse_t *))
{
  iresponse_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    if(n != res->end){
      if((*f)(b, n)){
        response_unlink(res, b);
        response_link(res, n, b);
      }
    }
  }
}

void
response_unlink_by(response_t *res, int (*f)(iresponse_t *))
{
  iresponse_t *b, *n;
  for(b = res->begin; b != res->end; b = n) {
    n = b->next;
    if ((*f)(b)) {
      response_unlink(res, b);
      break;
    }
  }
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

iresponse_t *
response_rpop(response_t *res)
{
	if(response_isempty(res)){
		return NULL;
	}
  iresponse_t *it = res->end->previous;
  response_unlink(res, it);
  return it;
}

iresponse_t *
response_rpush(response_t *res, void *value)
{
  iresponse_t *it = (iresponse_t *)malloc(sizeof(*it));
  if(it == NULL) {
      return NULL;
  }
  memset(it, 0, sizeof(iresponse_t));

  it->value = value;

  response_link(res, res->end, it);
  return it;
}

iresponse_t *
response_lpop(response_t *res)
{
  iresponse_t *it = res->begin;
  response_unlink(res, it);
  return it;
}

iresponse_t *
response_lpush(response_t *res, void *value)
{
  iresponse_t *it = (iresponse_t *)malloc(sizeof(*it));
  if(it == NULL) {
    return NULL;
  }
  memset(it, 0, sizeof(iresponse_t));

  it->value = value;

  response_link(res, res->begin, it);
  return it;
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
