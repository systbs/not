#pragma once

typedef struct itable {
    struct itable *previous;
    struct itable *next;
    tbval_t value;
} itable_t;

typedef struct table {
    itable_t *begin;
    itable_t *end;
} table_t;


table_t *
table_apply(table_t *tbl);

table_t *
table_create();

arval_t
table_isempty(table_t *tbl);

tbval_t
table_content(itable_t *current);

itable_t*
table_next(itable_t *current);

itable_t*
table_previous(itable_t *current);

arval_t
table_count(table_t *tbl);

arval_t
table_query(table_t *tbl, arval_t (*f)(itable_t*));

void
table_destroy(table_t *tbl);

itable_t*
table_link(table_t *tbl, itable_t *current, itable_t *it);

itable_t*
table_unlink(table_t *tbl, itable_t* it);

itable_t*
table_sort(table_t *tbl, arval_t (*f)(tbval_t, tbval_t));

itable_t*
table_remove(table_t *tbl, arval_t (*f)(tbval_t));

itable_t*
table_clear(table_t *tbl);

itable_t*
table_rpop(table_t *tbl);

itable_t *
table_rpush(table_t *tbl, tbval_t value);

itable_t*
table_lpop(table_t *tbl);

itable_t *
table_lpush(table_t *tbl, tbval_t value);

itable_t *
table_insert(table_t *tbl, itable_t *current, tbval_t value);

arval_t
table_null(table_t *tbl);

itable_t *
table_at(table_t *tbl, arval_t key);

itable_t *
table_first(table_t *tbl);

itable_t *
table_last(table_t *tbl);

itable_t *
table_first_or_default(table_t *tbl, arval_t (*f)(tbval_t));

itable_t *
table_last_or_default(table_t *tbl, arval_t (*f)(tbval_t));

tbval_t
table_aggregate(table_t *tbl, tbval_t(*f)(tbval_t, tbval_t));
