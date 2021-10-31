#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>

#include "types.h"
#include "utils.h"
#include "array.h"
#include "table.h"
#include "lexer.h"
#include "memory.h"
#include "object.h"
#include "data.h"
#include "variable.h"
#include "schema.h"
#include "parser.h"

size_t
object_sizeof(object_t *object){
	return qalam_sizeof(object->ptr) + sizeof(object_t);
}

object_t *
object_clone(object_t *object)
{
	object_t *obj;
	validate_format(!!(obj = object_define(object->type, qalam_sizeof(object->ptr))),
		"[OBJECT CLONE] object not defined");
	if(object->type == OTP_ARRAY || object->type == OTP_PARAMS){
		obj->ptr = data_clone((table_t *)object->ptr);
	} else {
		otp(obj, object);
	}
	return obj;
}

void 
object_assign(object_t *target, object_t *source){
	if(object_sizeof(source) != object_sizeof(target)){
		target = object_redefine(target, source->type, qalam_sizeof(source->ptr));
	}
	target->type = source->type;
	otp(target, source);
}

void
object_delete(object_t *obj) {
	qalam_free(obj);
}

const char * const object_type_name[] = {
  [OTP_SHORT]     = "SHORT",
  [OTP_INT]    	 = "INT",
  [OTP_LONG]    	 = "LONG",
  [OTP_LONG64]    = "LONG64",
  [OTP_FLAOT]     = "FLAOT",
  [OTP_DOUBLE]    = "DOUBLE",
  [OTP_DOUBLE64]  = "DOUBLE64",
  [OTP_CHAR]      = "CHAR",
  [OTP_NULL]      = "NULL",
  [OTP_ARRAY]     = "ARRAY",
  [OTP_PARAMS]    = "PARAMS",
  [OTP_SCHEMA]    = "SCHEMA",
  [OTP_ADRS]      = "ADRS"
};

const char *
object_typeAsString(object_type_t tp){
	return object_type_name[tp];
}

int object_typesLength[] = {
	[OTP_SHORT] = sizeof(short_t),
	[OTP_INT] = sizeof(int_t),
	[OTP_LONG] = sizeof(long_t),
	[OTP_LONG64] = sizeof(long64_t),
	[OTP_FLAOT] = sizeof(float_t),
	[OTP_DOUBLE] = sizeof(double_t),
	[OTP_DOUBLE64] = sizeof(double_t),
	[OTP_CHAR] = sizeof(char_t),
	[OTP_NULL] = sizeof(ptr_t),
	[OTP_ARRAY] = sizeof(table_t),
	[OTP_PARAMS] = sizeof(schema_t),
	[OTP_SCHEMA] = sizeof(table_t),
	[OTP_ADRS] = sizeof(arval_t)
};

int
object_typesSizeof(object_type_t tp){
	return object_typesLength[tp];
}
