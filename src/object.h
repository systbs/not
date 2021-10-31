#pragma once

typedef enum object_type {
	TP_IMM,
	TP_NUMBER,
	TP_SHORT,
	TP_LONG,
	TP_LONG64,
	TP_FLAOT,
	TP_DOUBLE,
	TP_DOUBLE64,
	TP_VAR,
	TP_PTR,
	TP_CHAR,
	//enum ptr types
	TP_NULL,
	TP_ARRAY,
	TP_PARAMS,
	TP_SCHEMA,
	TP_ADRS,
} object_type_t;

typedef struct object {
	object_type_t type;
	ptr_t ptr;
} object_t;



#define object_define(otp, size) ({\
    object_t *object = qalam_malloc(sizeof(object_t));\
	object->ptr = qalam_malloc(size);\
    object->type = otp;\
    object;\
})

#define object_redefine(object, otp, size) ({\
    object->ptr = qalam_realloc(object->ptr, size);\
    object->type = otp;\
    object;\
})

#define object_isnum(obj) ( \
	(obj->type == TP_SHORT) || \
	(obj->type == TP_LONG) || \
	(obj->type == TP_LONG64) || \
	(obj->type == TP_FLAOT) || \
	(obj->type == TP_DOUBLE) || \
	(obj->type == TP_DOUBLE64)\
)

size_t
object_sizeof(object_t *object);

arval_t
object_sort(arval_t *obj_1, arval_t *obj_2);

object_t *
object_clone(object_t *object);

void 
object_assign(object_t *target, object_t *source);

void
object_delete(object_t *obj);

const char *
object_typeAsString(object_type_t tp);