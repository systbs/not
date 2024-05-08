#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

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