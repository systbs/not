#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include "../types/types.h"
#include "path.h"

enum path_style_type
{
    PATH_STYLE_WINDOWS,
    PATH_STYLE_UNIX
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(_WIN64)
#include <io.h>
#include <direct.h>
#define F_OK 0
#define access _access
#define getcwd _getcwd
enum path_style_type path_style = PATH_STYLE_WINDOWS;
#else
#include <unistd.h>
enum path_style_type path_style = PATH_STYLE_UNIX;
#endif

#define windows_separator '\\'
#define unix_separator '/'

typedef struct not_path_segment
{
    const char *path;
    const char *segments;
    const char *begin;
    const char *end;
    size_t size;
} not_path_segment_t;

typedef enum not_path_segment_type
{
    PATH_NORMAL,
    PATH_CURRENT,
    PATH_BACK
} not_path_segment_type_t;

typedef struct not_path_segment_joined
{
    not_path_segment_t segment;
    const char **paths;
    size_t path_index;
} not_path_segment_joined_t;

int32_t
not_path_is_directory(const char *path)
{
    struct stat st;

    if (stat(path, &st) < 0)
        return -1;

    return S_ISDIR(st.st_mode);
}

static int32_t
not_path_is_separator(const char *path)
{
    if ((*path == windows_separator) || (*path == unix_separator))
    {
        return 1;
    }
    return 0;
}

static const char *
not_path_find_next_stop(const char *path)
{
    while (*path != '\0' && !not_path_is_separator(path))
    {
        ++path;
    }

    return path;
}

static const char *
not_path_find_previous_stop(const char *begin, const char *c)
{
    while (c > begin && !not_path_is_separator(c))
    {
        --c;
    }

    if (not_path_is_separator(c))
    {
        return c + 1;
    }
    else
    {
        return c;
    }
}

static void
not_path_get_root(const char *path, size_t *length)
{
    const char *c;
    c = path;
    *length = 0;
    if (!c)
    {
        return;
    }

    // We have to verify whether this is a network path
    if (not_path_is_separator(c))
    {
        ++c;
        if (!not_path_is_separator(c))
        {
            // Okey, this is not a network path
            *length = 1;
            return;
        }

        // A device path is a path which starts with "\\." or "\\?"
        ++c;
        if ((*c == '.' || *c == '?') && not_path_is_separator(++c))
        {
            *length = 4;
            return;
        }

        // We will grab anything up to next stop, that will be the server name
        c = not_path_find_next_stop(c);

        while (not_path_is_separator(c))
        {
            if (*c == '\0')
            {
                *length = (size_t)(c - path);
                return;
            }
            ++c;
        }

        // We are now skipping the shared folder name
        c = not_path_find_next_stop(c);

        if (not_path_is_separator(c))
        {
            ++c;
        }

        *length = (size_t)(c - path);
        return;
    }

    if (*++c == ':')
    {
        *length = 2;

        if (not_path_is_separator(++c))
        {
            *length = 3;
        }
    }
}

static int32_t
not_path_is_root_absolute(const char *path, size_t length)
{
    // This is definitely not absolute if there is no root.
    if (length == 0)
    {
        return 0;
    }

    // If there is a separator at the end of the root, we can safely consider this
    // to be an absolute path.
    return not_path_is_separator(&path[length - 1]);
}

int32_t
not_path_is_absolute(const char *path)
{
    size_t length;
    not_path_get_root(path, &length);
    return not_path_is_root_absolute(path, length);
}

int32_t
not_path_is_relative(const char *path)
{
    return !not_path_is_absolute(path);
}

static not_path_segment_type_t
not_path_get_segment_type(const not_path_segment_t *segment)
{
    if (strncmp(segment->begin, ".", segment->size) == 0)
    {
        return PATH_CURRENT;
    }
    else if (strncmp(segment->begin, "..", segment->size) == 0)
    {
        return PATH_BACK;
    }
    return PATH_NORMAL;
}

static int32_t
not_path_get_next_segment(not_path_segment_t *segment)
{
    const char *c;

    c = segment->begin + segment->size;
    if (*c == '\0')
    {
        return 0;
    }

    assert(not_path_is_separator(c));
    do
    {
        ++c;
    } while (not_path_is_separator(c));

    if (*c == '\0')
    {
        return 0;
    }

    segment->begin = c;

    c = not_path_find_next_stop(c);
    segment->end = c;
    segment->size = (size_t)(c - segment->begin);

    return 1;
}

static int32_t
not_path_get_previous_segment(not_path_segment_t *segment)
{
    const char *c;

    c = segment->begin;
    if (c <= segment->segments)
    {
        return 0;
    }

    do
    {
        --c;
        if (c < segment->segments)
        {
            return 0;
        }
    } while (not_path_is_separator(c));

    segment->end = c + 1;
    segment->begin = not_path_find_previous_stop(segment->segments, c);
    segment->size = (size_t)(segment->end - segment->begin);

    return 1;
}

static int32_t
not_path_get_first_segment_without_root(const char *path, const char *segments, not_path_segment_t *segment)
{
    segment->path = path;
    segment->segments = segments;
    segment->begin = segments;
    segment->end = segments;
    segment->size = 0;

    if (*segments == '\0')
    {
        return 0;
    }

    while (not_path_is_separator(segments))
    {
        ++segments;
        if (*segments == '\0')
        {
            return 0;
        }
    }

    segment->begin = segments;

    segments = not_path_find_next_stop(segments);

    segment->size = (size_t)(segments - segment->begin);
    segment->end = segments;

    return 1;
}

static int32_t
not_path_get_first_segment(const char *path, not_path_segment_t *segment)
{
    size_t length;
    const char *segments;

    not_path_get_root(path, &length);
    segments = path + length;

    return not_path_get_first_segment_without_root(path, segments, segment);
}

static int32_t
not_path_get_last_segment(const char *path, not_path_segment_t *segment)
{
    if (!not_path_get_first_segment(path, segment))
    {
        return 0;
    }

    while (not_path_get_next_segment(segment))
    {
    }

    return 1;
}

static size_t
not_path_output_sized(char *buffer, size_t buffer_size, size_t position, const char *str, size_t length)
{
    size_t amount_written;

    if (buffer_size > position + length)
    {
        amount_written = length;
    }
    else if (buffer_size > position)
    {
        amount_written = buffer_size - position;
    }
    else
    {
        amount_written = 0;
    }

    if (amount_written > 0)
    {
        memcpy(&buffer[position], str, amount_written);
    }

    return length;
}

static void
not_path_fix_root(char *buffer, size_t buffer_size, size_t length)
{
    if (path_style != PATH_STYLE_WINDOWS)
    {
        return;
    }

    if (length > buffer_size)
    {
        length = buffer_size;
    }

    size_t i;
    for (i = 0; i < length; ++i)
    {
        if (not_path_is_separator(&buffer[i]))
        {
            buffer[i] = windows_separator;
        }
    }
}

static int32_t
not_path_get_first_segment_joined(const char **paths, not_path_segment_joined_t *sj)
{
    sj->path_index = 0;
    sj->paths = paths;

    int32_t result;
    result = 0;
    while (paths[sj->path_index] != NOT_PTR_NULL && (result = not_path_get_first_segment(paths[sj->path_index], &sj->segment)) == 0)
    {
        ++sj->path_index;
    }

    return result;
}

static int32_t
not_path_get_last_segment_without_root(const char *path, not_path_segment_t *segment)
{
    if (!not_path_get_first_segment_without_root(path, path, segment))
    {
        return 0;
    }

    while (not_path_get_next_segment(segment))
    {
    }

    return 1;
}

static int32_t
not_path_get_previous_segment_joined(not_path_segment_joined_t *sj)
{
    if (*sj->paths == NOT_PTR_NULL)
    {
        return 0;
    }
    else if (not_path_get_previous_segment(&sj->segment))
    {
        return 1;
    }

    int32_t result = 0;

    do
    {
        if (sj->path_index == 0)
        {
            break;
        }
        --sj->path_index;
        if (sj->path_index == 0)
        {
            result = not_path_get_last_segment(sj->paths[sj->path_index], &sj->segment);
        }
        else
        {
            result = not_path_get_last_segment_without_root(sj->paths[sj->path_index], &sj->segment);
        }
    } while (!result);

    return result;
}

static int32_t
not_path_segment_back_will_be_removed(not_path_segment_joined_t *sj)
{
    int counter = 0;
    not_path_segment_type_t type;
    while (not_path_get_previous_segment_joined(sj))
    {
        type = not_path_get_segment_type(&sj->segment);
        if (type == PATH_NORMAL)
        {
            ++counter;
            if (counter > 0)
            {
                return 1;
            }
        }
        else if (type == PATH_BACK)
        {
            --counter;
        }
    }
    return 0;
}

static int32_t
not_path_get_next_segment_joined(not_path_segment_joined_t *sj)
{
    if (sj->paths[sj->path_index] == NOT_PTR_NULL)
    {
        return 0;
    }
    else if (not_path_get_next_segment(&sj->segment))
    {
        return 1;
    }

    int32_t result = false;
    do
    {
        ++sj->path_index;
        if (sj->paths[sj->path_index] == NOT_PTR_NULL)
        {
            break;
        }
        result = not_path_get_first_segment_without_root(sj->paths[sj->path_index],
                                                         sj->paths[sj->path_index], &sj->segment);
    } while (!result);

    return result;
}

static int32_t
not_path_segment_normal_will_be_removed(not_path_segment_joined_t *sj)
{
    not_path_segment_type_t type;
    int counter = 0;
    while (not_path_get_next_segment_joined(sj))
    {
        type = not_path_get_segment_type(&sj->segment);
        if (type == PATH_NORMAL)
        {
            ++counter;
        }
        else if (type == PATH_BACK)
        {
            --counter;
            if (counter < 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

static int32_t
not_path_segment_will_be_removed(const not_path_segment_joined_t *sj, int32_t absolute)
{
    not_path_segment_type_t type;
    not_path_segment_joined_t sjc;

    sjc = *sj;

    type = not_path_get_segment_type(&sj->segment);
    if (type == PATH_CURRENT || (type == PATH_BACK && absolute))
    {
        return 1;
    }
    else if (type == PATH_BACK)
    {
        return not_path_segment_back_will_be_removed(&sjc);
    }
    else
    {
        return not_path_segment_normal_will_be_removed(&sjc);
    }
}

static size_t
not_path_output_separator(char *buffer, size_t buffer_size, size_t position)
{
    if (path_style == PATH_STYLE_WINDOWS)
    {
        return not_path_output_sized(buffer, buffer_size, position, "\\", 1);
    }
    return not_path_output_sized(buffer, buffer_size, position, "/", 1);
}

static size_t
not_path_output_current(char *buffer, size_t buffer_size, size_t position)
{
    return not_path_output_sized(buffer, buffer_size, position, ".", 1);
}

static void
not_path_terminate_output(char *buffer, size_t buffer_size, size_t pos)
{
    if (buffer_size > 0)
    {
        if (pos >= buffer_size)
        {
            buffer[buffer_size - 1] = '\0';
        }
        else
        {
            buffer[pos] = '\0';
        }
    }
}

static size_t
not_path_join_and_normalize_multiple(const char **paths, char *buffer, size_t buffer_size)
{
    size_t pos;
    not_path_segment_joined_t sj;

    not_path_get_root(paths[0], &pos);

    int32_t absolute;
    absolute = not_path_is_root_absolute(paths[0], pos);

    not_path_output_sized(buffer, buffer_size, 0, paths[0], pos);
    not_path_fix_root(buffer, buffer_size, pos);

    if (!not_path_get_first_segment_joined(paths, &sj))
    {
        goto done;
    }

    int32_t has_segment_output = 0;

    do
    {
        if (not_path_segment_will_be_removed(&sj, absolute))
        {
            continue;
        }

        if (has_segment_output)
        {
            pos += not_path_output_separator(buffer, buffer_size, pos);
        }

        has_segment_output = true;

        pos += not_path_output_sized(buffer, buffer_size, pos, sj.segment.begin, sj.segment.size);
    } while (not_path_get_next_segment_joined(&sj));

    if (!has_segment_output && pos == 0)
    {
        assert(absolute == false);
        pos += not_path_output_current(buffer, buffer_size, pos);
    }

done:
    not_path_terminate_output(buffer, buffer_size, pos);
    return pos;
}

size_t
not_path_normalize(const char *path, char *buffer, size_t buffer_size)
{
    const char *paths[2];

    paths[0] = path;
    paths[1] = NOT_PTR_NULL;

    return not_path_join_and_normalize_multiple(paths, buffer, buffer_size);
}

size_t
not_path_join(const char *path_a, const char *path_b, char *buffer, size_t buffer_size)
{
    const char *paths[3];

    paths[0] = path_a;
    paths[1] = path_b;
    paths[2] = NOT_PTR_NULL;

    return not_path_join_and_normalize_multiple(paths, buffer, buffer_size);
}

size_t
not_path_join_multiple(const char **paths, char *buffer, size_t buffer_size)
{
    return not_path_join_and_normalize_multiple(paths, buffer, buffer_size);
}

size_t
not_path_get_absolute(const char *base, const char *path, char *buffer, size_t buffer_size)
{
    size_t i;
    const char *paths[4];

    if (not_path_is_absolute(base))
    {
        i = 0;
    }
    else if (path_style == PATH_STYLE_WINDOWS)
    {
        paths[0] = "\\";
        i = 1;
    }
    else
    {
        paths[0] = "/";
        i = 1;
    }

    if (not_path_is_absolute(path))
    {
        paths[i++] = path;
        paths[i] = NOT_PTR_NULL;
    }
    else
    {
        paths[i++] = base;
        paths[i++] = path;
        paths[i] = NOT_PTR_NULL;
    }

    return not_path_join_and_normalize_multiple(paths, buffer, buffer_size);
}

static int32_t
not_path_segment_joined_skip_invisible(not_path_segment_joined_t *sj, int32_t absolute)
{
    while (not_path_segment_will_be_removed(sj, absolute))
    {
        if (!not_path_get_next_segment_joined(sj))
        {
            return 0;
        }
    }

    return 1;
}

static int32_t
not_path_is_string_equal(const char *first, const char *second, size_t first_size, size_t second_size)
{
    if (first_size != second_size)
    {
        return 0;
    }

    if (path_style == PATH_STYLE_UNIX)
    {
        return strncmp(first, second, first_size) == 0;
    }

    int32_t are_both_separators;
    while (*first && *second && first_size > 0)
    {
        if (path_style == PATH_STYLE_WINDOWS)
        {
            are_both_separators = strchr("\\/", *first) != NOT_PTR_NULL && strchr("\\/", *second) != NOT_PTR_NULL;
        }
        else
        {
            are_both_separators = strchr("/", *first) != NOT_PTR_NULL && strchr("/", *second) != NOT_PTR_NULL;
        }

        if (tolower(*first) != tolower(*second) && !are_both_separators)
        {
            return 0;
        }

        first++;
        second++;

        --first_size;
    }

    return 1;
}

static void
not_path_skip_segments_until_diverge(not_path_segment_joined_t *bsj, not_path_segment_joined_t *osj, int32_t absolute, int32_t *base_available, int32_t *other_available)
{
    do
    {
        *base_available = not_path_segment_joined_skip_invisible(bsj, absolute);
        *other_available = not_path_segment_joined_skip_invisible(osj, absolute);

        if (!*base_available || !*other_available)
        {
            break;
        }

        if (!not_path_is_string_equal(bsj->segment.begin, osj->segment.begin,
                                      bsj->segment.size, osj->segment.size))
        {
            break;
        }

        *base_available = not_path_get_next_segment_joined(bsj);
        *other_available = not_path_get_next_segment_joined(osj);
    } while (*base_available && *other_available);
}

static size_t
not_path_output_back(char *buffer, size_t buffer_size, size_t position)
{
    return not_path_output_sized(buffer, buffer_size, position, "..", 2);
}

size_t
not_path_get_relative(const char *base_directory, const char *path, char *buffer, size_t buffer_size)
{
    size_t pos = 0;

    size_t base_root_length, path_root_length;
    not_path_get_root(base_directory, &base_root_length);
    not_path_get_root(path, &path_root_length);
    if (base_root_length != path_root_length || !not_path_is_string_equal(base_directory, path, base_root_length, path_root_length))
    {
        not_path_terminate_output(buffer, buffer_size, pos);
        return pos;
    }

    int32_t absolute;
    absolute = not_path_is_root_absolute(base_directory, base_root_length);

    const char *base_paths[2], *other_paths[2];
    not_path_segment_joined_t bsj, osj;

    base_paths[0] = base_directory;
    base_paths[1] = NOT_PTR_NULL;
    other_paths[0] = path;
    other_paths[1] = NOT_PTR_NULL;
    not_path_get_first_segment_joined(base_paths, &bsj);
    not_path_get_first_segment_joined(other_paths, &osj);

    int32_t base_available, other_available;
    not_path_skip_segments_until_diverge(&bsj, &osj, absolute, &base_available, &other_available);

    int32_t has_output = 0;

    if (base_available)
    {
        do
        {
            if (!not_path_segment_joined_skip_invisible(&bsj, absolute))
            {
                break;
            }
            has_output = 1;
            pos += not_path_output_back(buffer, buffer_size, pos);
            pos += not_path_output_separator(buffer, buffer_size, pos);
        } while (not_path_get_next_segment_joined(&bsj));
    }

    if (other_available)
    {
        do
        {
            if (!not_path_segment_joined_skip_invisible(&osj, absolute))
            {
                break;
            }
            has_output = 1;
            pos += not_path_output_sized(buffer, buffer_size, pos, osj.segment.begin, osj.segment.size);
            pos += not_path_output_separator(buffer, buffer_size, pos);
        } while (not_path_get_next_segment_joined(&osj));
    }

    if (has_output)
    {
        --pos;
    }
    else
    {
        pos += not_path_output_current(buffer, buffer_size, pos);
    }

    not_path_terminate_output(buffer, buffer_size, pos);

    return pos;
}

char *
not_path_get_current_directory(char *buffer, size_t buffer_size)
{
    return getcwd(buffer, buffer_size);
}

int32_t
not_path_is_root(const char *path)
{
    if (!path)
    {
        return 0;
    }
    if (path[0] == '~' && path[1] == '/')
    {
        return 1;
    }
    return 0;
}

size_t
not_path_get_filename(const char *path, char *buffer, size_t buffer_size)
{
    size_t length;
    const char *c, *d;
    c = strrchr(path, windows_separator);
    d = strrchr(path, unix_separator);
    if (!!c && !!d)
    {
        if ((size_t)c > (size_t)d)
        {
            length = strlen(c + 1);
            not_path_output_sized(buffer, buffer_size, 0, c + 1, length);
            not_path_terminate_output(buffer, buffer_size, length);
            return length;
        }
        else
        {
            length = strlen(d + 1);
            not_path_output_sized(buffer, buffer_size, 0, d + 1, length);
            not_path_terminate_output(buffer, buffer_size, length);
            return length;
        }
    }
    else if (!!c)
    {
        length = strlen(c + 1);
        not_path_output_sized(buffer, buffer_size, 0, c + 1, length);
        not_path_terminate_output(buffer, buffer_size, length);
        return length;
    }
    else if (!!d)
    {
        length = strlen(d + 1);
        not_path_output_sized(buffer, buffer_size, 0, d + 1, length);
        not_path_terminate_output(buffer, buffer_size, length);
        return length;
    }
    else
    {
        length = strlen(path);
        not_path_output_sized(buffer, buffer_size, 0, path, length);
        not_path_terminate_output(buffer, buffer_size, length);
        return length;
    }
}

size_t
not_path_get_directory_path(const char *path, char *buffer, size_t buffer_size)
{
    size_t length = 0;
    if (not_path_is_directory(path))
    {
        length = strlen(path);
        not_path_output_sized(buffer, buffer_size, 0, path, length);
        not_path_terminate_output(buffer, buffer_size, length);
        return length;
    }

    if (strchr(path, '.'))
    {
        const char *c, *d;
        c = strrchr(path, windows_separator);
        d = strrchr(path, unix_separator);
        if (!!c && !!d)
        {
            if ((size_t)c > (size_t)d)
            {
                length = (size_t)((c + 1) - path);
            }
            else
            {
                length = (size_t)((d + 1) - path);
            }
        }
        else if (!!c)
        {
            length = (size_t)((c + 1) - path);
        }
        else if (!!d)
        {
            length = (size_t)((d + 1) - path);
        }
        else
        {
            not_path_terminate_output(buffer, buffer_size, 0);
            return 0;
        }
    }

    not_path_output_sized(buffer, buffer_size, 0, path, length);
    not_path_terminate_output(buffer, buffer_size, length);
    return length;
}

int32_t
not_path_exist(const char *path)
{
    if (access(path, F_OK) == 0)
    {
        return 1;
    }
    return 0;
}