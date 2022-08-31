#pragma once
typedef char_t* map_key_t;
typedef long_t* map_value_t;

typedef struct imap {
    struct imap *previous;
    struct imap *next;
    map_key_t key;
    map_value_t value;
} imap_t;

typedef struct map {
    imap_t *begin;
    imap_t *end;
} map_t;


map_t *
map_apply(map_t *map);

map_t *
map_create();

long_t
map_isempty(map_t *map);

map_value_t
map_content(imap_t *current);

imap_t*
map_next(imap_t *current);

imap_t*
map_previous(imap_t *current);

long_t
map_count(map_t *map);

long_t
map_query(map_t *map, long_t (*f)(imap_t*));

void
map_destroy(map_t *map);

imap_t*
map_link(map_t *map, imap_t *current, imap_t *it);

imap_t*
map_unlink(map_t *map, imap_t* it);

imap_t*
map_sort(map_t *map, long_t (*f)(imap_t*, imap_t*));

imap_t*
map_remove(map_t *map, long_t (*f)(imap_t*));

map_t*
map_clear(map_t *map);

imap_t *
map_insert(map_t *map, imap_t *current, map_key_t key, map_value_t value);

long_t
map_isempty(map_t *map);

imap_t *
map_at(map_t *map, map_key_t key);

imap_t *
map_first(map_t *map);

imap_t *
map_last(map_t *map);

