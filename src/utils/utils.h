#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

char * 
SyUtils_ReplaceChar(char* str, char find, char replace);

double128_t
SyUtils_SToUiD(const char *str);

uint64_t
SyUtils_SToUi(const char *str);

int32_t
SyUtils_IndexOf(char *str, char x);

int32_t
SyUtils_StrIsFloat(char *str);

#endif