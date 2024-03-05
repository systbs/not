#pragma once
#ifndef __RESPONSE_H__
#define __RESPONSE_H__

typedef struct itable {
    struct itable *previous;
    struct itable *next;
    
    node_t *original;
    node_t *scope;
    node_t *value;

    uint64_t reference;
    uint64_t flag;
} itable_t;

typedef struct table {
  struct table *parent;

  itable_t *begin;
  itable_t *end;
} table_t;

typedef enum itable_flag
{
	ITABLE_FLAG_NONE 	      = 1 << 0,
  ITABLE_FLAG_INITALIZE 	= 1 << 1
} itable_flag_t;

table_t *
table_apply(table_t *res);

table_t *
table_create();

int
table_isempty(table_t *res);

itable_t*
table_next(itable_t *current);

itable_t*
table_previous(itable_t *current);

uint64_t
table_count(table_t *res);

int32_t
table_query(table_t *res, int32_t (*f)(itable_t*));

void
table_destroy(table_t *res);

void
table_link(table_t *res, itable_t *current, itable_t *it);

void
table_unlink(table_t *res, itable_t* it);

void
table_sort(table_t *res, int32_t (*f)(itable_t *, itable_t *));

void
table_clear(table_t *res);

itable_t *
table_new();

itable_t*
table_rpop(table_t *res);

itable_t*
table_lpop(table_t *res);

itable_t *
table_rpush(table_t *res, node_t *original);

itable_t *
table_lpush(table_t *res, node_t *original);

itable_t *
table_first(table_t *res);

itable_t *
table_last(table_t *res);

#endif