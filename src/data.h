#pragma once

long_t
data_sizeof(list_t *lst);

list_t *
data_clone(list_t *lst);

list_t *
data_from(char *str);

list_t *
data_format(list_t *lst, list_t *format);

long_t
data_compare(list_t *lst1, list_t *lst2);

char *
data_to(list_t *lst);

void
data_delete(list_t *lst);
