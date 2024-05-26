#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../parser/syntax/syntax.h"
#include "../error.h"
#include "../memory.h"
#include "../mutex.h"
#include "record.h"
#include "garbage.h"

sy_garbage_t base_garbage;

static sy_garbage_t *
sy_garbage_get()
{
  return &base_garbage;
}

int32_t
sy_garbage_init()
{
	sy_garbage_t *garbage = sy_garbage_get();
	if (sy_mutex_init(&garbage->lock) < 0)
	{
		sy_error_system("'%s' could not initialize the lock", "sy_garbage.lock");
		return -1;
	}

    garbage->begin = NULL;

	return 0;
}

int32_t
sy_garbage_destroy()
{
    sy_garbage_t *garbage = sy_garbage_get();
    if (sy_mutex_destroy(&garbage->lock) < 0)
    {
        sy_error_system("'%s' could not destroy the lock", "sy_garbage.lock");
        return -1;
    }
    return 0;
}

static void
sy_garbage_unsafe_link(sy_garbage_entry_t *current, sy_garbage_entry_t *entry)
{
	sy_garbage_t *garbage = sy_garbage_get();
    entry->next = current;
    if (current)
    {
        entry->previous = current->previous;
        current->previous = entry;
        if (current->previous)
        {
            current->previous->next = entry;
        }
    }
    
    if(garbage->begin == current)
    {
        garbage->begin = entry;
    }
}

static void
sy_garbage_unsafe_unlink(sy_garbage_entry_t *entry)
{
	sy_garbage_t *garbage = sy_garbage_get();
	if (entry == garbage->begin)
	{
		garbage->begin = entry->next;
	}

	if (entry->next)
	{
		entry->next->previous = entry->previous;
	}

	if (entry->previous)
	{
		entry->previous->next = entry->next;
	}
}

sy_garbage_entry_t *
sy_garbage_push(sy_record_t *value)
{
    sy_garbage_entry_t *entry = (sy_garbage_entry_t *)sy_memory_calloc(1, sizeof(sy_garbage_entry_t));
    if(!entry)
    {
		sy_error_no_memory();
        return ERROR;
    }

	entry->value = value;

	sy_garbage_t *garbage = sy_garbage_get();
    
	if (sy_mutex_lock(&garbage->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not lock", "sy_garbage.lock");
        return ERROR;
    }

	sy_garbage_unsafe_link(garbage->begin, entry);

	if (sy_mutex_unlock(&garbage->lock) < 0)
    {
        sy_memory_free(entry);
        sy_error_system("'%s' could not unlock", "sy_garbage.lock");
        return ERROR;
    }

    return entry;
}
