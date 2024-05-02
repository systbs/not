#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

double128_t
utils_stold(char *str)
{
    for (uint64_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '0')
        {
            i += 1;
            if (tolower(str[i]) == 'x')
            {
				i += 1;
				uint64_t number = 0;
				for (uint64_t j = i; j < strlen(str); j++)
    			{
					uint8_t value = ((str[j] & 0xF) + (str[j] >> 6)) | ((str[j] >> 3) & 0x8);
					number = (number << 4) | (uint64_t) value;
				}
                return (double128_t)number;
            }
            else
            if (tolower(str[i]) == 'b')
            {
				i += 1;
				uint64_t number = 0;
				for (uint64_t j = i; j < strlen(str); j++)
    			{
					uint8_t value = ((str[j] & 0x1) - '0');
					number = (number << 1) | (uint64_t) value;
				}
                return (double128_t)number;
            }
            else
            if (tolower(str[i]) == 'o')
            {
                i += 1;
				uint64_t number = 0;
				for (uint64_t j = i; j < strlen(str); j++)
    			{
					uint8_t value = (str[j] - '0');
					number = (number << 3) | (uint64_t) value;
				}
				return (double128_t)number;
            }
            else
            if (tolower(str[i]) == '.')
            {
				i += 1;
				double128_t number = 0, fact = 1;
				int32_t sign = 1;
				for (uint64_t j = i; j < strlen(str); j++)
    			{
					if (tolower(str[j]) == 'e')
					{
						j += 1;
						if (tolower(str[j]) == '+')
						{
							sign = 1;
							j += 1;
						}
						else
						if (tolower(str[j]) == '-')
						{
							sign = -1;
							j += 1;
						}
						uint64_t sym = 0;
						for (uint64_t k = j;k < strlen(str);k++)
    					{
							uint8_t value = (str[k] - '0');
							sym = (sym * 10) + (uint64_t) value;
						}
						for (uint64_t k = 0;k < sym;k++)
    					{
							if (sign == 1)
							{
								number *= 10;
							}
							else
							{
								number /= 10;
							}
						}
						break;
					}
					else
					{
						uint8_t value = (str[j] - '0');
						fact /= 10;
						number = (number *10 ) + (uint64_t) value;
					}
				}
				number = number * fact;
                return number;
            }
            else
            {
				i += 1;
				uint64_t number = 0;
				for (uint64_t j = i; j < strlen(str); j++)
    			{
					uint8_t value = (str[j] - '0');
					number = (number << 3) | (uint64_t) value;
				}
				return (double128_t)number;
            }
        }
        else
        {
			double number = 0, fact = 1;
			int32_t sign = 0, exp = 0;
			for (uint64_t j = i; j < strlen(str); j++)
			{
				if (tolower(str[j]) == 'e')
				{
					j += 1;
					if (tolower(str[j]) == '+')
					{
						sign = 1;
						j += 1;
					}
					else
					if (tolower(str[j]) == '-')
					{
						sign = -1;
						j += 1;
					}
					uint64_t sym = 0;
					for (uint64_t k = j;k < strlen(str);k++)
					{
						uint8_t value = (str[k] - '0');
						sym = (sym * 10) + (uint64_t) value;
					}
					for (uint64_t k = 0;k < (exp ? sym : sym + 1);k++)
					{
						if (sign == 1)
						{
							number *= 10;
						}
						else
						{
							number /= 10;
						}
					}
					break;
				}
				if (tolower(str[j]) == '.')
				{
					exp = 1;
					continue;
				}
				else
				{
					uint8_t value = (str[j] - '0');
					if (exp == 1)
					{
						fact /= 10;
					}
					number = (number * 10) + (uint64_t) value;
				}
			}
			number = number * fact;
			return (double128_t)number;
        }
    }
	return 0;
}

int32_t
utils_isinteger(double128_t value)
{
    uint64_t value2 = (uint64_t)value;
    if (value2 == value)
    {
        return 1;
    }
    return 0;
}

int32_t
utils_indexof(char *str, char x)
{
    int32_t index = -1;
    for (size_t i = 0; i < strlen(str); i++)
        if (str[i] == x){
            index = i;
            break;
        }
            
    return index;
}

int32_t
utils_issfloat(char *str)
{
	return utils_indexof(str, '.') >= 0;
}

/*
uint64_t 
utils_xtou64(const char *str)
{
    uint64_t res = 0;
    char c;

    while ((c = *str++)) {
        char v = ((c & 0xF) + (c >> 6) )| ((c >> 3) & 0x8);
        res = (res << 4) | (uint64_t) v;
    }

    return res;
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


*/

