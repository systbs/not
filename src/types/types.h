#pragma once
#ifndef __TYPES_H__
#define __TYPES_H__

typedef char 		char_t;
typedef short 		short_t;
typedef int 		int_t;
typedef long 		long_t;
typedef float 		float_t;
typedef double 		double_t;
typedef long double	double128_t;
typedef void 		void_t;

typedef unsigned char 	uchar_t;
typedef unsigned short 	ushort_t;
typedef unsigned int 	uint_t;
typedef unsigned long 	ulong_t;

typedef short int16_t;
typedef int int32_t;
//typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef unsigned long long uint64_t;

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

#endif /* __TYPES_H__ */
