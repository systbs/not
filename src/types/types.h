#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stdbool.h>

typedef long double float128_t;

#if defined(_WIN32) || defined(_WIN64)
#else
#define _MAX_PATH 256
#define _MAX_FNAME 256
#define _MAX_EXT 256
#define _MAX_DIR 256
#define MAX_PATH (_MAX_PATH + _MAX_DIR + _MAX_FNAME + _MAX_EXT)
#endif

#define NOT_PTR_NULL ((void *)0)
#define NOT_PTR_ERROR ((void *)(int *)-1)

#endif /* __TYPES_H__ */
