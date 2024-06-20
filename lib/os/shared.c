#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <gmp.h>

mpz_t *
add(mpz_t *a, mpz_t *b)
{
    mpz_t *num = calloc(1, sizeof(mpz_t));
    mpz_init(*num);
    mpz_add(*num, *a, *b);

    return num;
}

mpz_t *
print(const char *message)
{
    int r = printf("%s", message);

    mpz_t *num = calloc(1, sizeof(mpz_t));
    mpz_init(*num);
    mpz_set_si(*num, r);

    return num;
}

mpz_t *
println(const char *message)
{
    int r = printf("%s\n", message);

    mpz_t *num = calloc(1, sizeof(mpz_t));
    mpz_init(*num);
    mpz_set_si(*num, r);

    return num;
}

mpz_t *
printfln(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int r = vprintf(format, arg);
    va_end(arg);
    printf("\n");

    mpz_t *num = calloc(1, sizeof(mpz_t));
    mpz_init(*num);
    mpz_set_si(*num, r);

    return num;
}