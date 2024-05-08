#include <stdio.h>
#include <math.h>

int main(void)
{
  double result = pow(2, 112);
  printf("The square root of %lf\n", result);
  return 0;
}

#define FLOAT32_MAX  9007199254740992
#define FLOAT64_MAX  9007199254740992
#define FLOAT128_MAX  5192296858534827628530496329220096