#pragma once


typedef enum object_type {
	OTP_LONG,
	OTP_DOUBLE,
	OTP_CHAR,
	OTP_PARAMS,
	OTP_ADRS,
	OTP_ARRAY,
	OTP_SCHEMA,
	OTP_NULL
} object_type_t;

typedef struct object {
	object_type_t type;
	ptr_t ptr;
} object_t;


#define otp(target, source) { \
	if(target->type == OTP_CHAR && source->type == OTP_CHAR){ \
		*(char_t *)target->ptr = *(char_t *)source->ptr;\
	}\
	else if(target->type == OTP_LONG && source->type == OTP_LONG){ \
		*(long_t *)target->ptr = *(long_t *)source->ptr;\
	}\
	else if(target->type == OTP_DOUBLE && source->type == OTP_DOUBLE){ \
		*(double_t *)target->ptr = *(double_t *)source->ptr;\
	}\
	else if(target->type == OTP_ARRAY && source->type == OTP_ARRAY){ \
		*(table_t *)target->ptr = *(table_t *)source->ptr;\
	}\
	else if(target->type == OTP_PARAMS && source->type == OTP_PARAMS){ \
		*(table_t *)target->ptr = *(table_t *)source->ptr;\
	}\
	else if(target->type == OTP_SCHEMA && source->type == OTP_SCHEMA){ \
		*(schema_t *)target->ptr = *(schema_t *)source->ptr;\
	}\
	else if(target->type == OTP_ADRS && source->type == OTP_ADRS){ \
		*(long_t *)target->ptr = *(long_t *)source->ptr;\
	}\
	else { \
		target->ptr = source->ptr;\
	}\
}

#define object_define(tp, size) ({\
    object_t *object = qalam_malloc(sizeof(object_t));\
	object->ptr = qalam_malloc(size);\
    object->type = tp;\
    object;\
})

#define object_redefine(object, tp, size) ({\
    object->ptr = qalam_realloc(object->ptr, size);\
    object->type = tp;\
    object;\
})


int
object_isnum(object_t *obj);

long_t
object_sizeof(object_t *object);

object_t *
object_clone(object_t *object);

void 
object_assign(object_t *target, object_t *source);

void
object_delete(object_t *obj);

const char *
object_tas(object_type_t tp);