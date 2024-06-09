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
#include "strip.h"
#include "garbage.h"

#include "../interpreter.h"
#include "../thread.h"


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
sy_garbage_unsafe_link(sy_garbage_entry_t *entry)
{
	sy_garbage_t *garbage = sy_garbage_get();
    entry->next = garbage->begin;
    if (garbage->begin)
    {
        garbage->begin->previous = entry;
    }
    entry->previous = NULL;
    garbage->begin = entry;
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

int32_t
sy_garbage_push(sy_record_t *value)
{
    sy_garbage_entry_t *entry = (sy_garbage_entry_t *)sy_memory_calloc(1, sizeof(sy_garbage_entry_t));
    if(!entry)
    {
		sy_error_no_memory();
        return -1;
    }

	entry->value = value;

	sy_garbage_t *garbage = sy_garbage_get();
    
	if (sy_mutex_lock(&garbage->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_garbage.lock");
        sy_memory_free(entry);
        return -1;
    }

	sy_garbage_unsafe_link(entry);

	if (sy_mutex_unlock(&garbage->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_garbage.lock");
        sy_memory_free(entry);
        return -1;
    }

    return 0;
}

int32_t
sy_garbage_clean()
{
    sy_garbage_t *garbage = sy_garbage_get();
    
	if (sy_mutex_lock(&garbage->lock) < 0)
    {
        sy_error_system("'%s' could not lock", "sy_garbage.lock");
        return -1;
    }

    for (sy_garbage_entry_t *item = garbage->begin, *next = NULL;item != NULL; item = next)
    {
        next = item->next;

        if (item->value)
        {
            sy_record_t *record = item->value;
            if (record->link <= 0)
            {
                if (sy_record_destroy(record) < 0)
                {
                    if (sy_mutex_unlock(&garbage->lock) < 0)
                    {
                        sy_error_system("'%s' could not unlock", "sy_garbage.lock");
                        return -1;
                    }
                    return -1;
                }
                sy_garbage_unsafe_unlink(item);
                sy_memory_free(item);
            }
        }
        else
        {
            sy_garbage_unsafe_unlink(item);
            sy_memory_free(item);
        }
    }

    if (sy_mutex_unlock(&garbage->lock) < 0)
    {
        sy_error_system("'%s' could not unlock", "sy_garbage.lock");
        return -1;
    }

    return 0;
}

#if _WIN32
DWORD WINAPI 
sy_garbage_clean_by_thread(LPVOID arg)
#else
void * 
sy_garbage_clean_by_thread(void *arg)
#endif
{
    sy_grabage_thread_data_t *data = (sy_grabage_thread_data_t*)arg;
    data->ret = -1;

    while (1)
    {
        sy_thread_sleep(500);

        if (sy_garbage_clean() < 0)
        {
            goto region_error;
        }
    }

    data->ret = 0;
    if (sy_thread_exit() < 0)
    {
        goto region_error;
    }
    return data;

    region_error:
    data->ret = -1;
    if (sy_thread_exit() < 0)
    {
        goto region_error;
    }
    return data;
}

