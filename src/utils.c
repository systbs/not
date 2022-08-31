#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>
#include <stdarg.h>


#include "types.h"
#include "utils.h"
#include "list.h"
#include "lexer.h"
#include "memory.h"


void
utils_swap(char* a, char* b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

void
utils_reverse(char str[], int length)
{
  int start = 0;
  int end = length -1;
  while (start < end)
  {
    utils_swap((str+start), (str+end));
    start++;
    end--;
  }
}

char *
utils_itoa(int num, char* str, int base)
{
  int i = 0;
  bool_t isNegative = false;

  /* Handle 0 explicitely, otherwise empty string is printed for 0 */
  if (num == 0)
  {
    str[i++] = '0';
    str[i] = '\0';
    return str;
  }

  // In standard utils_itoa(), negative numbers are handled only with
  // base 10. Otherwise numbers are considered unsigned.
  if (num < 0 && base == 10)
  {
    isNegative = true;
    num = -num;
  }

  // Process individual digits
  while (num != 0)
  {
    int rem = num % base;
    str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
    num = num/base;
  }

  // If number is negative, append '-'
  if (isNegative)
    str[i++] = '-';

  str[i] = '\0'; // Append string terminator

  // utils_reverse the string
  utils_reverse(str, i);

  return str;
}

double
utils_pow(double a, double b)
{
  union {
    double d;
    int x[2];
  } u = { a };
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

int
utils_itos(int x, char str[], int d)
{
  int i = 0;
  while (x)
  {
    str[i++] = (x%10) + '0';
    x = x/10;
  }

  // If number of digits required is more, then
  // add 0s at the beginning
  while (i < d)
    str[i++] = '0';

  utils_reverse(str, i);
  str[i] = '\0';
  return i;
}

char *
utils_ftoa(double n, char *res, int afterpoint)
{
  // modulusct integer part
  int ipart = (int)n;

  // modulusct floating part
  double fpart = n - (double)ipart;

  // convert integer part to string
  int i = utils_itos(ipart, res, 0);

  // check for display option after point
  if (afterpoint != 0)
  {
    res[i] = '.';  // add dot

    // Get the value of fraction part upto given no.
    // of points after dot. The third parameter is needed
    // to handle cases like 233.007
    fpart = fpart * utils_pow(10, afterpoint);

    utils_itos((int)fpart, res + i + 1, afterpoint);
  }
  return res;
}

double
utils_atof (const char *p)
{
  int frac;
  double sign, value, scale;

  // Skip leading white space, if any.

  while (white_space(*p) ) {
    p += 1;
  }

  // Get sign, if any.

  sign = 1.0;
  if (*p == '-') {
    sign = -1.0;
    p += 1;

  } else if (*p == '+') {
    p += 1;
  }

  // Get digits before decimal point or exponent, if any.

  for (value = 0.0; valid_digit(*p); p += 1) {
    value = value * 10.0 + (*p - '0');
  }

  // Get digits after decimal point, if any.

  if (*p == '.') {
    double pow10 = 10.0;
    p += 1;
    while (valid_digit(*p)) {
      value += (*p - '0') / pow10;
      pow10 *= 10.0;
      p += 1;
    }
  }

  // Handle exponent, if any.

  frac = 0;
  scale = 1.0;
  if ((*p == 'e') || (*p == 'E')) {
    unsigned int expon;

    // Get sign of exponent, if any.

    p += 1;
    if (*p == '-') {
      frac = 1;
      p += 1;

    } else if (*p == '+') {
      p += 1;
    }

    // Get digits of exponent, if any.

    for (expon = 0; valid_digit(*p); p += 1) {
      expon = expon * 10 + (*p - '0');
    }
    if (expon > 308) expon = 308;

    // Calculate scaling factor.

    while (expon >= 50) { scale *= 1E50; expon -= 50; }
    while (expon >=  8) { scale *= 1E8;  expon -=  8; }
    while (expon >   0) { scale *= 10.0; expon -=  1; }
  }

  // Return signed and scaled floating point result.

  return sign * (frac ? (value / scale) : (value * scale));
}

int
utils_atoi(char *str)
{
  if (*str == '\0')
    return 0;

  int res = 0; // Initialize result
  int sign = 1; // Initialize sign as positive
  int i = 0; // Initialize index of first digit

  // If number is negative, then update sign
  if (str[0] == '-')
  {
    sign = -1;
    i++; // Also update index of first digit
  }

  // Iterate through all digits of input string and update result
  for (; str[i] != '\0'; ++i)
  {
    if (!valid_digit(str[i]))
      return 0; // You may add some lines to write error message
          // to error stream
    res = res*10 + str[i] - '0';
  }

  // Return result with sign
  return sign*res;
}


long_t
check_hexadecimal(char *str)
{
  long_t i;
  for(i = 0; i < strlen(str); i++){
    if(i == 1 && str[i] == 'x'){
      continue;
    }
    if(!valid_hexadecimal(str[i])){
      return 0;
    }
  }
  return 1;
}

long_t
check_double(char *str)
{
  long_t i, rep = 0;
  for(i = 0; i < strlen(str); i++){
    if(str[i] == '.'){
      rep++;
      if(rep > 1){
        return 0;
      }
      continue;
    }
    if(!valid_digit(str[i])){
      return 0;
    }
  }
  return 1;
}

long_t
check_integer(char *str)
{
  long_t i;
  for(i = 0; i < strlen(str); i++){
    if(!valid_digit(str[i])){
      return 0;
    }
  }
  return 1;
}

void
utils_combine(char* destination, const char* path1, const char* path2)
{
    if(path1 == NULL && path2 == NULL) {
        strcpy(destination, "");
    }
    else if(path2 == NULL || strlen(path2) == 0) {
        strcpy(destination, path1);
    }
    else if(path1 == NULL || strlen(path1) == 0) {
        strcpy(destination, path2);
    }
    else {
        char directory_separator[] = "/";
#ifdef WIN32
        directory_separator[0] = '\\';
#endif
        const char *last_char = path1;
        while(*last_char != '\0')
            last_char++;
        int append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}

void
get_random_bytes(char *bytes, int n)
{
    
}

char_t * 
uuid()
{
  int n = 16;

  char_t *bytes = (char_t *)qalam_malloc(sizeof(char_t) * (n + 1));

	char alphabet[16] = {'1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    
  // srand (time (0));
  for (int i = 0; i < n; i++){
    bytes[i] = alphabet[rand() % 16];
  }

  bytes[n] = '\0';

	/* Set UUID version to 4 --- truly random generation */
	bytes[6] = (bytes[6] & 0x0F) | 0x40;
	/* Set the UUID variant to DCE */
	bytes[8] = (bytes[8] & 0x3F) | 0x80;
  
  return bytes;
}


void
assert_format(int __cond__, char *__format__, ...){
	if(!__cond__){
		va_list args;

		va_start(args, __format__);
		vprintf(__format__, args);
		printf("\n");
		va_end(args);
		exit(-1);
	}
}
