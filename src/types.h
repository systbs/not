#pragma once

typedef char 		char_t;
typedef short 		short_t;
typedef int 		int_t;
typedef long 		long_t;
typedef float 		float_t;
typedef double 		double_t;
typedef long long 	long64_t;
typedef long double double64_t;

typedef void * ptr_t;
typedef char * string_t;
typedef void * void_t;

typedef unsigned char 		 uchar_t;
typedef unsigned short 		 ushort_t;
typedef unsigned int 		 uint_t;
typedef unsigned long 		 ulong_t;
typedef unsigned long long 	 ulong64_t;

typedef int bool_t;
#define true 1
#define false 0

#ifndef NULL
//Possible implementation
#define NULL 0
//since C++11
#endif


#define nullptr 0


typedef long64_t  arval_t;
typedef long64_t* tbval_t;

typedef double_t var_t;