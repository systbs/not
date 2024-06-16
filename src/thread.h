#pragma once
#ifndef __THREAD_H__
#define __THREAD_H__ 1

#ifdef _WIN32
#include <windows.h>
typedef DWORD sy_thread_id_t;
#else
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
typedef pthread_t sy_thread_id_t;
#endif

typedef struct sy_thread
{
    struct sy_thread *parent;
    sy_interpreter_t *interpreter;
    sy_queue_t *childrens;
#ifdef _WIN32
    DWORD id;
    HANDLE thread;
#else
    pthread_t id;
#endif
} sy_thread_t;

int32_t
sy_thread_init();

void sy_thread_destroy();

sy_thread_t *
sy_thread_get();

sy_thread_t *
sy_thread_create(
#ifdef _WIN32
    DWORD (*start_routine)(LPVOID), LPVOID arg
#else
    void *(*start_routine)(void *), void *arg
#endif
);

sy_thread_t *
sy_thread_get_current();

int32_t
sy_thread_join(sy_thread_t *thread);

int32_t
sy_thread_join_all_childrens();

int32_t
sy_thread_exit();

void sy_thread_set_rax(sy_record_t *value);

sy_record_t *
sy_thread_get_rax();

sy_record_t *
sy_thread_get_and_set_rax(sy_record_t *value);

void sy_thread_sleep(uint64_t ms);

#endif