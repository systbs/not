#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

#define MIN_UINT8 (-2^07)+1
#define MAX_UINT8 (+2^07)-1
#define MIN_INT16 (-2^15)+1 
#define MAX_INT16 (+2^15)-1
#define MIN_INT32 (-2^31)+1
#define MAX_INT32 (+2^31)-1
#define MIN_INT64 (-2^63)+1
#define MAX_INT64 (+2^63)-1

char * 
utils_replace_char(char* str, char find, char replace);

double128_t
utils_stold(char *str);

int32_t
utils_isinteger(double128_t value);

#endif