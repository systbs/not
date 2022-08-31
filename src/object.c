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
#include "list.h"
#include "lexer.h"
#include "memory.h"
#include "object.h"
#include "data.h"
#include "variable.h"
#include "schema.h"
#include "layout.h"
#include "parser.h"

long_t
object_sizeof(object_t *object){
	return qalam_sizeof(object->ptr) + sizeof(object_t);
}

object_t *
object_clone(object_t *object)
{
	object_t *obj;
	assert_format(!!(obj = object_define(object->type, qalam_sizeof(object->ptr))),
		"[OBJECT CLONE] object not defined");
	if(object->type == OTP_ARRAY || object->type == OTP_TUPLE){
		obj->ptr = data_clone((list_t *)object->ptr);
	} else {
		object_assign(obj, object);
	}
	return obj;
}

void 
object_assign(object_t *target, object_t *source){
	if((target->type == OTP_ARRAY) && ((source->type == OTP_ARRAY) || (source->type == OTP_TUPLE))){
		list_t *tt = (list_t *)target->ptr;
		list_t *ts = (list_t *)source->ptr;
		ilist_t *a, *b;
		for(a = tt->begin, b = ts->begin; a != tt->end; a = a->next){
			if(b == ts->end){
				return;
			}
			object_assign((object_t *)a->value, (object_t *)b->value);
		}
		return;
	}
	if(object_sizeof(source) != object_sizeof(target)){
		target = object_redefine(target, source->type, qalam_sizeof(source->ptr));
	}
	target->type = source->type;
	oset(target, source);
}

void
object_delete(object_t *obj) {
	qalam_free(obj);
}

const char * const object_tn[] = {
	[OTP_LONG]      = "LONG",
	[OTP_DOUBLE]    = "DOUBLE",
	[OTP_CHAR]      = "CHAR",
	[OTP_NULL]      = "NULL",
	[OTP_ARRAY]     = "ARRAY",
	[OTP_TUPLE]    	= "TUPLE",
	[OTP_SET]    	= "SET",
	[OTP_SCHEMA]    = "SCHEMA",
	[OTP_LAYOUT]    = "LAYOUT",
	[OTP_ADRS]      = "ADRS"
};

const char *
object_tas(object_t *obj){
	return object_tn[obj->type];
}

const int object_typesLength[] = {
	[OTP_LONG] 		= sizeof(long_t),
	[OTP_DOUBLE] 	= sizeof(double_t),
	[OTP_CHAR] 		= sizeof(char_t),
	[OTP_NULL] 		= sizeof(ptr_t),
	[OTP_ARRAY] 	= sizeof(list_t),
	[OTP_TUPLE] 	= sizeof(list_t),
	[OTP_SCHEMA] 	= sizeof(schema_t),
	[OTP_LAYOUT] 	= sizeof(layout_t),
	[OTP_ADRS] 		= sizeof(long_t)
};

int
object_ts(object_t *obj){
	return object_typesLength[obj->type];
}

int
object_isnum(object_t *obj) {
	if((obj->type == OTP_LONG) || (obj->type == OTP_DOUBLE)) {
		return 1;
	}
	return 0;
}
