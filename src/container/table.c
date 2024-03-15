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
#include "table.h"

table_t *
table_apply(table_t *res)
{
  itable_t *it = (itable_t *)malloc(sizeof(itable_t));
  if(it == NULL) {
    return NULL;
  }

  memset(it, 0, sizeof(itable_t));

  it->next = it->previous = it;
  res->end = res->begin = it;

  return res;
}

table_t *
table_create()
{
  table_t *res = (table_t *)malloc(sizeof(*res));
  if(!res) {
    return NULL;
  }
  memset(res, 0, sizeof(table_t));

  return table_apply(res);
}

int32_t
table_isempty(table_t *res)
{
  return (res->begin == res->end);
}

itable_t*
table_next(itable_t *current)
{
  return current->next;
}

itable_t*
table_previous(itable_t *current)
{
  return current->previous;
}

uint64_t
table_count(table_t *res)
{
  uint64_t cnt = 0;
  itable_t *b;
  for(b = res->begin; b && (b != res->end); b = b->next){
    cnt++;
  }
  return cnt;
}

int32_t
table_query(table_t *res, int (*f)(itable_t*))
{
  if (table_isempty(res))
  {
    return 0;
  }

  itable_t *b, *n;
  for(b = res->begin; b && (b != res->end); b = n){
    n = b->next;
    if(!(*f)(b)){
      return 0;
    }
  }

  return 1;
}

void
table_destroy(table_t *res)
{
  table_clear(res);
  free (res);
}

void
table_link(table_t *res, itable_t *current, itable_t *it)
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
table_unlink(table_t *res, itable_t* it)
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
table_clear(table_t *res)
{
  itable_t *b, *n;
  for(b = res->begin; b != res->end; b = n){
    n = b->next;
    table_unlink(res, b);
    free(b);
  }
}

itable_t *
table_rpop(table_t *res)
{
	if(table_isempty(res)){
		return NULL;
	}
  itable_t *it = res->end->previous;
  table_unlink(res, it);
  return it;
}

itable_t *
table_rpush(table_t *res, void *value)
{
  itable_t *it = (itable_t *)malloc(sizeof(*it));
  if(it == NULL) {
      return NULL;
  }
  memset(it, 0, sizeof(itable_t));

  it->value = value;

  table_link(res, res->end, it);
  return it;
}

itable_t *
table_lpop(table_t *res)
{
  itable_t *it = res->begin;
  table_unlink(res, it);
  return it;
}

itable_t *
table_lpush(table_t *res, void *value)
{
  itable_t *it = (itable_t *)malloc(sizeof(*it));
  if(it == NULL) {
    return NULL;
  }
  memset(it, 0, sizeof(itable_t));

  it->value = value;

  table_link(res, res->begin, it);
  return it;
}

itable_t *
table_first(table_t *res)
{
  if(res->begin != 0)
    return res->begin;
  return NULL;
}

itable_t *
table_last(table_t *res)
{
  if(res->end->previous != 0 && res->end->previous != res->end)
    return res->end->previous;
  return NULL;
}
