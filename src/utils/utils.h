#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

char * 
sy_utils_replace_char(char* str, char find, char replace);

double128_t
sy_utils_stod(const char *str);

uint64_t
sy_utils_stoui(const char *str);

int32_t
sy_utils_index_of(char *str, char x);

int32_t
sy_utils_str_is_float(char *str);

#endif