#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <gmp.h>
#include <jansson.h>

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

not_garbage_t base_garbage;

static not_garbage_t *
not_garbage_get()
{
    return &base_garbage;
}

int32_t
not_garbage_init()
{
    not_garbage_t *garbage = not_garbage_get();
    if (not_mutex_init(&garbage->lock) < 0)
    {
        not_error_system("'%s' could not initialize the lock", "not_garbage.lock");
        return -1;
    }

    garbage->begin = NOT_PTR_NULL;

    return 0;
}

int32_t
not_garbage_destroy()
{
    not_garbage_t *garbage = not_garbage_get();
    if (not_mutex_destroy(&garbage->lock) < 0)
    {
        not_error_system("'%s' could not destroy the lock", "not_garbage.lock");
        return -1;
    }
    return 0;
}

static void
not_garbage_unsafe_link(not_garbage_entry_t *entry)
{
    not_garbage_t *garbage = not_garbage_get();
    entry->next = garbage->begin;
    if (garbage->begin)
    {
        garbage->begin->previous = entry;
    }
    entry->previous = NOT_PTR_NULL;
    garbage->begin = entry;
}

static void
not_garbage_unsafe_unlink(not_garbage_entry_t *entry)
{
    not_garbage_t *garbage = not_garbage_get();
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
not_garbage_push(not_record_t *value)
{
    not_garbage_entry_t *entry = (not_garbage_entry_t *)not_memory_calloc(1, sizeof(not_garbage_entry_t));
    if (!entry)
    {
        not_error_no_memory();
        return -1;
    }

    entry->value = value;

    not_garbage_t *garbage = not_garbage_get();

    if (not_mutex_lock(&garbage->lock) < 0)
    {
        not_error_system("'%s' could not lock", "not_garbage.lock");
        not_memory_free(entry);
        return -1;
    }

    not_garbage_unsafe_link(entry);

    if (not_mutex_unlock(&garbage->lock) < 0)
    {
        not_error_system("'%s' could not unlock", "not_garbage.lock");
        not_memory_free(entry);
        return -1;
    }

    return 0;
}

int32_t
not_garbage_clean()
{
    not_garbage_t *garbage = not_garbage_get();

    if (not_mutex_lock(&garbage->lock) < 0)
    {
        not_error_system("'%s' could not lock", "not_garbage.lock");
        return -1;
    }

    for (not_garbage_entry_t *item = garbage->begin, *next = NOT_PTR_NULL; item != NOT_PTR_NULL; item = next)
    {
        next = item->next;

        if (item->value)
        {
            not_record_t *record = item->value;
            if (record->link <= 0)
            {
                if (not_record_destroy(record) < 0)
                {
                    if (not_mutex_unlock(&garbage->lock) < 0)
                    {
                        not_error_system("'%s' could not unlock", "not_garbage.lock");
                        return -1;
                    }
                    return -1;
                }
                not_garbage_unsafe_unlink(item);
                not_memory_free(item);
            }
        }
        else
        {
            not_garbage_unsafe_unlink(item);
            not_memory_free(item);
        }
    }

    if (not_mutex_unlock(&garbage->lock) < 0)
    {
        not_error_system("'%s' could not unlock", "not_garbage.lock");
        return -1;
    }

    return 0;
}

#if _WIN32
DWORD WINAPI
not_garbage_clean_by_thread(LPVOID arg)
#else
void *
not_garbage_clean_by_thread(void *arg)
#endif
{
    not_grabage_thread_data_t *data = (not_grabage_thread_data_t *)arg;
    data->ret = -1;

    while (1)
    {
        not_thread_sleep(500);

        if (not_garbage_clean() < 0)
        {
            goto region_error;
        }
    }

    data->ret = 0;
    if (not_thread_exit() < 0)
    {
        goto region_error;
    }
    return data;

region_error:
    data->ret = -1;
    if (not_thread_exit() < 0)
    {
        goto region_error;
    }
    return data;
}
