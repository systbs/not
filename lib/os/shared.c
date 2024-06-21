#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <gmp.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

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

#define P_O_RDONLY 0x0000
#define P_O_WRONLY 0x0001
#define P_O_RDWR 0x0002
#define P_O_APPEND 0x0008
#define P_O_CREAT 0x0100
#define P_O_TRUNC 0x0200
#define P_O_EXCL 0x0400
#define P_O_ACCMODE (P_O_RDONLY | P_O_WRONLY | P_O_RDWR)
#define P_O_TEMPORARY 0x0040
#define P_O_RANDOM 0x0010

struct flag
{
    int term;
    int value;
};

struct flag flags[10] = {
    {P_O_RDONLY, O_RDONLY},
    {P_O_WRONLY, O_WRONLY},
    {P_O_RDWR, O_RDWR},
    {P_O_APPEND, O_APPEND},
    {P_O_CREAT, O_CREAT},
    {P_O_TRUNC, O_TRUNC},
    {P_O_EXCL, O_EXCL},
    {P_O_ACCMODE, O_ACCMODE},
#if defined(_WIN32) || defined(_WIN64)
    {P_O_TEMPORARY, O_TEMPORARY},
    {P_O_RANDOM, O_RANDOM}
#else
    {P_O_TEMPORARY, __O_TMPFILE},
    {P_O_RANDOM, __O_DIRECT}
#endif
};

#define P_S_IFMT 0xF000
#define P_S_IFDIR 0x4000
#define P_S_IFCHR 0x2000
#define P_S_IFREG 0x8000
#define P_S_IREAD 0x0100
#define P_S_IWRITE 0x0080
#define P_S_IEXEC 0x0040
#define P_S_IFIFO 0x1000
#define P_S_IFBLK 0x3000

#define P_S_IRWXU (P_S_IREAD | P_S_IWRITE | P_S_IEXEC)
#define P_S_IXUSR P_S_IEXEC
#define P_S_IWUSR P_S_IWRITE
#define P_S_IRUSR P_S_IREAD

#define P_S_IRGRP (P_S_IRUSR >> 3)
#define P_S_IWGRP (P_S_IWUSR >> 3)
#define P_S_IXGRP (P_S_IXUSR >> 3)
#define P_S_IRWXG (P_S_IRWXU >> 3)

#define P_S_IROTH (P_S_IRGRP >> 3)
#define P_S_IWOTH (P_S_IWGRP >> 3)
#define P_S_IXOTH (P_S_IXGRP >> 3)
#define P_S_IRWXO (P_S_IRWXG >> 3)

struct permission
{
    int term;
    int value;
};

struct permission permissions[21] = {
#if defined(_WIN32) || defined(_WIN64)
    {P_S_IFMT, S_IFMT},
    {P_S_IFDIR, S_IFDIR},
    {P_S_IFCHR, S_IFCHR},
    {P_S_IFREG, S_IFREG},
    {P_S_IREAD, S_IREAD},
    {P_S_IWRITE, S_IWRITE},
    {P_S_IEXEC, S_IEXEC},
    {P_S_IFIFO, S_IFIFO},
    {P_S_IFBLK, S_IFBLK},
#else
    {P_S_IFMT, __S_IFMT},
    {P_S_IFDIR, __S_IFDIR},
    {P_S_IFCHR, __S_IFCHR},
    {P_S_IFREG, __S_IFREG},
    {P_S_IREAD, __S_IREAD},
    {P_S_IWRITE, __S_IWRITE},
    {P_S_IEXEC, __S_IEXEC},
    {P_S_IFIFO, __S_IFIFO},
    {P_S_IFBLK, __S_IFBLK},
#endif
    {P_S_IRWXU, S_IRWXU},
    {P_S_IXUSR, S_IXUSR},
    {P_S_IWUSR, S_IWUSR},
    {P_S_IRUSR, S_IRUSR},
    {P_S_IRGRP, S_IRGRP},
    {P_S_IWGRP, S_IWGRP},
    {P_S_IXGRP, S_IXGRP},
    {P_S_IRWXG, S_IRWXG},
    {P_S_IROTH, S_IROTH},
    {P_S_IWOTH, S_IWOTH},
    {P_S_IXOTH, S_IXOTH},
    {P_S_IRWXO, S_IRWXO}};

mpz_t *
f_open(const char *path, mpz_t *flag, mpz_t *mode)
{
    int _flag = mpz_get_si(*flag);

    size_t num_flags = sizeof(flags) / sizeof(flags[0]);
    mode_t final_flags = 0;

    for (size_t i = 0; i < num_flags; i++)
    {
        if ((_flag & flags[i].term) == flags[i].term)
            final_flags |= flags[i].value;
    }

#if defined(_WIN32) || defined(_WIN64)
    final_flags |= O_BINARY;
    final_flags |= O_SEQUENTIAL;
#else
    final_flags |= __O_LARGEFILE;
    final_flags |= O_NONBLOCK;
#endif

    int _mode = mpz_get_si(*mode);

    size_t num_permissions = sizeof(permissions) / sizeof(permissions[0]);
    mode_t final_modes = 0;

    for (size_t i = 0; i < num_permissions; i++)
    {
        if ((_mode & permissions[i].term) == permissions[i].term)
            final_modes |= permissions[i].value;
    }

    int fd = open(path, final_flags, final_modes);

    mpz_t *r = calloc(1, sizeof(mpz_t));
    mpz_init(*r);
    mpz_set_si(*r, fd);

    return r;
}

char *
f_read(mpz_t *fd, mpz_t *count)
{
    int _fd = mpz_get_si(*fd);
    ssize_t __nbytes = mpz_get_si(*count);

    if (__nbytes < 0)
    {
        char buffer[1024];
        ssize_t bytesRead;
        ssize_t totalBytesRead = 0;
        size_t bufferSize = sizeof(buffer);

        size_t contentSize = 1024;
        char *content = (char *)malloc(contentSize);
        if (content == NULL)
        {
            goto err;
        }

        while ((bytesRead = read(_fd, buffer, bufferSize)) > 0)
        {
            while (totalBytesRead + bytesRead > contentSize)
            {
                contentSize *= 2;
                content = (char *)realloc(content, contentSize);
                if (content == NULL)
                {
                    goto err;
                }
            }
            memcpy(content + totalBytesRead, buffer, bytesRead);
            totalBytesRead += bytesRead;
        }

        if (bytesRead == -1)
        {
            free(content);
            goto err;
        }

        return content;
    }
    else
    {
        ssize_t bytesRead;

        char *buffer = (char *)malloc(__nbytes);
        if (buffer == NULL)
        {
            goto err;
        }

        bytesRead = read(_fd, buffer, __nbytes);
        if (bytesRead == -1)
        {
            free(buffer);
            goto err;
        }

        return buffer;
    }

err:
    return NULL;
}

mpz_t *
f_write(mpz_t *fd, const char *buf, mpz_t *n)
{
    int _fd = mpz_get_si(*fd);
    ssize_t nbytes = mpz_get_ui(*n);

    ssize_t t = write(_fd, buf, nbytes);

    mpz_t *r = calloc(1, sizeof(mpz_t));
    mpz_init(*r);
    mpz_set_si(*r, t);

    return r;
}

mpz_t *
f_close(mpz_t *fd)
{
    int _fd = mpz_get_si(*fd);
    int t = close(_fd);

    mpz_t *r = calloc(1, sizeof(mpz_t));
    mpz_init(*r);
    mpz_set_si(*r, t);

    return r;
}

#define P_SEEK_SET 0
#define P_SEEK_CUR 1
#define P_SEEK_END 2

struct whence
{
    int term;
    int value;
};

struct whence whences[3] = {
#if defined(_WIN32) || defined(_WIN64)
    {P_SEEK_SET, SEEK_SET},
    {P_SEEK_CUR, SEEK_CUR},
    {P_SEEK_END, SEEK_END}
#else
    {P_SEEK_SET, SEEK_SET},
    {P_SEEK_CUR, SEEK_CUR},
    {P_SEEK_END, SEEK_END}
#endif
};

mpz_t *
f_seek(mpz_t *fd, mpz_t *offset, mpz_t *whence)
{
    int _fd = mpz_get_si(*fd);
    long _off = mpz_get_si(*offset);
    int _whence = mpz_get_si(*whence);

    size_t num_whences = sizeof(whences) / sizeof(whences[0]);
    mode_t final_whences = 0;

    for (size_t i = 0; i < num_whences; i++)
    {
        if ((_whence & whences[i].term) == whences[i].term)
            final_whences |= whences[i].value;
    }

    long t = lseek(_fd, _off, final_whences);

    mpz_t *r = calloc(1, sizeof(mpz_t));
    mpz_init(*r);
    mpz_set_si(*r, t);

    return r;
}