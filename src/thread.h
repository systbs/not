#pragma once
#ifndef __THREAD_H__
#define __THREAD_H__ 1

#ifdef _WIN32
#include <windows.h>
typedef DWORD not_thread_id_t;
#else
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
typedef pthread_t not_thread_id_t;
#endif

typedef struct not_thread
{
    struct not_thread *parent;
    not_interpreter_t *interpreter;
    not_queue_t *childrens;
#ifdef _WIN32
    DWORD id;
    HANDLE thread;
#else
    pthread_t id;
#endif
} not_thread_t;

int32_t
not_thread_init();

void not_thread_destroy();

not_thread_t *
not_thread_get();

not_thread_t *
not_thread_create(
#ifdef _WIN32
    DWORD (*start_routine)(LPVOID), LPVOID arg
#else
    void *(*start_routine)(void *), void *arg
#endif
);

not_thread_t *
not_thread_get_current();

int32_t
not_thread_join(not_thread_t *thread);

int32_t
not_thread_join_all_childrens();

int32_t
not_thread_exit();

void not_thread_set_rax(not_record_t *value);

not_record_t *
not_thread_get_rax();

not_record_t *
not_thread_get_and_set_rax(not_record_t *value);

void not_thread_sleep(uint64_t ms);

#endif