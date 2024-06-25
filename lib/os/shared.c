#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <gmp.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <jansson.h>
#include <dirent.h>
#include <errno.h>

#include "../not.h"

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

not_record_t *
f_open(not_record_t *args) // (const char *path, mpz_t *flag, mpz_t *mode)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    not_record_t *arg1 = not_record_tuple_arg_by_index(args, 1);
    not_record_t *arg2 = not_record_tuple_arg_by_index(args, 2);

    int _flag = mpz_get_ui(*(mpz_t *)arg1->value);

    size_t num_flags = sizeof(flags) / sizeof(flags[0]);
    unsigned int final_flags = 0;

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

    unsigned int mode = mpz_get_ui(*(mpz_t *)arg2->value);

    size_t num_permissions = sizeof(permissions) / sizeof(permissions[0]);
    mode_t final_modes = 0;

    for (size_t i = 0; i < num_permissions; i++)
    {
        if ((mode & permissions[i].term) == permissions[i].term)
            final_modes |= permissions[i].value;
    }

    int fd = open((char *)arg0->value, final_flags, final_modes);

    return not_record_make_int_from_si(fd);
}

not_record_t *
f_read(not_record_t *args) // (mpz_t *fd, mpz_t *count)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    not_record_t *arg1 = not_record_tuple_arg_by_index(args, 1);

    int fd = mpz_get_si(*(mpz_t *)arg0->value);
    ssize_t nbytes = mpz_get_si(*(mpz_t *)arg1->value);

    if (nbytes < 0)
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

        while ((bytesRead = read(fd, buffer, bufferSize)) > 0)
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

        not_record_t *record = not_record_make_string(content);
        free(content);
        return record;
    }
    else
    {
        ssize_t bytesRead;

        char *buffer = (char *)malloc(nbytes);
        if (buffer == NULL)
        {
            goto err;
        }

        bytesRead = read(fd, buffer, nbytes);
        if (bytesRead == -1)
        {
            free(buffer);
            goto err;
        }

        not_record_t *record = not_record_make_string(buffer);
        free(buffer);
        return record;
    }

err:
    return not_record_make_null();
}

static inline void
replace_escaped(const char *input, char *output)
{
    while (*input)
    {
        if (*input == '\\' && *(input + 1) == 'n')
        {
            *output++ = '\n';
            input += 2;
        }
        else if (*input == '\\' && *(input + 1) == 't')
        {
            *output++ = '\t';
            input += 2;
        }
        else if (*input == '\\' && *(input + 1) == 'r')
        {
            *output++ = '\r';
            input += 2;
        }
        else if (*input == '\\' && *(input + 1) == 'v')
        {
            *output++ = '\v';
            input += 2;
        }
        else if (*input == '\\' && *(input + 1) == '\\')
        {
            *output++ = '\\';
            input += 2;
        }
        else
        {
            *output++ = *input++;
        }
    }
    *output = '\0';
}

not_record_t *
f_write(not_record_t *args) // (mpz_t *fd, const char *buf, mpz_t *n)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    not_record_t *arg1 = not_record_tuple_arg_by_index(args, 1);
    not_record_t *arg2 = not_record_tuple_arg_by_index(args, 2);

    int fd = mpz_get_si(*(mpz_t *)arg0->value);
    const char *buf = (char *)arg1->value;
    ssize_t nbytes = mpz_get_ui(*(mpz_t *)arg2->value);

    ssize_t t = -1;

    char *modified = calloc(strlen(buf) + 1, sizeof(char));
    if (!modified)
    {
        goto final;
    }
    replace_escaped(buf, modified);

    t = write(fd, modified, nbytes);

    free(modified);

final:

    return not_record_make_int_from_si(t);
}

not_record_t *
f_close(not_record_t *args) // (mpz_t *fd)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);

    int fd = mpz_get_si(*(mpz_t *)arg0->value);
    int t = close(fd);

    return not_record_make_int_from_si(t);
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

not_record_t *
f_seek(not_record_t *args) // (mpz_t *fd, mpz_t *offset, mpz_t *whence)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    not_record_t *arg1 = not_record_tuple_arg_by_index(args, 1);
    not_record_t *arg2 = not_record_tuple_arg_by_index(args, 2);

    int fd = mpz_get_si(*(mpz_t *)arg0->value);
    long off = mpz_get_si(*(mpz_t *)arg1->value);
    int whence = mpz_get_si(*(mpz_t *)arg2->value);

    size_t num_whences = sizeof(whences) / sizeof(whences[0]);
    mode_t final_whences = 0;

    for (size_t i = 0; i < num_whences; i++)
    {
        if ((whence & whences[i].term) == whences[i].term)
            final_whences |= whences[i].value;
    }

    long t = lseek(fd, off, final_whences);

    return not_record_make_int_from_si(t);
}

static inline void
get_input(char **buffer, size_t *size)
{
    int c;
    size_t i = 0;

    while ((c = getchar()) != '\n' && c != EOF)
    {
        if (c == '\b' || c == 127)
        { // 127 is the ASCII code for DEL
            if (i > 0)
            {
                // Move the cursor back
                printf("\b \b");
                fflush(stdout);
                i--;
            }
        }
        else
        {
            if (i >= *size - 1)
            {
                *size *= 2;
                *buffer = realloc(*buffer, *size);
                if (*buffer == NULL)
                {
                    return;
                }
            }
            (*buffer)[i++] = c;
            fflush(stdout);
        }
    }

    (*buffer)[i] = '\0';
}

not_record_t *
f_gets(not_record_t *args)
{
    size_t buffer_size = 100;
    char *buffer = (char *)malloc(buffer_size * sizeof(char));

    if (buffer == NULL)
    {
        goto err;
    }

    fflush(stdout);
    get_input(&buffer, &buffer_size);

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    not_record_t *record = not_record_make_string(buffer);
    free(buffer);
    return record;

err:
    return not_record_make_null();
}

not_record_t *
f_getc(not_record_t *args)
{
    int c = getchar();

    return not_record_make_char(c);

err:
    return not_record_make_null();
}

not_record_t *
f_stat(not_record_t *args) // (const char *path)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    const char *path = (char *)arg0->value;

    struct stat st;
    if (stat(path, &st) < 0)
    {
        return not_record_make_int_from_si(errno);
    }

    size_t num_permissions = sizeof(permissions) / sizeof(permissions[0]);
    mode_t final_modes = 0;

    for (size_t i = 0; i < num_permissions; i++)
    {
        if ((st.st_mode & permissions[i].term) == permissions[i].term)
            final_modes |= permissions[i].value;
    }

    not_record_t *value;
    not_record_object_t *top = NULL, *iter = NULL;

    value = not_record_make_int_from_si(st.st_ctime);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter = not_record_make_object("ctime", value, NULL);
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    top = iter;

    value = not_record_make_int_from_si(st.st_mtime);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("mtime", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_si(st.st_atime);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("atime", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_size);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("size", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_rdev);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("rdev", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_gid);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("gid", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_uid);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("uid", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_nlink);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("nlink", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(final_modes);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("mode", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_ino);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("ino", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    value = not_record_make_int_from_ui(st.st_dev);
    if (value == NOT_PTR_ERROR)
    {
        goto cleanup;
    }
    iter->next = not_record_make_object("dev", value, NULL);
    iter = iter->next;
    if (iter == NOT_PTR_ERROR)
    {
        goto cleanup;
    }

    return not_record_create(RECORD_KIND_OBJECT, top);

cleanup:
    if (top)
        not_record_object_destroy(top);

err:
    return not_record_make_null();
}

not_record_t *
f_remove(not_record_t *args) // (const char *path)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    const char *path = (char *)arg0->value;

    if (remove(path) == 0)
    {
        return not_record_make_int_from_si(0);
    }
    else
    {
        return not_record_make_int_from_si(errno);
    }
}

typedef struct
{
    int dd_fd;
    long dd_loc;
    long dd_size;
    char *dd_buf;
    int dd_len;
    long dd_seek;
    void *dd_direct;
} MyDIR;

not_record_t *
d_read(not_record_t *args) // (const char *path)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);

    const char *path = (char *)arg0->value;

    DIR *dp = opendir(path);
    if (dp == NULL)
    {
        return not_record_make_int_from_si(errno);
    }

    not_record_tuple_t *tuple_top = NULL,
                       *tuple_iter = NULL;

    struct dirent *entry;
    while ((entry = readdir(dp)))
    {
        not_record_t *value;
        not_record_object_t *top = NULL, *iter = NULL;

        value = not_record_make_string(entry->d_name);
        if (value == NOT_PTR_ERROR)
        {
            goto cleanup;
        }
        iter = not_record_make_object("name", value, NULL);
        if (iter == NOT_PTR_ERROR)
        {
            goto cleanup;
        }
        top = iter;

        value = not_record_make_int_from_ui(entry->d_type);
        if (value == NOT_PTR_ERROR)
        {
            goto cleanup;
        }
        iter->next = not_record_make_object("type", value, NULL);
        iter = iter->next;
        if (iter == NOT_PTR_ERROR)
        {
            goto cleanup;
        }

        value = not_record_make_int_from_ui(entry->d_reclen);
        if (value == NOT_PTR_ERROR)
        {
            goto cleanup;
        }
        iter->next = not_record_make_object("length", value, NULL);
        iter = iter->next;
        if (iter == NOT_PTR_ERROR)
        {
            goto cleanup;
        }

        value = not_record_make_int_from_ui(entry->d_off);
        if (value == NOT_PTR_ERROR)
        {
            goto cleanup;
        }
        iter->next = not_record_make_object("offset", value, NULL);
        iter = iter->next;
        if (iter == NOT_PTR_ERROR)
        {
            goto cleanup;
        }

        value = not_record_make_int_from_ui(entry->d_ino);
        if (value == NOT_PTR_ERROR)
        {
            goto cleanup;
        }
        iter->next = not_record_make_object("ino", value, NULL);
        iter = iter->next;
        if (iter == NOT_PTR_ERROR)
        {
            goto cleanup;
        }

        not_record_t *record_object = not_record_create(RECORD_KIND_OBJECT, top);
        if (record_object == NOT_PTR_ERROR)
        {
            goto cleanup;
        }

        not_record_tuple_t *tuple = not_record_make_tuple(record_object, NULL);
        if (tuple == NOT_PTR_ERROR)
        {
            goto cleanup;
        }

        if (!tuple_top)
        {
            tuple_top = tuple;
            tuple_iter = tuple;
        }
        else
        {
            tuple_iter->next = tuple;
            tuple_iter = tuple;
        }
        continue;

    cleanup:
        if (top)
            not_record_object_destroy(top);
        goto err;
    }

    closedir(dp);

    not_record_t *record_tuple = not_record_create(RECORD_KIND_TUPLE, tuple_top);
    if (record_tuple == NOT_PTR_ERROR)
    {
        goto err;
    }

    return record_tuple;

err:
    if (tuple_top)
        not_record_tuple_destroy(tuple_top);

    return not_record_make_int_from_si(errno);
}

not_record_t *
d_remove(not_record_t *args) // (const char *path)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    const char *path = (char *)arg0->value;

    if (rmdir(path) == 0)
    {
        return not_record_make_int_from_si(0);
    }
    else
    {
        return not_record_make_int_from_si(errno);
    }
}

not_record_t *
d_make(not_record_t *args) // (const char *path)
{
    not_record_t *arg0 = not_record_tuple_arg_by_index(args, 0);
    not_record_t *arg1 = not_record_tuple_arg_by_index(args, 1);
    const char *path = (char *)arg0->value;

    unsigned int mode = mpz_get_ui(*(mpz_t *)arg1->value);

    size_t num_permissions = sizeof(permissions) / sizeof(permissions[0]);
    mode_t final_modes = 0;

    for (size_t i = 0; i < num_permissions; i++)
    {
        if ((mode & permissions[i].term) == permissions[i].term)
            final_modes |= permissions[i].value;
    }

    if (mkdir(path, final_modes) == 0)
    {
        return not_record_make_int_from_si(0);
    }
    else
    {
        return not_record_make_int_from_si(errno);
    }
}
