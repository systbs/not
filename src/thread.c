#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "types/types.h"
#include "utils/path.h"
#include "token/position.h"
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

    if (sy_mutex_init(&bt->lock) < 0)
    {
        sy_error_system("'%s' could not initialize the lock", "sy_thread.lock");
        return -1;
    }

#ifdef _WIN32
    bt->id = GetCurrentThreadId();
    bt->thread = = GetCurrentThread();
#else
    bt->id = pthread_self();
#endif

    bt->begin = NULL;
    bt->parent = NULL;

    sy_interpreter_t *interpreter = sy_interpreter_create();
    if (!interpreter)
    {
        return -1;
    }
    bt->interpreter = interpreter;

    return 0;
}

int32_t
sy_thread_destroy()
{
    sy_thread_t *thread = sy_thread_get();

    if (sy_interpreter_destroy(thread->interpreter) < 0)
    {
        return -1;
    }

    if (sy_mutex_destroy(&thread->lock) < 0)
    {
        sy_error_system("'%s' could not destroy the lock", "sy_thread.lock");
        return -1;
    }
    return 0;
}

sy_thread_t *
sy_thread_get()
{
    return &base_thread;
}

static void
sy_thread_unsafe_link(sy_thread_t *parent, sy_thread_t *current, sy_thread_t *it)
{
    it->next = current;
    if (current)
    {
        it->previous = current->previous;
        current->previous = it;
        if (current->previous)
        {
            current->previous->next = it;
        }
    }

    if (parent->begin == current)
    {
        parent->begin = it;
    }

    it->parent = parent;
}

static void
sy_thread_unsafe_unlink(sy_thread_t *parent, sy_thread_t *it)
{
    if (it == parent->begin)
    {
        parent->begin = it->next;
    }

    if (it->next)
    {
        it->next->previous = it->previous;
    }

    if (it->previous)
    {
        it->previous->next = it->next;
    }
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

    if (sy_mutex_init(&t->lock) < 0)
    {
        sy_memory_free(t);
        sy_error_system("'%s-%lu' could not initialize the lock", "sy_thread.lock", t->id);
        return ERROR;
    }

    t->begin = NULL;

    sy_interpreter_t *interpreter = sy_interpreter_create();
    if (!interpreter)
    {
        return ERROR;
    }
    t->interpreter = interpreter;

    sy_thread_t *parent = sy_thread_get_current();
    if (parent == ERROR)
    {
        if (sy_interpreter_destroy(interpreter) < 0)
        {
            return ERROR;
        }
        sy_memory_free(t);
        return ERROR;
    }

    assert(parent != NULL);

    if (sy_mutex_lock(&parent->lock) < 0)
    {
        if (sy_interpreter_destroy(interpreter) < 0)
        {
            return ERROR;
        }
        sy_memory_free(t);
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", parent->id);
        return ERROR;
    }

    sy_thread_unsafe_link(parent, parent->begin, t);

    if (sy_mutex_unlock(&parent->lock) < 0)
    {
        if (sy_interpreter_destroy(interpreter) < 0)
        {
            return ERROR;
        }
        sy_memory_free(t);
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", parent->id);
        return ERROR;
    }

#ifdef _WIN32
    DWORD threadId;
    HANDLE thread = CreateThread(NULL, 0, sy_repository_load_by_thread, &data, 0, &threadId);
    if (!thread)
    {
        if (sy_interpreter_destroy(interpreter) < 0)
        {
            return ERROR;
        }
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
        if (sy_interpreter_destroy(interpreter) < 0)
        {
            return ERROR;
        }
        sy_memory_free(t);
        sy_error_system("new thread not created\n");
        return ERROR;
    }
    t->id = thread;
#endif

    return t;
}

static sy_thread_t *
sy_thread_unsafe_find_by_id(sy_thread_t *parent, sy_thread_id_t id)
{
    for (sy_thread_t *t = parent->begin; t != NULL; t = t->next)
    {
        if (t->id == id)
        {
            return t;
        }

        if (sy_mutex_lock(&t->lock) < 0)
        {
            sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", t->id);
            return ERROR;
        }

        sy_thread_t *r1 = sy_thread_unsafe_find_by_id(t, id);
        if (r1 != NULL)
        {
            if (sy_mutex_unlock(&t->lock) < 0)
            {
                sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", t->id);
                return ERROR;
            }
            return r1;
        }

        if (sy_mutex_unlock(&t->lock) < 0)
        {
            sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", t->id);
            return ERROR;
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
    if (sy_mutex_lock(&bt->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", bt->id);
        return ERROR;
    }

    if (bt->id == id)
    {
        if (sy_mutex_unlock(&bt->lock) < 0)
        {
            sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", bt->id);
            return ERROR;
        }
        return bt;
    }

    sy_thread_t *r1 = sy_thread_unsafe_find_by_id(bt, id);
    if (r1 != NULL)
    {
        if (sy_mutex_unlock(&bt->lock) < 0)
        {
            sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", bt->id);
            return ERROR;
        }
        return r1;
    }

    if (sy_mutex_unlock(&bt->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", bt->id);
        return ERROR;
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

    if (sy_mutex_lock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", thread->id);
        return -1;
    }

    for (sy_thread_t *t = thread->begin; t != NULL; t = t->next)
    {
        if (sy_thread_join(t) < 0)
        {
            if (sy_mutex_unlock(&thread->lock) < 0)
            {
                sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", thread->id);
                return -1;
            }
            return -1;
        }
    }

    if (sy_mutex_unlock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", thread->id);
        return -1;
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

    assert(thread->parent != NULL);

    sy_thread_t *parent = thread->parent;

    if (sy_mutex_lock(&parent->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", parent->id);
        return -1;
    }

    sy_thread_unsafe_unlink(parent, thread);

    if (sy_mutex_unlock(&parent->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", parent->id);
        return -1;
    }

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
    if (sy_mutex_lock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", thread->id);
        return ERROR;
    }

    sy_record_t *value = thread->interpreter->rax;

    if (sy_mutex_unlock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", thread->id);
        return ERROR;
    }

    return value;
}

int32_t
sy_thread_set_rax(sy_record_t *value)
{
    sy_thread_t *thread = sy_thread_get_current();
    if (sy_mutex_lock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", thread->id);
        return -1;
    }

    thread->interpreter->rax = value;

    if (sy_mutex_unlock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", thread->id);
        return -1;
    }

    return 0;
}

sy_record_t *
sy_thread_get_and_set_rax(sy_record_t *value)
{
    sy_thread_t *thread = sy_thread_get_current();
    if (sy_mutex_lock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not lock", "sy_thread.lock", thread->id);
        return ERROR;
    }

    sy_record_t *result = thread->interpreter->rax;
    thread->interpreter->rax = value;

    if (sy_mutex_unlock(&thread->lock) < 0)
    {
        sy_error_system("'%s-%lu' could not unlock", "sy_thread.lock", thread->id);
        return ERROR;
    }

    return result;
}