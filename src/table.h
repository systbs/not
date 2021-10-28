#pragma once

typedef struct itable {
    struct itable *previous;
    struct itable *next;
    value_p value;
} itable_t;

typedef struct table {
    itable_t *begin;
    itable_t *end;
} table_t;


table_t *
table_apply(table_t *tbl);

table_t *
table_create();

value_t
table_isempty(table_t *tbl);

value_p
table_content(itable_t *current);

itable_t*
table_next(itable_t *current);

itable_t*
table_previous(itable_t *current);

value_t
table_count(table_t *tbl);

value_t
table_query(table_t *tbl, value_t (*f)(itable_t*));

void
table_destroy(table_t *tbl);

itable_t*
table_link(table_t *tbl, itable_t *current, itable_t *it);

itable_t*
table_unlink(table_t *tbl, itable_t* it);

itable_t*
table_sort(table_t *tbl, value_t (*f)(value_p, value_p));

itable_t*
table_remove(table_t *tbl, value_t (*f)(value_p));

itable_t*
table_clear(table_t *tbl);

itable_t*
table_rpop(table_t *tbl);

itable_t *
table_rpush(table_t *tbl, value_p value);

itable_t*
table_lpop(table_t *tbl);

itable_t *
table_lpush(table_t *tbl, value_p value);

itable_t *
table_insert(table_t *tbl, itable_t *current, value_p value);

value_t
table_null(table_t *tbl);

itable_t *
table_at(table_t *tbl, value_t key);

itable_t *
table_first(table_t *tbl);

itable_t *
table_last(table_t *tbl);

itable_t *
table_first_or_default(table_t *tbl, value_t (*f)(value_p));

itable_t *
table_last_or_default(table_t *tbl, value_t (*f)(value_p));

value_p
table_aggregate(table_t *tbl, value_p(*f)(value_p, value_p));
