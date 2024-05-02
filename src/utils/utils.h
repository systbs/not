#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__


# if __WORDSIZE == 64
#  define __INT64_C(c)	c ## L
#  define __UINT64_C(c)	c ## UL
# else
#  define __INT64_C(c)	c ## LL
#  define __UINT64_C(c)	c ## ULL
# endif

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
# define INT8_MIN		(-128)
# define INT16_MIN		(-32767-1)
# define INT32_MIN		(-2147483647-1)
# define INT64_MIN		(-__INT64_C(9223372036854775807)-1)
/* Maximum of signed integral types.  */
# define INT8_MAX		(127)
# define INT16_MAX		(32767)
# define INT32_MAX		(2147483647)
# define INT64_MAX		(__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
# define UINT8_MAX		(255)
# define UINT16_MAX		(65535)
# define UINT32_MAX		(4294967295U)
# define UINT64_MAX		(__UINT64_C(18446744073709551615))


/* Minimum of signed integral types having a minimum size.  */
# define INT_LEAST8_MIN		(-128)
# define INT_LEAST16_MIN	(-32767-1)
# define INT_LEAST32_MIN	(-2147483647-1)
# define INT_LEAST64_MIN	(-__INT64_C(9223372036854775807)-1)
/* Maximum of signed integral types having a minimum size.  */
# define INT_LEAST8_MAX		(127)
# define INT_LEAST16_MAX	(32767)
# define INT_LEAST32_MAX	(2147483647)
# define INT_LEAST64_MAX	(__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types having a minimum size.  */
# define UINT_LEAST8_MAX	(255)
# define UINT_LEAST16_MAX	(65535)
# define UINT_LEAST32_MAX	(4294967295U)
# define UINT_LEAST64_MAX	(__UINT64_C(18446744073709551615))


/* Minimum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MIN		(-128)
# if __WORDSIZE == 64
#  define INT_FAST16_MIN	(-9223372036854775807L-1)
#  define INT_FAST32_MIN	(-9223372036854775807L-1)
# else
#  define INT_FAST16_MIN	(-2147483647-1)
#  define INT_FAST32_MIN	(-2147483647-1)
# endif
# define INT_FAST64_MIN		(-__INT64_C(9223372036854775807)-1)
/* Maximum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MAX		(127)
# if __WORDSIZE == 64
#  define INT_FAST16_MAX	(9223372036854775807L)
#  define INT_FAST32_MAX	(9223372036854775807L)
# else
#  define INT_FAST16_MAX	(2147483647)
#  define INT_FAST32_MAX	(2147483647)
# endif
# define INT_FAST64_MAX		(__INT64_C(9223372036854775807))

/* Maximum of fast unsigned integral types having a minimum size.  */
# define UINT_FAST8_MAX		(255)
# if __WORDSIZE == 64
#  define UINT_FAST16_MAX	(18446744073709551615UL)
#  define UINT_FAST32_MAX	(18446744073709551615UL)
# else
#  define UINT_FAST16_MAX	(4294967295U)
#  define UINT_FAST32_MAX	(4294967295U)
# endif
# define UINT_FAST64_MAX	(__UINT64_C(18446744073709551615))


/* Values to test for integral types holding `void *' pointer.  */
# if __WORDSIZE == 64
#  define INTPTR_MIN		(-9223372036854775807L-1)
#  define INTPTR_MAX		(9223372036854775807L)
#  define UINTPTR_MAX		(18446744073709551615UL)
# else
#  define INTPTR_MIN		(-2147483647-1)
#  define INTPTR_MAX		(2147483647)
#  define UINTPTR_MAX		(4294967295U)
# endif


/* Minimum for largest signed integral type.  */
# define INTMAX_MIN		(-__INT64_C(9223372036854775807)-1)
/* Maximum for largest signed integral type.  */
# define INTMAX_MAX		(__INT64_C(9223372036854775807))

/* Maximum for largest unsigned integral type.  */
# define UINTMAX_MAX		(__UINT64_C(18446744073709551615))


char * 
utils_replace_char(char* str, char find, char replace);

double128_t
utils_stold(char *str);

int32_t
utils_isinteger(double128_t value);

int32_t
utils_indexof(char *str, char x);

int32_t
utils_issfloat(char *str);

#endif