#pragma once
#ifndef __MUTEX_H__
#define __MUTEX_H__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct sy_mutex {
    #ifdef _WIN32
        HANDLE handle;
    #else
        pthread_mutex_t handle;
    #endif
} sy_mutex_t;

int32_t 
sy_mutex_init(sy_mutex_t *mutex);

int32_t 
sy_mutex_lock(sy_mutex_t *mutex);

int32_t 
sy_mutex_unlock(sy_mutex_t *mutex);

int32_t 
sy_mutex_destroy(sy_mutex_t *mutex);

#endif