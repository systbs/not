#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <gmp.h>
#include <fcntl.h>

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

#define TO_RDONLY 0x0000
#define TO_WRONLY 0x0001
#define TO_RDWR 0x0002
#define TO_APPEND 0x0008
#define TO_CREAT 0x0100
#define TO_TRUNC 0x0200
#define TO_EXCL 0x0400
#define TO_ACCMODE (TO_RDONLY | TO_WRONLY | TO_RDWR)
#define TO_TEMPORARY 0x0040
#define TO_RANDOM 0x0010

mpz_t *
f_open(const char *path, mpz_t *flag, mpz_t *mode)
{
    int _flag = 0;

    int tflag = mpz_get_si(*flag);

#if defined(_WIN32) || defined(_WIN64)
    if ((tflag & TO_RDONLY) == TO_RDONLY)
    {
        _flag |= O_RDONLY;
    }

    if ((tflag & TO_WRONLY) == TO_WRONLY)
    {
        _flag |= O_WRONLY;
    }

    if ((tflag & TO_RDWR) == TO_RDWR)
    {
        _flag |= O_RDWR;
    }

    if ((tflag & TO_APPEND) == TO_APPEND)
    {
        _flag |= O_APPEND;
    }

    if ((tflag & TO_CREAT) == TO_CREAT)
    {
        _flag |= O_CREAT;
    }

    if ((tflag & TO_TRUNC) == TO_TRUNC)
    {
        _flag |= O_TRUNC;
    }

    if ((tflag & TO_EXCL) == TO_EXCL)
    {
        _flag |= O_EXCL;
    }

    if ((tflag & TO_ACCMODE) == TO_ACCMODE)
    {
        _flag |= O_ACCMODE;
    }

    if ((tflag & TO_TEMPORARY) == TO_TEMPORARY)
    {
        _flag |= O_TEMPORARY;
    }

    if ((tflag & TO_RANDOM) == TO_RANDOM)
    {
        _flag |= O_RANDOM;
    }
#else
    if ((tflag & TO_RDONLY) == TO_RDONLY)
    {
        _flag |= O_RDONLY;
    }

    if ((tflag & TO_WRONLY) == TO_WRONLY)
    {
        _flag |= O_WRONLY;
    }

    if ((tflag & TO_RDWR) == TO_RDWR)
    {
        _flag |= O_RDWR;
    }

    if ((tflag & TO_APPEND) == TO_APPEND)
    {
        _flag |= O_APPEND;
    }

    if ((tflag & TO_CREAT) == TO_CREAT)
    {
        _flag |= O_CREAT;
    }

    if ((tflag & TO_TRUNC) == TO_TRUNC)
    {
        _flag |= O_TRUNC;
    }

    if ((tflag & TO_EXCL) == TO_EXCL)
    {
        _flag |= O_EXCL;
    }

    if ((tflag & TO_ACCMODE) == TO_ACCMODE)
    {
        _flag |= O_ACCMODE;
    }

    if ((tflag & TO_TEMPORARY) == TO_TEMPORARY)
    {
        _flag |= __O_TMPFILE;
    }

    if ((tflag & TO_RANDOM) == TO_RANDOM)
    {
        _flag |= __O_DIRECT;
    }
#endif

#if defined(_WIN32) || defined(_WIN64)
    _flag |= O_BINARY;
    _flag |= O_SEQUENTIAL;
#else
    _flag |= __O_LARGEFILE;
    _flag |= O_NONBLOCK;
#endif

    int fd = open(path, _flag, 0644);

    mpz_t *num = calloc(1, sizeof(mpz_t));
    mpz_init(*num);
    mpz_set_si(*num, fd);

    return num;
}