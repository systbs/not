#include <stdio.h>
#include <stdarg.h>
#include <gmp.h>

int print(const char *message)
{
    return printf("%s", message);
}

int println(const char *message)
{
    return printf("%s\n", message);
}

int printfln(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int r = vprintf(format, arg);
    va_end(arg);
    printf("\n");
    return r;
}