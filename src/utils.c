#include <string.h>
#include <stdarg.h>

#include "types.h"
#include "utils.h"

void
utils_swap(char* a, char* b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

void
utils_reverse(char str[], int32_t length)
{
  int32_t start = 0;
  int32_t end = length -1;
  while (start < end)
  {
    utils_swap((str+start), (str+end));
    start++;
    end--;
  }
}

char *
utils_itoa(int32_t num, char* str, int32_t base)
{
  int32_t i = 0;
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

  // reverse the string
  utils_reverse(str, i);

  return str;
}

double
utils_pow(double a, double b)
{
  union {
    double d;
    int32_t x[2];
  } u = { a };
  u.x[1] = (int32_t)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

int
utils_itos(int32_t x, char str[], int32_t d)
{
  int32_t i = 0;
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
utils_ftoa(double n, char *res, int32_t afterpoint)
{
  // modulusct integer part
  int32_t ipart = (int32_t)n;

  // modulusct floating part
  double fpart = n - (double)ipart;

  // convert integer part to string
  int32_t i = utils_itos(ipart, res, 0);

  // check for display option after point
  if (afterpoint != 0)
  {
    res[i] = '.';  // add dot

    // Get the value of fraction part upto given no.
    // of points after dot. The third parameter is needed
    // to handle cases like 233.007
    fpart = fpart * utils_pow(10, afterpoint);

    utils_itos((int32_t)fpart, res + i + 1, afterpoint);
  }
  return res;
}

double
utils_atof (const char *p)
{
  int32_t frac;
  double sign, value, scale;

  // Skip leading white space, if any.

  while (utils_white_space(*p) ) {
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

  for (value = 0.0; utils_isdigit(*p); p += 1) {
    value = value * 10.0 + (*p - '0');
  }

  // Get digits after decimal point, if any.

  if (*p == '.') {
    double pow10 = 10.0;
    p += 1;
    while (utils_isdigit(*p)) {
      value += (*p - '0') / pow10;
      pow10 *= 10.0;
      p += 1;
    }
  }

  // Handle exponent, if any.

  frac = 0;
  scale = 1.0;
  if ((*p == 'e') || (*p == 'E')) {
    uint32_t expon;

    // Get sign of exponent, if any.

    p += 1;
    if (*p == '-') {
      frac = 1;
      p += 1;

    } else if (*p == '+') {
      p += 1;
    }

    // Get digits of exponent, if any.

    for (expon = 0; utils_isdigit(*p); p += 1) {
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

int32_t
utils_atoi(char *str)
{
  if (*str == '\0')
    return 0;

  int32_t res = 0; // Initialize result
  int32_t sign = 1; // Initialize sign as positive
  int32_t i = 0; // Initialize index of first digit

  // If number is negative, then update sign
  if (str[0] == '-')
  {
    sign = -1;
    i++; // Also update index of first digit
  }

  // Iterate through all digits of input string and update result
  for (; str[i] != '\0'; ++i)
  {
    if (!utils_isdigit(str[i]))
      return 0; // You may add some lines to write error message
          // to error stream
    res = res*10 + str[i] - '0';
  }

  // Return result with sign
  return sign*res;
}


int32_t
utils_is_hexadecimal(char *str)
{
  size_t i;
  for(i = 0; i < strlen(str); i++){
    if(i == 1 && str[i] == 'x'){
      continue;
    }
    if(!utils_ishex(str[i])){
      return 0;
    }
  }
  return 1;
}

int32_t
utils_is_double(char *str)
{
  size_t i, rep = 0;
  for(i = 0; i < strlen(str); i++){
    if(str[i] == '.'){
      rep++;
      if(rep > 1){
        return 0;
      }
      continue;
    }
    if(!utils_isdigit(str[i])){
      return 0;
    }
  }
  return 1;
}

int32_t
utils_is_integer(char *str)
{
  size_t i;
  for(i = 0; i < strlen(str); i++){
    if(!utils_isdigit(str[i])){
      return 0;
    }
  }
  return 1;
}

void
utils_combine_path(char* destination, const char* path1, const char* path2)
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
        int32_t append_directory_separator = 0;
        if(strcmp(last_char, directory_separator) != 0) {
            append_directory_separator = 1;
        }
        strcpy(destination, path1);
        if(append_directory_separator)
            strcat(destination, directory_separator);
        strcat(destination, path2);
    }
}
