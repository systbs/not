#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include "types/types.h"
#include "mutex.h"

int32_t
not_mutex_init(not_mutex_t *mutex)
{
#if defined(_WIN32) || defined(_WIN64)
    mutex->handle = CreateMutex(NULL, FALSE, NULL);
    return (mutex->handle == NULL) ? -1 : 0;
#else
    return pthread_mutex_init(&mutex->handle, NULL);
#endif
}

int32_t
not_mutex_lock(not_mutex_t *mutex)
{
#if defined(_WIN32) || defined(_WIN64)
    return (WaitForSingleObject(mutex->handle, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
#else
    return pthread_mutex_lock(&mutex->handle);
#endif
}

int32_t
not_mutex_unlock(not_mutex_t *mutex)
{
#if defined(_WIN32) || defined(_WIN64)
    return (ReleaseMutex(mutex->handle) != 0) ? 0 : -1;
#else
    return pthread_mutex_unlock(&mutex->handle);
#endif
}

int32_t
not_mutex_destroy(not_mutex_t *mutex)
{
#if defined(_WIN32) || defined(_WIN64)
    return (CloseHandle(mutex->handle) != 0) ? 0 : -1;
#else
    return pthread_mutex_destroy(&mutex->handle);
#endif
}