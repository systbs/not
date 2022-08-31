#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "map.h"
#include "memory.h"

map_t *
map_apply(map_t *map)
{
    imap_t *it;

    if(!(it = (imap_t *)qalam_malloc(sizeof(imap_t)))) {
        return 0;
    }

    it->next = it->previous = it;
    map->end = map->begin = it;

    return map;
}

map_t *
map_create()
{
    map_t *map;

    if(!(map = (map_t *)qalam_malloc(sizeof(*map)))) {
        return 0;
    }

    return map_apply(map);
}

map_value_t
map_content(imap_t *current){
    return current ? current->value : 0;
}

imap_t*
map_next(imap_t *current)
{
    return current->next;
}

imap_t*
map_previous(imap_t *current)
{
    return current->previous;
}

long_t
map_count(map_t *map)
{
    long_t cnt = 0;
    imap_t *b;
    for(b = map->begin; b && (b != map->end); b = b->next){
        cnt++;
    }
    return cnt;
}

long_t
map_query(map_t *map, long_t (*f)(imap_t*))
{
    if (map_isempty(map))
        return 0;

    imap_t *b, *n;
    for(b = map->begin; b && (b != map->end); b = n){
        n = b->next;
        if(!(*f)(b)){
          return 0;
        }
    }

    return 1;
}

void
map_destroy(map_t *map)
{
    map_clear(map);
    qalam_free (map);
}

imap_t*
map_link(map_t *map, imap_t *current, imap_t *it)
{
    it->next = current;
    it->previous = current->previous;
    current->previous->next = it;
    current->previous = it;

    if(map->begin == current){
        map->begin = it;
    }

    return it;
}

imap_t*
map_unlink(map_t *map, imap_t* it)
{
    if (it == map->end){
        return 0;
    }

    if (it == map->begin){
        map->begin = it->next;
    }

    it->next->previous = it->previous;
    it->previous->next = it->next;

    return it;
}

imap_t*
map_sort(map_t *map, long_t (*f)(imap_t*, imap_t*))
{
    imap_t *b, *n;
    for(b = map->begin; b != map->end; b = n){
        n = b->next;
        if(n != map->end){
            if((*f)(b, n)){
                map_unlink(map, b);
                map_link(map, n, b);
            }
        }
    }
    return 0;
}

imap_t*
map_remove(map_t *map, long_t (*f)(imap_t*))
{
    imap_t *b, *n;
    for(b = map->begin; b != map->end; b = n){
        n = b->next;
        if((*f)(b)){
            return map_unlink(map, b);
        }
    }
    return 0;
}

map_t*
map_clear(map_t *map)
{
    imap_t *b, *n;
    for(b = map->begin; b != map->end; b = n){
        n = b->next;
        map_unlink(map, b);
        qalam_free(b->key);
        qalam_free(b);
    }
    return map;
}

imap_t *
map_insert(map_t *map, imap_t *current, map_key_t key, map_value_t value){
	imap_t *b, *n;
    for(b = map->begin; b != map->end; b = n){
        n = b->next;
        if(strcmp(b->key, key)==0){
        	b->value = value;
        	return b;
        }
    }

	imap_t *it;
    if(!(it = (imap_t *)qalam_malloc(sizeof(*it)))) {
        return 0;
    }
    
	if(!(it->key = (map_key_t)qalam_malloc(strlen(key)))) {
		qalam_free(it);
		return 0;
    }
    
	strcpy(it->key, key);
	
    it->value = value;

    return map_link(map, current, it);
}

long_t
map_isempty(map_t *map)
{
    if(map == 0){
        return 0;
    }
    return (map->begin == map->end);
}

imap_t *
map_at(map_t *map, map_key_t key)
{
    imap_t *b, *n;
    for(b = map->begin; b && (b != map->end); b = n){
        n = b->next;
        if (strcmp(key, b->key) == 0){
            return b;
        }
    }

    return 0;
}

imap_t *
map_first(map_t *map)
{
    if(map->begin != 0)
        return map->begin;
    return 0;
}

imap_t *
map_last(map_t *map)
{
    if(map->end->previous != 0 && map->end->previous != map->end)
        return map->end->previous;
    return 0;
}

