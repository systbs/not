#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "types/types.h"
#include "utils/path.h"
#include "token/position.h"
#include "container/queue.h"
#include "error.h"
#include "mutex.h"
#include "memory.h"
#include "interpreter.h"
#include "thread.h"

not_thread_t base_thread;

int32_t
not_thread_init()
{
    not_thread_t *bt = not_thread_get();

#if defined(_WIN32) || defined(_WIN64)
    bt->id = GetCurrentThreadId();
    bt->thread = = GetCurrentThread();
#else
    bt->id = pthread_self();
#endif

    bt->parent = NULL;

    bt->interpreter = not_interpreter_create();
    if (bt->interpreter == ERROR)
    {
        return -1;
    }

    bt->childrens = not_queue_create();
    if (bt->childrens == ERROR)
    {
        not_interpreter_destroy(bt->interpreter);
        return -1;
    }

    return 0;
}

void not_thread_destroy()
{
    not_thread_t *thread = not_thread_get();
    not_interpreter_destroy(thread->interpreter);
}

not_thread_t *
not_thread_get()
{
    return &base_thread;
}

not_thread_t *
not_thread_create(
#if defined(_WIN32) || defined(_WIN64)
    DWORD (*start_routine)(LPVOID), LPVOID arg
#else
    void *(*start_routine)(void *), void *arg
#endif
)
{
    not_thread_t *t = (not_thread_t *)not_memory_calloc(1, sizeof(not_thread_t));
    if (!t)
    {
        not_error_no_memory();
        return ERROR;
    }

    t->interpreter = not_interpreter_create();
    if (t->interpreter == ERROR)
    {
        not_memory_free(t);
        return ERROR;
    }

    t->childrens = not_queue_create();
    if (t->childrens == ERROR)
    {
        not_interpreter_destroy(t->interpreter);
        not_memory_free(t);
        return ERROR;
    }

    not_thread_t *parent = not_thread_get_current();
    if (parent == ERROR)
    {
        not_interpreter_destroy(t->interpreter);
        not_memory_free(t);
        return ERROR;
    }

    if (ERROR == not_queue_right_push(parent->childrens, t))
    {
        not_interpreter_destroy(t->interpreter);
        not_memory_free(t);
        return ERROR;
    }

#if defined(_WIN32) || defined(_WIN64)
    DWORD threadId;
    HANDLE thread = CreateThread(NULL, 0, not_repository_load_by_thread, &data, 0, &threadId);
    if (!thread)
    {
        not_interpreter_destroy(t->interpreter);
        not_memory_free(t);
        not_error_system("new thread not created\n");
        return ERROR;
    }

    t->id = threadId;
    t->thread = = thread;
#else
    pthread_t thread;
    if (pthread_create(&thread, NULL, start_routine, arg) != 0)
    {
        not_interpreter_destroy(t->interpreter);
        not_memory_free(t);
        not_error_system("new thread not created\n");
        return ERROR;
    }
    t->id = thread;
#endif

    return t;
}

static not_thread_t *
not_thread_find_by_id(not_thread_t *parent, not_thread_id_t id)
{
    not_queue_entry_t *item;
    for (item = parent->childrens->begin; item != parent->childrens->end; item = item->next)
    {
        not_thread_t *t = (not_thread_t *)item->value;

        if (t->id == id)
        {
            return t;
        }

        not_thread_t *r1 = not_thread_find_by_id(t, id);
        if (r1 != NULL)
        {
            return r1;
        }
    }

    return NULL;
}

not_thread_t *
not_thread_get_current()
{
#if defined(_WIN32) || defined(_WIN64)
    DWORD id = GetCurrentThreadId();
#else
    pthread_t id = pthread_self();
#endif

    not_thread_t *bt = not_thread_get();

    if (bt->id == id)
    {
        return bt;
    }

    not_thread_t *r1 = not_thread_find_by_id(bt, id);
    if (r1 != NULL)
    {
        return r1;
    }

    return NULL;
}

int32_t
not_thread_join(not_thread_t *thread)
{
#if defined(_WIN32) || defined(_WIN64)
    int32_t r1 = WaitForSingleObject(thread->thread, INFINITE);
    CloseHandle(thread->thread);

    if (r1 == WAIT_OBJECT_0)
    {
        return 0;
    }
    else
    {
        DWORD request = GetCurrentThreadId();
        not_error_system("'%s-%lu' could not join '%lu', %d", "not_thread",
                         request, thread->id, GetLastError());
        return -1;
    }
#else
    int32_t r1 = pthread_join(thread->id, NULL);
    if (r1 == 0)
    {
        return 0;
    }
    else
    {
        pthread_t request = pthread_self();
        not_error_system("'%s-%lu' could not join '%lu', %s", "not_thread",
                         request, thread->id, strerror(r1));
        return -1;
    }
#endif
}

int32_t
not_thread_join_all_childrens()
{
    not_thread_t *thread = not_thread_get_current();
    if (thread == ERROR)
    {
        return -1;
    }
    else if (thread == NULL)
    {
        return 0;
    }

    for (not_queue_entry_t *item = thread->childrens->begin; item != thread->childrens->end; item = item->next)
    {
        not_thread_t *t = (not_thread_t *)item->value;
        if (not_thread_join(t) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int32_t
not_thread_exit()
{
    not_thread_t *thread = not_thread_get_current();
    if (thread == ERROR)
    {
        return -1;
    }

    not_thread_t *parent = thread->parent;

    for (not_queue_entry_t *a = parent->childrens->begin, *b = NULL; a != parent->childrens->end; a = b)
    {
        b = a->next;
        not_thread_t *t = (not_thread_t *)a->value;
        if (t->id == thread->id)
        {
            not_queue_unlink(parent->childrens, a);
            break;
        }
    }

    not_interpreter_destroy(thread->interpreter);
    not_memory_free(thread);

#if defined(_WIN32) || defined(_WIN64)
    ExitThread(0);
#else
    pthread_exit(NULL);
#endif
    return 0;
}

void not_thread_sleep(uint64_t ms)
{
#if defined(_WIN32) || defined(_WIN64)
    sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

not_record_t *
not_thread_get_rax()
{
    not_thread_t *thread = not_thread_get_current();
    not_record_t *value = thread->interpreter->rax;
    return value;
}

void not_thread_set_rax(not_record_t *value)
{
    not_thread_t *thread = not_thread_get_current();
    thread->interpreter->rax = value;
}

not_record_t *
not_thread_get_and_set_rax(not_record_t *value)
{
    not_thread_t *thread = not_thread_get_current();

    not_record_t *result = thread->interpreter->rax;
    thread->interpreter->rax = value;

    return result;
}
