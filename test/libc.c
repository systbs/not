// gcc -c -nostdlib -Wno-builtin-declaration-mismatch -o libc.o libc.c
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

static const char digits_lower[] = "0123456789abcdef";
static const char digits_upper[] = "0123456789ABCDEF";

//#pragma GCC diagnostic error "-Wno-builtin-declaration-mismatch"
extern void 
fputs(uint64_t fd, const char *str, uint64_t length);


uint64_t
strlen(const char *str)
{
  uint64_t n;
  for(n = 0; str[n]; n++);
  return n;
}

void*
memset(void *dst, int c, uint64_t n)
{
  char *cdst = (char *) dst;
  uint64_t i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

static void
fputc(uint64_t fd, char c)
{
  fputs(fd, &c, 1);
}

static void
printint(uint64_t fd, int64_t value, int32_t base, int32_t sgn, const char *digits)
{
	uint64_t x;

	int32_t neg = 0;
	if(sgn && value < 0)
	{
		neg = 1;
		x = -value;
	} 
	else 
	{
		x = value;
	}

	char buf[255];
	memset(buf, 0, 255);

	uint64_t i = 0;
	do {
		buf[i++] = digits[x % base];
	} while((x /= base) != 0);

	if (neg)
		buf[i++] = '-';

    uint64_t len = strlen(buf);
    for (uint64_t i = 0, j = len - 1; i <= j; i++, j--) {
        char c = buf[i];
        buf[i] = buf[j];
        buf[j] = c;
    }

	fputs(fd, buf, len);
}

static void 
printfloat(uint64_t fd, double value, int32_t precision, int32_t sgn, const char *digits)
{
	double x;

	int32_t neg = 0;
	if(sgn && value < 0)
	{
		neg = 1;
		x = -value;
	} 
	else 
	{
		x = value;
	}

	uint64_t dp = 1;
	for (uint64_t i = 0; i < precision; i++)
	{
		dp *= 10;
	}

	uint64_t dVal, dec;
    dVal = (uint64_t)x;
    dec = (uint64_t)(x * dp) % dp;

	char buf[255];
    memset(buf, 0, 255);

	uint64_t i = 0;
	do {
		buf[i++] = digits[dec % 10];
	} while((dec /= 10) != 0);
    buf[i++] = '.';

    while (dVal > 0)
    {
        buf[i] = digits[dVal % 10];
        dVal /= 10;
        i++;
    }

	if (neg)
		buf[i++] = '-';

    uint64_t len = strlen(buf);
    for (uint64_t i = 0, j = len - 1; i <= j; i++, j--) {
        char c = buf[i];
        buf[i] = buf[j];
        buf[j] = c;
    }

	fputs(fd, buf, len);
}

static void
printptr(uint64_t fd, uint64_t value, const char *digits) {
	uint64_t i;
	fputc(fd, '0');
	fputc(fd, 'x');
	for (i = 0; i < (sizeof(uint64_t) * 2); i++, value <<= 4)
		fputc(fd, digits[value >> (sizeof(uint64_t) * 8 - 4)]);
}

void
vfprintf(uint64_t fd, const char *format, va_list arg)
{
	char *s;

	uint64_t i;
	int32_t state = 0;
	for(i = 0; format[i] != '\0'; i++)
	{
		int32_t c = format[i] & 0xff;
		if(state == 0)
		{
			if(c == '%')
			{
				state = '%';
			} 
			else
			if(c == '\\')
			{
				state = '\\';
			} 
			else
			{
				fputc(fd, c);
			}
		}
		else 
		if(state == '%')
		{
			if(c == 'd')
			{
				printint(fd, va_arg(arg, int32_t), 10, 1, digits_lower);
			}
			else 
			if(c == 'i')
			{
				printint(fd, va_arg(arg, int32_t), 10, 1, digits_lower);
			}
			else
			if(c == 'o')
			{
				printint(fd, va_arg(arg, uint32_t), 8, 1, digits_lower);
			}
			else 
			if(c == 'x')
			{
				printint(fd, va_arg(arg, uint32_t), 16, 0, digits_lower);
			}
			else 
			if(c == 'X')
			{
				printint(fd, va_arg(arg, uint32_t), 16, 0, digits_upper);
			}
			else
			if(c == 'u')
			{
				printint(fd, va_arg(arg, uint32_t), 10, 0, digits_lower);
			}
			else
			if(c == 'f')
			{
				printfloat(fd, va_arg(arg, double), 6, 1, digits_lower);
			}
			else
			if(c == 'l')
			{
				printint(fd, va_arg(arg, uint64_t), 10, 0, digits_lower);
			}
			else 
			if(c == 'p')
			{
				printptr(fd, va_arg(arg, uint64_t), digits_upper);
			} 
			else 
			if(c == 's')
			{
				s = va_arg(arg, char*);
				if(s == 0)
					s = "(null)";
				while (*s != 0)
				{
					fputc(fd, *s);
					s++;
				}
			} 
			else 
			if(c == 'c')
			{
				fputc(fd, va_arg(arg, uint32_t));
			} 
			else 
			if(c == '%')
			{
				fputc(fd, c);
			} 
			else
			{
				// Unknown % sequence.  Print it to draw attention.
				fputc(fd, '%');
				fputc(fd, c);
			}
			state = 0;
		}
		else
		if(state == '\\')
		{
			if(c == 'n')
			{
				fputc(fd, '\n');
			}
			else
			if(c == 't')
			{
				fputc(fd, '\t');
			}
			else
			if(c == '\\')
			{
				fputc(fd, '\\');
			}
			else
			{
				fputc(fd, '\\');
				fputc(fd, c);
			}
			state = 0;
		}
	}
}

void
fprintf(uint64_t fd, const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vfprintf(fd, format, arg);
	va_end(arg);
}

void 
printf(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vfprintf(1, format, arg);
	va_end(arg);
}

void *
sy_node_make_class(void *parent, const char *name, uint64_t flag)
{
	printf(name);
	return parent;
}