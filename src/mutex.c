#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include "types/types.h"
#include "mutex.h"

int32_t 
sy_mutex_init(sy_mutex_t *mutex)
{
#ifdef _WIN32
    mutex->handle = CreateMutex(NULL, FALSE, NULL);
    return (mutex->handle == NULL) ? -1 : 0;
#else
    return pthread_mutex_init(&mutex->handle, NULL);
#endif
}

int32_t 
sy_mutex_lock(sy_mutex_t *mutex)
{
#ifdef _WIN32
    return (WaitForSingleObject(mutex->handle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
#else
    return pthread_mutex_lock(&mutex->handle);
#endif
}

int32_t 
sy_mutex_unlock(sy_mutex_t *mutex)
{
#ifdef _WIN32
    return (ReleaseMutex(mutex->handle) != 0) ? 0 : -1;
#else
    return pthread_mutex_unlock(&mutex->handle);
#endif
}

int32_t 
sy_mutex_destroy(sy_mutex_t *mutex)
{
#ifdef _WIN32
    return (CloseHandle(mutex->handle) != 0) ? 0 : -1;
#else
    return pthread_mutex_destroy(&mutex->handle);
#endif
}