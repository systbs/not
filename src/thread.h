#pragma once
#ifndef __THREAD_H__
#define __THREAD_H__ 1

#ifdef _WIN32
#include <windows.h>
typedef DWORD sy_thread_id_t;
#else
#include <pthread.h>
#include <errno.h>
typedef pthread_t sy_thread_id_t;
#endif

typedef struct sy_thread {
    struct sy_thread *begin;

    struct sy_thread *previous;
    struct sy_thread *next;

    struct sy_thread *parent;

#ifdef _WIN32
    DWORD id;
    HANDLE thread;
#else
    pthread_t id;
#endif

    sy_mutex_t lock;
} sy_thread_t;

int32_t
sy_thread_init();

int32_t
sy_thread_destroy();

sy_thread_t *
sy_thread_get();

#ifdef _WIN32
sy_thread_t *
sy_thread_create(DWORD (*start_routine)(LPVOID), LPVOID arg);
#else
sy_thread_t *
sy_thread_create(void *(*start_routine)(void *), void *arg);
#endif

sy_thread_t *
sy_thread_get_current();

int32_t
sy_thread_join(sy_thread_t *thread);

int32_t
sy_thread_join_all_childrens();

int32_t
sy_thread_exit();

#endif