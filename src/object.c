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

	switch (object->type) {
		case TP_NULL:
			obj->ptr = object->ptr;
			break;
		case TP_CHAR:
			*(char_t *)obj->ptr = *(char_t *)object->ptr;
			break;
		case TP_NUMBER:
			*(double64_t *)obj->ptr = *(double64_t *)object->ptr;
			break;
		case TP_ARRAY:
		case TP_PARAMS:
			obj->ptr = data_clone((table_t *)object->ptr);
			break;
		case TP_SCHEMA:
			obj->ptr = (schema_t *)object->ptr;
			break;
		default:
			printf("clone, unknown type! %d\n", object->type);
			exit(-1);
	}

	return obj;
}

value_t
object_sort(value_t *obj_1, value_t *obj_2)
{
	object_t *obj1 = (object_t *)obj_1;
	object_t *obj2 = (object_t *)obj_2;
	if(obj1->type == TP_NUMBER && obj2->type == TP_NUMBER){
		return *(double_t *)obj1->ptr < *(double_t *)obj2->ptr;
	}
	return 0;
}

void 
object_assign(object_t *target, object_t *source){
	if(object_sizeof(source) != object_sizeof(target)){
		target = object_redefine(target, source->type, qalam_sizeof(source->ptr));
	}
	target->type = source->type;
	switch (source->type) {
		case TP_CHAR:
			*(char_t *)target->ptr = *(char_t *)source->ptr;
			break;
		case TP_NUMBER:
			*(double64_t *)target->ptr = *(double64_t *)source->ptr;
			break;
		case TP_PARAMS:
		case TP_ARRAY:
			*(table_t *)target->ptr = *(table_t *)source->ptr;
			break;
		case TP_SCHEMA:
			*(schema_t *)target->ptr = *(schema_t *)source->ptr;
			break;
		default:
			printf("ASSIGN, unknown type! %d\n", source->type);
			exit(-1);
	}
}

void
object_delete(object_t *obj) {
	qalam_free(obj);
}

const char * const object_type_name[] = {
  [TP_IMM]       = "IMM",
  [TP_NUMBER]    = "NUMBER",
  [TP_VAR]     = "LABEL",
  [TP_PTR]       = "PTR",
  [TP_CHAR]      = "CHAR",
  [TP_NULL]      = "NULL",
  [TP_ARRAY]     = "ARRAY",
  [TP_PARAMS]    = "PARAMS",
  [TP_SCHEMA]    = "SCHEMA",
  [TP_ADRS]      = "ADRS"
};

const char *
object_typeAsString(object_type_t tp){
	return object_type_name[tp];
}
