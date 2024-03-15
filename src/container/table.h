#pragma once
#ifndef __TABLE_H__
#define __TABLE_H__

typedef struct itable {
    struct itable *previous;
    struct itable *next;
    void *value;
    void *update;
} itable_t;

typedef struct table {
    itable_t *begin;
    itable_t *end;

    struct table *parent;
} table_t;

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
table_query(table_t *res, int (*f)(itable_t*));

void
table_destroy(table_t *res);

void
table_link(table_t *res, itable_t *current, itable_t *it);

void
table_unlink(table_t *res, itable_t* it);

void
table_sort(table_t *res, int (*f)(itable_t *, itable_t *));

void
table_unlink_by(table_t *res, int (*f)(itable_t *));

void
table_clear(table_t *res);

itable_t*
table_rpop(table_t *res);

itable_t *
table_rpush(table_t *res, void *value);

itable_t*
table_lpop(table_t *res);

itable_t *
table_lpush(table_t *res, void *value);

itable_t *
table_first(table_t *res);

itable_t *
table_last(table_t *res);

#endif