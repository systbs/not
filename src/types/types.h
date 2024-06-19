#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stdbool.h>

typedef long double float128_t;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#define MAX_PATH 4096
#endif

#define NOT_PTR_NULL ((void *)0)
#define NOT_PTR_ERROR ((void *)(int *)-1)

#endif /* __TYPES_H__ */
