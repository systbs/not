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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
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

typedef struct SyPath_Segment {
    const char *path;
    const char *segments;
    const char *begin;
    const char *end;
    size_t size;
} SyPath_Segment_t;

typedef enum SyPath_SegmentType
{
    PATH_NORMAL,
    PATH_CURRENT,
    PATH_BACK
} SyPath_SegmentType_t;

typedef struct SyPath_SegmentJoined
{
    SyPath_Segment_t segment;
    const char **paths;
    size_t path_index;
} SyPath_SegmentJoined_t;

int32_t 
SyPath_IsDirectory(const char *path)
{
    struct stat st;

    if (stat(path, &st) < 0)
        return -1;

    return S_ISDIR(st.st_mode);
}

static int32_t
SyPath_IsSeparator(const char *path)
{
    if((*path == windows_separator) || (*path == unix_separator))
    {
        return 1;
    }
    return 0;
}

static const char *
SyPath_FindNextStop(const char *path)
{
    while (*path != '\0' && !SyPath_IsSeparator(path)) {
        ++path;
    }

    return path;
}

static const char *
SyPath_FindPreviousStop(const char *begin, const char *c)
{
    while (c > begin && !SyPath_IsSeparator(c))
    {
        --c;
    }

    if (SyPath_IsSeparator(c))
    {
        return c + 1;
    }
    else
    {
        return c;
    }
}

static void
SyPath_GetRoot(const char *path, size_t *length)
{
    const char *c;
    c = path;
    *length = 0;
    if(!c)
    {
        return;
    }

    // We have to verify whether this is a network path
    if(SyPath_IsSeparator(c))
    {
        ++c;
        if(!SyPath_IsSeparator(c))
        {
            // Okey, this is not a network path
            *length = 1;
            return;
        }

        // A device path is a path which starts with "\\." or "\\?"
        ++c;
        if((*c == '.' || *c == '?') && SyPath_IsSeparator(++c))
        {
            *length = 4;
            return;
        }

        // We will grab anything up to next stop, that will be the server name
        c = SyPath_FindNextStop(c);

        while(SyPath_IsSeparator(c))
        {
            if(*c == '\0')
            {
                *length = (size_t)(c - path);
                return;
            }
            ++c;
        }

        // We are now skipping the shared folder name
        c = SyPath_FindNextStop(c);

        if(SyPath_IsSeparator(c))
        {
            ++c;
        }

        *length = (size_t)(c - path);
        return;
    }

    if(*++c == ':')
    {
        *length = 2;

        if(SyPath_IsSeparator(++c))
        {
            *length = 3;
        }
    }
}

static int32_t 
SyPath_IsRootAbsolute(const char *path, size_t length)
{
  // This is definitely not absolute if there is no root.
  if (length == 0) {
    return 0;
  }

  // If there is a separator at the end of the root, we can safely consider this
  // to be an absolute path.
  return SyPath_IsSeparator(&path[length - 1]);
}

int32_t
SyPath_IsAbsolute(const char *path)
{
    size_t length;
    SyPath_GetRoot(path, &length);
    return SyPath_IsRootAbsolute(path, length);
}

int32_t
SyPath_IsRelative(const char *path)
{
    return !SyPath_IsAbsolute(path);
}

static SyPath_SegmentType_t
SyPath_GetSegmentType(const SyPath_Segment_t *segment)
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
SyPath_GetNextSegment(SyPath_Segment_t *segment)
{
    const char *c;

    c = segment->begin + segment->size;
    if (*c == '\0')
    {
        return 0;
    }

    assert(SyPath_IsSeparator(c));
    do
    {
        ++c;
    }
    while (SyPath_IsSeparator(c));

    if (*c == '\0')
    {
        return 0;
    }

    segment->begin = c;

    c = SyPath_FindNextStop(c);
    segment->end = c;
    segment->size = (size_t)(c - segment->begin);

    return 1;
}

static int32_t 
SyPath_GetPreviousSegment(SyPath_Segment_t *segment)
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
    }
    while (SyPath_IsSeparator(c));

    segment->end = c + 1;
    segment->begin = SyPath_FindPreviousStop(segment->segments, c);
    segment->size = (size_t)(segment->end - segment->begin);

    return 1;
}

static int32_t 
SyPath_GetFirstSegmentWithoutRoot(const char *path, const char *segments, SyPath_Segment_t *segment)
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

    while (SyPath_IsSeparator(segments))
    {
        ++segments;
        if (*segments == '\0')
        {
            return 0;
        }
    }

    segment->begin = segments;

    segments = SyPath_FindNextStop(segments);

    segment->size = (size_t)(segments - segment->begin);
    segment->end = segments;

    return 1;
}

static int32_t 
SyPath_GetFirstSegment(const char *path, SyPath_Segment_t *segment)
{
    size_t length;
    const char *segments;

    SyPath_GetRoot(path, &length);
    segments = path + length;

    return SyPath_GetFirstSegmentWithoutRoot(path, segments, segment);
}

static int32_t 
SyPath_GetLastSegment(const char *path, SyPath_Segment_t *segment)
{
    if (!SyPath_GetFirstSegment(path, segment))
    {
        return 0;
    }

    while (SyPath_GetNextSegment(segment))
    {
    }

    return 1;
}

static size_t 
SyPath_OutputSized(char *buffer, size_t buffer_size, size_t position, const char *str, size_t length)
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
SyPath_FixRoot(char *buffer, size_t buffer_size, size_t length)
{
    if (path_style != PATH_STYLE_WINDOWS) {
        return;
    }

    if (length > buffer_size)
    {
        length = buffer_size;
    }

    size_t i;
    for (i = 0; i < length; ++i)
    {
        if (SyPath_IsSeparator(&buffer[i]))
        {
            buffer[i] = windows_separator;
        }
    }
}

static int32_t 
SyPath_GetFirstSegmentJoined(const char **paths, SyPath_SegmentJoined_t *sj)
{
    sj->path_index = 0;
    sj->paths = paths;

    int32_t result;
    result = 0;
    while (paths[sj->path_index] != NULL && (result = SyPath_GetFirstSegment(paths[sj->path_index], &sj->segment)) == 0)
    {
        ++sj->path_index;
    }

    return result;
}

static int32_t 
SyPath_GetLastSegmentWithoutRoot(const char *path, SyPath_Segment_t *segment)
{
    if (!SyPath_GetFirstSegmentWithoutRoot(path, path, segment))
    {
        return 0;
    }

    while (SyPath_GetNextSegment(segment))
    {
    }

    return 1;
}

static int32_t 
SyPath_GetPreviousSegmentJoined(SyPath_SegmentJoined_t *sj)
{
    if (*sj->paths == NULL)
    {
        return 0;
    }
    else if (SyPath_GetPreviousSegment(&sj->segment))
    {
        return 1;
    }

    int32_t result = 0;

    do {
        if (sj->path_index == 0)
        {
            break;
        }
        --sj->path_index;
        if (sj->path_index == 0)
        {
            result = SyPath_GetLastSegment(sj->paths[sj->path_index],&sj->segment);
        }
        else
        {
            result = SyPath_GetLastSegmentWithoutRoot(sj->paths[sj->path_index],&sj->segment);
        }
    }
    while (!result);

    return result;
}

static int32_t 
SyPath_SegmentBackWillBeRemoved(SyPath_SegmentJoined_t *sj)
{
    int counter = 0;
    SyPath_SegmentType_t type;
    while (SyPath_GetPreviousSegmentJoined(sj))
    {
        type = SyPath_GetSegmentType(&sj->segment);
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
SyPath_GetNextSegmentJoined(SyPath_SegmentJoined_t *sj)
{
    if (sj->paths[sj->path_index] == NULL)
    {
        return 0;
    }
    else if (SyPath_GetNextSegment(&sj->segment))
    {
        return 1;
    }

    int32_t result = false;
    do
    {
        ++sj->path_index;
        if (sj->paths[sj->path_index] == NULL)
        {
            break;
        }
        result = SyPath_GetFirstSegmentWithoutRoot(sj->paths[sj->path_index],
        sj->paths[sj->path_index], &sj->segment);
    }
    while (!result);

    return result;
}

static int32_t 
SyPath_SegmentNormalWillBeRemoved(SyPath_SegmentJoined_t *sj)
{
    SyPath_SegmentType_t type;
    int counter = 0;
    while (SyPath_GetNextSegmentJoined(sj))
    {
        type = SyPath_GetSegmentType(&sj->segment);
        if (type == PATH_NORMAL)
        {
            ++counter;
        }
        else if (type == PATH_BACK)
        {
            --counter;
            if (counter < 0) {
                return 1;
            }
        }
    }
    return 0;
}

static int32_t
SyPath_SegmentWillBeRemoved(const SyPath_SegmentJoined_t *sj, int32_t absolute)
{
    SyPath_SegmentType_t type;
    SyPath_SegmentJoined_t sjc;

    sjc = *sj;

    type = SyPath_GetSegmentType(&sj->segment);
    if (type == PATH_CURRENT || (type == PATH_BACK && absolute))
    {
        return 1;
    }
    else if (type == PATH_BACK)
    {
        return SyPath_SegmentBackWillBeRemoved(&sjc);
    }
    else
    {
        return SyPath_SegmentNormalWillBeRemoved(&sjc);
    }
}

static size_t 
SyPath_OutputSeparator(char *buffer, size_t buffer_size, size_t position)
{
    if(path_style == PATH_STYLE_WINDOWS)
    {
        return SyPath_OutputSized(buffer, buffer_size, position, "\\", 1);
    }
    return SyPath_OutputSized(buffer, buffer_size, position, "/", 1);
}

static size_t 
SyPath_OutputCurrent(char *buffer, size_t buffer_size, size_t position)
{
  return SyPath_OutputSized(buffer, buffer_size, position, ".", 1);
}

static void 
SyPath_TerminateOutput(char *buffer, size_t buffer_size, size_t pos)
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
SyPath_JoinAndNormalizeMultiple(const char **paths, char *buffer, size_t buffer_size)
{
    size_t pos;
    SyPath_SegmentJoined_t sj;

    SyPath_GetRoot(paths[0], &pos);

    int32_t absolute;
    absolute = SyPath_IsRootAbsolute(paths[0], pos);

    SyPath_OutputSized(buffer, buffer_size, 0, paths[0], pos);
    SyPath_FixRoot(buffer, buffer_size, pos);

    if (!SyPath_GetFirstSegmentJoined(paths, &sj))
    {
        goto done;
    }

    int32_t has_segment_output = 0;

    do
    {
        if (SyPath_SegmentWillBeRemoved(&sj, absolute))
        {
            continue;
        }

        if (has_segment_output)
        {
            pos += SyPath_OutputSeparator(buffer, buffer_size, pos);
        }

        has_segment_output = true;

        pos += SyPath_OutputSized(buffer, buffer_size, pos, sj.segment.begin, sj.segment.size);
    } 
    while (SyPath_GetNextSegmentJoined(&sj));

    if (!has_segment_output && pos == 0)
    {
        assert(absolute == false);
        pos += SyPath_OutputCurrent(buffer, buffer_size, pos);
    }

done:
    SyPath_TerminateOutput(buffer, buffer_size, pos);
    return pos;
}

size_t
SyPath_Normalize(const char *path, char *buffer, size_t buffer_size)
{
    const char *paths[2];

    paths[0] = path;
    paths[1] = NULL;

    return SyPath_JoinAndNormalizeMultiple(paths, buffer, buffer_size);
}

size_t
SyPath_Join(const char *path_a, const char *path_b, char *buffer, size_t buffer_size)
{
    const char *paths[3];

    paths[0] = path_a;
    paths[1] = path_b;
    paths[2] = NULL;

    return SyPath_JoinAndNormalizeMultiple(paths, buffer, buffer_size);
}

size_t
SyPath_JoinMultiple(const char **paths, char *buffer, size_t buffer_size)
{
    return SyPath_JoinAndNormalizeMultiple(paths, buffer, buffer_size);
}

size_t 
SyPath_GetAbsolute(const char *base, const char *path, char *buffer, size_t buffer_size)
{
    size_t i;
    const char *paths[4];

    if (SyPath_IsAbsolute(base)) {
        i = 0;
    } else if (path_style == PATH_STYLE_WINDOWS) {
        paths[0] = "\\";
        i = 1;
    } else {
        paths[0] = "/";
        i = 1;
    }

    if (SyPath_IsAbsolute(path)) {
        paths[i++] = path;
        paths[i] = NULL;
    } else {
        paths[i++] = base;
        paths[i++] = path;
        paths[i] = NULL;
    }

    return SyPath_JoinAndNormalizeMultiple(paths, buffer, buffer_size);
}

static int32_t
SyPath_SegmentJoinedSkipInvisible(SyPath_SegmentJoined_t *sj, int32_t absolute)
{
    while (SyPath_SegmentWillBeRemoved(sj, absolute))
    {
        if (!SyPath_GetNextSegmentJoined(sj))
        {
            return 0;
        }
    }

    return 1;
}

static int32_t 
SyPath_IsStringEqual(const char *first, const char *second, size_t first_size, size_t second_size)
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
        if(path_style == PATH_STYLE_WINDOWS)
        {
            are_both_separators = strchr("\\/", *first) != NULL && strchr("\\/", *second) != NULL;
        } 
        else
        {
            are_both_separators = strchr("/", *first) != NULL && strchr("/", *second) != NULL;
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
SyPath_SkipSegmentsUntilDiverge(SyPath_SegmentJoined_t *bsj, SyPath_SegmentJoined_t *osj, int32_t absolute, int32_t *base_available, int32_t *other_available)
{
    do {
        *base_available = SyPath_SegmentJoinedSkipInvisible(bsj, absolute);
        *other_available = SyPath_SegmentJoinedSkipInvisible(osj, absolute);

        if (!*base_available || !*other_available) {
        break;
        }

        if (!SyPath_IsStringEqual(bsj->segment.begin, osj->segment.begin,
            bsj->segment.size, osj->segment.size)) {
        break;
        }

        *base_available = SyPath_GetNextSegmentJoined(bsj);
        *other_available = SyPath_GetNextSegmentJoined(osj);
    } while (*base_available && *other_available);
}

static size_t 
SyPath_OutputBack(char *buffer, size_t buffer_size, size_t position)
{
    return SyPath_OutputSized(buffer, buffer_size, position, "..", 2);
}

size_t 
SyPath_GetRelative(const char *base_directory, const char *path, char *buffer, size_t buffer_size)
{
    size_t pos = 0;

    size_t base_root_length, path_root_length;
    SyPath_GetRoot(base_directory, &base_root_length);
    SyPath_GetRoot(path, &path_root_length);
    if (base_root_length != path_root_length || !SyPath_IsStringEqual(base_directory, path, base_root_length, path_root_length))
    {
        SyPath_TerminateOutput(buffer, buffer_size, pos);
        return pos;
    }

    int32_t absolute;
    absolute = SyPath_IsRootAbsolute(base_directory, base_root_length);

    const char *base_paths[2], *other_paths[2];
    SyPath_SegmentJoined_t bsj, osj;

    base_paths[0] = base_directory;
    base_paths[1] = NULL;
    other_paths[0] = path;
    other_paths[1] = NULL;
    SyPath_GetFirstSegmentJoined(base_paths, &bsj);
    SyPath_GetFirstSegmentJoined(other_paths, &osj);

    int32_t base_available, other_available;
    SyPath_SkipSegmentsUntilDiverge(&bsj, &osj, absolute, &base_available, &other_available);

    int32_t has_output = 0;

    if (base_available)
    {
        do
        {
            if (!SyPath_SegmentJoinedSkipInvisible(&bsj, absolute))
            {
                break;
            }
            has_output = 1;
            pos += SyPath_OutputBack(buffer, buffer_size, pos);
            pos += SyPath_OutputSeparator(buffer, buffer_size, pos);
        }
        while (SyPath_GetNextSegmentJoined(&bsj));
    }

    if (other_available)
    {
        do
        {
            if (!SyPath_SegmentJoinedSkipInvisible(&osj, absolute))
            {
                break;
            }
            has_output = 1;
            pos += SyPath_OutputSized(buffer, buffer_size, pos, osj.segment.begin, osj.segment.size);
            pos += SyPath_OutputSeparator(buffer, buffer_size, pos);
        }
        while (SyPath_GetNextSegmentJoined(&osj));
    }

    if (has_output)
    {
        --pos;
    }
    else
    {
        pos += SyPath_OutputCurrent(buffer, buffer_size, pos);
    }

    SyPath_TerminateOutput(buffer, buffer_size, pos);

    return pos;
}

char *
SyPath_GetCurrentDirectory(char *buffer, size_t buffer_size)
{
    return getcwd(buffer, buffer_size);
}

int32_t
SyPath_IsRoot(const char *path)
{
    if(!path)
    {
        return 0;
    }
    if(path[0] == '~' && path[1] == '/')
    {
        return 1;
    }
    return 0;
}

size_t
SyPath_GetFilename(const char *path, char *buffer, size_t buffer_size)
{
    size_t length;
    const char *c, *d;
    c = strrchr(path, windows_separator);
    d = strrchr(path, unix_separator);
    if(!!c && !!d)
    {
        if((size_t)c > (size_t)d)
        {
            length = strlen(c + 1);
            SyPath_OutputSized(buffer, buffer_size, 0, c + 1, length);
            SyPath_TerminateOutput(buffer, buffer_size, length);
            return length;
        }
        else
        {
            length = strlen(d + 1);
            SyPath_OutputSized(buffer, buffer_size, 0, d + 1, length);
            SyPath_TerminateOutput(buffer, buffer_size, length);
            return length;
        }
    }
    else if (!!c)
    {
        length = strlen(c + 1);
        SyPath_OutputSized(buffer, buffer_size, 0, c + 1, length);
        SyPath_TerminateOutput(buffer, buffer_size, length);
        return length;
    }
    else if (!!d)
    {
        length = strlen(d + 1);
        SyPath_OutputSized(buffer, buffer_size, 0, d + 1, length);
        SyPath_TerminateOutput(buffer, buffer_size, length);
        return length;
    }
    else
    {
        length = strlen(path);
        SyPath_OutputSized(buffer, buffer_size, 0, path, length);
        SyPath_TerminateOutput(buffer, buffer_size, length);
        return length;
    }
}

size_t
SyPath_GetDirectoryPath(const char *path, char *buffer, size_t buffer_size)
{
    size_t length = 0;
    if(SyPath_IsDirectory(path))
    {
        length = strlen(path);
        SyPath_OutputSized(buffer, buffer_size, 0, path, length);
        SyPath_TerminateOutput(buffer, buffer_size, length);
        return length;
    }

    if(strchr(path, '.'))
    {
        const char *c, *d;
        c = strrchr(path, windows_separator);
        d = strrchr(path, unix_separator);
        if(!!c && !!d)
        {
            if((size_t)c > (size_t)d)
            {
                length = (size_t)((c + 1) - path);
            }
            else
            {
                length = (size_t)((d + 1) - path);
            }
        }
        else if(!!c)
        {
            length = (size_t)((c + 1) - path);
        }
        else if(!!d)
        {
            length = (size_t)((d + 1) - path);
        }
        else 
        {
            SyPath_TerminateOutput(buffer, buffer_size, 0);
            return 0;
        }
    }

    SyPath_OutputSized(buffer, buffer_size, 0, path, length);
    SyPath_TerminateOutput(buffer, buffer_size, length);
    return length;
}

int32_t
SyPath_Exist(const char *path)
{
    if (access(path, F_OK) == 0) 
    {
        return 1;
    }
    return 0;
}