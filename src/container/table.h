#pragma once
#ifndef __RESPONSE_H__
#define __RESPONSE_H__

typedef struct itable {
    struct itable *previous;
    struct itable *next;
    
    symbol_t *key;
    symbol_t *type;
    symbol_t *value;
    symbol_t *default_value;

    uint64_t reference;
} itable_t;

typedef struct table {
  struct table *parent;

  itable_t *begin;
  itable_t *end;
} table_t;

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

int
table_query(table_t *res, int (*f)(itable_t*));

void
table_destroy(table_t *res);

itable_t*
table_link(table_t *res, itable_t *current, itable_t *it);

itable_t*
table_unlink(table_t *res, itable_t* it);

int
table_sort(table_t *res, int (*f)(itable_t *, itable_t *));

void
table_clear(table_t *res);

itable_t *
table_new();

itable_t*
table_rpop(table_t *res);

itable_t*
table_lpop(table_t *res);

itable_t *
table_first(table_t *res);

itable_t *
table_last(table_t *res);

#endif