#pragma once
#ifndef __MUTEX_H__
#define __MUTEX_H__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct not_mutex
{
#ifdef _WIN32
    HANDLE handle;
#else
    pthread_mutex_t handle;
#endif
} not_mutex_t;

int32_t
not_mutex_init(not_mutex_t *mutex);

int32_t
not_mutex_lock(not_mutex_t *mutex);

int32_t
not_mutex_unlock(not_mutex_t *mutex);

int32_t
not_mutex_destroy(not_mutex_t *mutex);

#endif