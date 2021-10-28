#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

typedef enum object_type {
	TP_NULL,
	TP_CHAR,
	TP_NUMBER,
	TP_DATA,
	TP_CLASS
} object_type_t;

typedef struct object {
	object_type_t type;
	union {
		double num;
		void *ptr;
	} value;
} object_t;

#define SIZEOF_CONTENT(container) sizeof(container)
#define TYPEOF_CONTENT(container) container->type

#define UPDATE_CONTENT(container, val, tp) \
	if((tp == TP_NUMBER) || (tp == TP_CHAR)) {\
		container->value.num = val; \
		container->type = tp;\
	}else{\
		container->value.ptr = (void *)val;\
		container->type = tp;\
	}

long call(long i){
	return i > 0 ? call(i-1) : 0;
}


int main(int argc, char **argv){
	clock_t before = clock();
	long sum = 0;
    long i = 1000000;
    while(i > 0){
        //sum = sum + i;
        i = i - 1;
    }
    //printf("%ld\n", sum);
	clock_t difference = clock() - before;

    printf("c at %lf sec done.\n", (double)((double)difference / (double)CLOCKS_PER_SEC));

    i = 1000000;

    before = clock();
    call(i);
    difference = clock() - before;
    printf("c at %lf sec done.\n", (double)((double)difference / (double)CLOCKS_PER_SEC));

/*
	printf("char = %ld\nshort = %ld\nint = %ld\nfloat = %ld\nlong = %ld\ndouble = %ld\nlong long = %ld\nlong long int = %ld\nlong double = %ld.\n",
	sizeof(char),
	sizeof(short),
	sizeof(int),
	sizeof(float),
	sizeof(long),
	sizeof(double),
	sizeof(long long),
	sizeof(unsigned long long),
	sizeof(long double));

	object_t *obj = malloc(sizeof(object_t));
	UPDATE_CONTENT(obj, 100, TP_NUMBER);
	printf("ptr size %f\n", obj->value.num);
*/
}
