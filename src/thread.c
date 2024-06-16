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

sy_thread_t base_thread;

int32_t
sy_thread_init()
{
    sy_thread_t *bt = sy_thread_get();

#ifdef _WIN32
    bt->id = GetCurrentThreadId();
    bt->thread = = GetCurrentThread();
#else
    bt->id = pthread_self();
#endif

    bt->parent = NULL;

    bt->interpreter = sy_interpreter_create();
    if (bt->interpreter == ERROR)
    {
        return -1;
    }

    bt->childrens = sy_queue_create();
    if (bt->childrens == ERROR)
    {
        sy_interpreter_destroy(bt->interpreter);
        return -1;
    }

    return 0;
}

void sy_thread_destroy()
{
    sy_thread_t *thread = sy_thread_get();
    sy_interpreter_destroy(thread->interpreter);
}

sy_thread_t *
sy_thread_get()
{
    return &base_thread;
}

sy_thread_t *
sy_thread_create(
#ifdef _WIN32
    DWORD (*start_routine)(LPVOID), LPVOID arg
#else
    void *(*start_routine)(void *), void *arg
#endif
)
{
    sy_thread_t *t = (sy_thread_t *)sy_memory_calloc(1, sizeof(sy_thread_t));
    if (!t)
    {
        sy_error_no_memory();
        return ERROR;
    }

    t->interpreter = sy_interpreter_create();
    if (t->interpreter == ERROR)
    {
        sy_memory_free(t);
        return ERROR;
    }

    t->childrens = sy_queue_create();
    if (t->childrens == ERROR)
    {
        sy_interpreter_destroy(t->interpreter);
        sy_memory_free(t);
        return ERROR;
    }

    sy_thread_t *parent = sy_thread_get_current();
    if (parent == ERROR)
    {
        sy_interpreter_destroy(t->interpreter);
        sy_memory_free(t);
        return ERROR;
    }

    if (ERROR == sy_queue_right_push(parent->childrens, t))
    {
        sy_interpreter_destroy(t->interpreter);
        sy_memory_free(t);
        return ERROR;
    }

#ifdef _WIN32
    DWORD threadId;
    HANDLE thread = CreateThread(NULL, 0, sy_repository_load_by_thread, &data, 0, &threadId);
    if (!thread)
    {
        sy_interpreter_destroy(t->interpreter);
        sy_memory_free(t);
        sy_error_system("new thread not created\n");
        return ERROR;
    }

    t->id = threadId;
    t->thread = = thread;
#else
    pthread_t thread;
    if (pthread_create(&thread, NULL, start_routine, arg) != 0)
    {
        sy_interpreter_destroy(t->interpreter);
        sy_memory_free(t);
        sy_error_system("new thread not created\n");
        return ERROR;
    }
    t->id = thread;
#endif

    return t;
}

static sy_thread_t *
sy_thread_find_by_id(sy_thread_t *parent, sy_thread_id_t id)
{
    sy_queue_entry_t *item;
    for (item = parent->childrens->begin; item != parent->childrens->end; item = item->next)
    {
        sy_thread_t *t = (sy_thread_t *)item->value;

        if (t->id == id)
        {
            return t;
        }

        sy_thread_t *r1 = sy_thread_find_by_id(t, id);
        if (r1 != NULL)
        {
            return r1;
        }
    }

    return NULL;
}

sy_thread_t *
sy_thread_get_current()
{
#ifdef _WIN32
    DWORD id = GetCurrentThreadId();
#else
    pthread_t id = pthread_self();
#endif

    sy_thread_t *bt = sy_thread_get();

    if (bt->id == id)
    {
        return bt;
    }

    sy_thread_t *r1 = sy_thread_find_by_id(bt, id);
    if (r1 != NULL)
    {
        return r1;
    }

    return NULL;
}

int32_t
sy_thread_join(sy_thread_t *thread)
{
#ifdef _WIN32
    int32_t r1 = WaitForSingleObject(thread->thread, INFINITE);
    CloseHandle(thread->thread);

    if (r1 == WAIT_OBJECT_0)
    {
        return 0;
    }
    else
    {
        DWORD request = GetCurrentThreadId();
        sy_error_system("'%s-%lu' could not join '%lu', %d", "sy_thread",
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
        sy_error_system("'%s-%lu' could not join '%lu', %s", "sy_thread",
                        request, thread->id, strerror(r1));
        return -1;
    }
#endif
}

int32_t
sy_thread_join_all_childrens()
{
    sy_thread_t *thread = sy_thread_get_current();
    if (thread == ERROR)
    {
        return -1;
    }
    else if (thread == NULL)
    {
        return 0;
    }

    for (sy_queue_entry_t *item = thread->childrens->begin; item != thread->childrens->end; item = item->next)
    {
        sy_thread_t *t = (sy_thread_t *)item->value;
        if (sy_thread_join(t) < 0)
        {
            return -1;
        }
    }

    return 0;
}

int32_t
sy_thread_exit()
{
    sy_thread_t *thread = sy_thread_get_current();
    if (thread == ERROR)
    {
        return -1;
    }

    sy_thread_t *parent = thread->parent;

    for (sy_queue_entry_t *a = parent->childrens->begin, *b = NULL; a != parent->childrens->end; a = b)
    {
        b = a->next;
        sy_thread_t *t = (sy_thread_t *)a->value;
        if (t->id == thread->id)
        {
            sy_queue_unlink(parent->childrens, a);
            break;
        }
    }

    sy_interpreter_destroy(thread->interpreter);
    sy_memory_free(thread);

#ifdef _WIN32
    ExitThread(0);
#else
    pthread_exit(NULL);
#endif
    return 0;
}

void sy_thread_sleep(uint64_t ms)
{
#if _WIN32
    sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

sy_record_t *
sy_thread_get_rax()
{
    sy_thread_t *thread = sy_thread_get_current();
    sy_record_t *value = thread->interpreter->rax;
    return value;
}

void sy_thread_set_rax(sy_record_t *value)
{
    sy_thread_t *thread = sy_thread_get_current();
    thread->interpreter->rax = value;
}

sy_record_t *
sy_thread_get_and_set_rax(sy_record_t *value)
{
    sy_thread_t *thread = sy_thread_get_current();

    sy_record_t *result = thread->interpreter->rax;
    thread->interpreter->rax = value;

    return result;
}
