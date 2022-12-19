#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../types/types.h"

#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#include <direct.h>
#define F_OK 0
#define access _access
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#define DIR_SEPARATOR '/'

char * 
utils_replace_char(char* str, char find, char replace)
{
    char *current_pos = strchr(str,find);
    while (current_pos)
    {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

static int32_t
last_indexof(char *str, char x)
{
    int32_t index = -1;
    for (size_t i = 0; i < strlen(str); i++)
        if (str[i] == x)
            index = i;
    return index;
}

static int32_t
indexof(char *str, char x)
{
    int32_t index = -1;
    for (size_t i = 0; i < strlen(str); i++)
        if (str[i] == x){
            index = i;
            break;
        }
            
    return index;
}

