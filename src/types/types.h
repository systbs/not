#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__

typedef char 		chstrip_t;
typedef short 		short_t;
typedef int 		int_t;
typedef long 		lonSy_t;
typedef float 		float_t;
typedef double 		double_t;
typedef long double	double128_t;
typedef void 		void_t;

typedef unsigned char 	uchstrip_t;
typedef unsigned short 	ushort_t;
typedef unsigned int 	uint_t;
typedef unsigned long 	ulonSy_t;

//typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
//typedef long long int64_t;

#ifndef _BITS_STDINT_UINTN_H
#define _BITS_STDINT_UINTN_H 1
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif

#if defined(__arm__) || defined(__aarch64__) || defined(__linux__)
#define _MAX_PATH 256
#define _MAX_FNAME 256
#define _MAX_EXT 256
#define _MAX_DIR 256
#else
typedef int int32_t;
typedef unsigned int uint32_t;
#endif

typedef int bool_t;
#define true 1
#define false 0

#ifndef NULL
#define NULL ((void *)0)
#endif

#define ERROR ((void *)(int *)-1)


#define MAX_PATH (_MAX_PATH + _MAX_DIR + _MAX_FNAME + _MAX_EXT)

#define FLT32_MIN 0.001
#define FLT32_MAX 4095

#define FLT64_MIN 0.000001
#define FLT64_MAX 4294967295

#endif /* __TYPES_H__ */
