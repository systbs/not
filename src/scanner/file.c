#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "file.h"

file_source_t *
file_create_source(char *path)
{
    if(!path)
    {
        return NULL;
    }

    file_source_t *file_source;
	file_source = (file_source_t *)malloc(sizeof(file_source_t));
	if(!file_source)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(file_source_t));
		return NULL;
	}
	memset(file_source, 0, sizeof(file_source_t));

    memset(file_source->path, 0, sizeof(file_source->path));
    strcpy(file_source->path, path);

    path_get_filename(file_source->path, file_source->name, sizeof(file_source->name));

    FILE *fd;

    if (!(fd = fopen(file_source->path, "rb")))
	{
        fprintf(stderr, "could not open(%s)\n", file_source->path);
        return NULL;
    }
    
    int64_t pos = ftell(fd);
    fseek(fd, 0, SEEK_END);
    int64_t chunk = ftell(fd);
    fseek(fd, pos, SEEK_SET);

    char *buf;
    if (!(buf = malloc(chunk + 1)))
	{
    	fprintf(stderr, "unable to allocted a block of %llu bytes", chunk);
        return NULL;
    }

	int64_t i;
    if ((i = fread(buf, 1, chunk, fd)) < chunk)
	{
        fprintf(stderr, "read returned %lld\n", i);
        return NULL;
    }

    buf[i] = '\0';

    fclose(fd);

    file_source->text = buf;
    
    return file_source;
}

void
file_destroy_source(file_source_t *file_source)
{
    free(file_source->text);
    free(file_source);
}