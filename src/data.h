#pragma once

arval_t
data_sizeof(table_t *tbl);

table_t *
data_clone(table_t *tbl);

table_t *
data_from(char *str);

table_t *
data_format(table_t *tbl, table_t *format);

arval_t
data_compare(table_t *tbl1, table_t *tbl2);

char *
data_to(table_t *tbl);

void
data_delete(table_t *tbl);