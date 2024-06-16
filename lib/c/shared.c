#include <stdio.h>
#include <stdarg.h>
#include <gmp.h>

int add(int a, int b)
{
    return a + b;
}

void add2(mpz_t *a, mpz_t **b)
{
    gmp_printf("value = %Zd %Zd\n", *a, *b[0]);
}

int print(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int r = vprintf(format, arg);
    va_end(arg);

    return r;
}

int println(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int r = vprintf(format, arg);
    va_end(arg);
    printf("\n");
    return r;
}