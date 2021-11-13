#pragma once

typedef enum object_type {
	OTP_LONG,
	OTP_DOUBLE,
	OTP_CHAR,
	OTP_PARAMS,
	OTP_ADRS,
	OTP_ARRAY,
	OTP_SCHEMA,
	OTP_LAYOUT,
	OTP_NULL,
	OTP_ASM
} object_type_t;

typedef struct object {
	object_type_t type;
	ptr_t ptr;
} object_t;

const char *
object_tas(object_t *obj);

#define oset(target, source) { \
	validate_format((target->type == source->type), \
		"[OBJECT SET] invalid type, %s, %s", \
	object_tas(target), object_tas(source));\
	if(source->type == OTP_CHAR){ \
		*(char_t *)target->ptr = *(char_t *)source->ptr;\
	}\
	else if(source->type == OTP_LONG){ \
		*(long_t *)target->ptr = *(long_t *)source->ptr;\
	}\
	else if(source->type == OTP_DOUBLE){ \
		*(double_t *)target->ptr = *(double_t *)source->ptr;\
	}\
	else if(source->type == OTP_ARRAY){ \
		*(table_t *)target->ptr = *(table_t *)source->ptr;\
	}\
	else if(source->type == OTP_PARAMS){ \
		*(table_t *)target->ptr = *(table_t *)source->ptr;\
	}\
	else if(source->type == OTP_SCHEMA){ \
		*(schema_t *)target->ptr = *(schema_t *)source->ptr;\
	}\
	else if(source->type == OTP_LAYOUT){ \
		*(layout_t *)target->ptr = *(layout_t *)source->ptr;\
	}\
	else if(source->type == OTP_ADRS){ \
		*(long_t *)target->ptr = *(long_t *)source->ptr;\
	}\
	else { \
		target->ptr = source->ptr;\
	}\
}

#define oget(obj) ((obj->type == OTP_LONG) ? *(long_t*)obj->ptr : *(double_t*)obj->ptr)

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
