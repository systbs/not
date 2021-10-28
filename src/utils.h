#pragma once

#define valid_alpha(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define valid_hexadecimal(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
#define valid_octal(c) (c >= '0' && c <= '7')
#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

#define max(a, b) ((a > b) ? a : b)
#define min(a,b) ((a < b) ? a : b)
#define abs(a) ((a > 0) ? a : -a)

void
utils_swap(char* a, char* b);

void
utils_reverse(char str[], int length);

char *
utils_itoa(int num, char* str, int base);

double
utils_pow(double a, double b);

int
utils_itos(int x, char str[], int d);

char *
utils_ftoa(double n, char *res, int afterpoint);

double
utils_atof (const char *p);

int
utils_atoi(char *str);

long64_t
check_hexadecimal(char *str);

long64_t
check_double(char *str);

long64_t
check_integer(char *str);

#define MAX_PATH 4098

void
utils_combine(char* destination, const char* path1, const char* path2);

char *
uuid(int n);

void
validate_format(int __cond__, char *__format__, ...);