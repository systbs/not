#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

#define MAX_PATH 4098

#define utils_isalpha(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define utils_ishex(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define utils_isoctal(c) (c >= '0' && c <= '7')
#define utils_isdigit(c) ((c) >= '0' && (c) <= '9')
#define utils_white_space(c) ((c) == ' ' || (c) == '\t')

char *
utils_itoa(int num, char* str, int base);

int
utils_itos(int x, char str[], int d);

char *
utils_ftoa(double n, char *res, int afterpoint);

double
utils_atof (const char *p);

int
utils_atoi(char *str);

int
utils_is_hexadecimal(char *str);

int
utils_is_double(char *str);

int
utils_is_integer(char *str);

void
utils_combine_path(char* destination, const char* path1, const char* path2);

#endif //__UTILS_H__
