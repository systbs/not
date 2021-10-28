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

object_t *
object_clone(object_t *object)
{
	object_t *obj;
	validate_format(!!(obj = object_define(object->type, sizeof(ptr_t))),
		"[OBJECT CLONE] object not defined");

	switch (object->type) {
		case TP_NULL:
			if(obj->type != object->type){
				obj->ptr = qalam_object_realloc(obj->ptr, sizeof(ptr_t));;
			}
			obj->ptr = object->ptr;
			break;
		case TP_CHAR:
			if(obj->type != object->type){
				obj->ptr = qalam_object_realloc(obj->ptr, sizeof(char));
			}
			obj->num = object->num;
			break;
		case TP_NUMBER:
			if(obj->type != object->type){
				obj->ptr = qalam_object_realloc(obj->ptr, sizeof(double));
			}
			obj->num = object->num;
			break;
		case TP_ARRAY:
		case TP_PARAMS:
			if(obj->type != object->type){
				obj->ptr = qalam_object_realloc(obj->ptr, sizeof(table_t));;
			}
			obj->ptr = data_clone((table_t *)object->ptr);
			break;
		case TP_SCHEMA:
			if(obj->type != object->type){
				obj->ptr = qalam_object_realloc(obj->ptr, sizeof(schema_t));
			}
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
		return obj1->num < obj2->num;
	}
	return 0;
}

value_t
object_sizeof(object_t *obj)
{
    if(obj->type == TP_NULL){
        return sizeof(object_t);
    }
    else if(obj->type == TP_NUMBER){
        return sizeof(double);
    }
	else if(obj->type == TP_CHAR){
        return sizeof(char);
    }
    else if(obj->type == TP_ARRAY){
        return data_sizeof((table_t *)obj->ptr);
    }
	else if(obj->type == TP_PARAMS){
        return data_sizeof((table_t *)obj->ptr);
    }
    printf("object sizeof, unknown type.\n");
    exit(-1);
}

void 
object_assign(object_t *target, object_t *source){
	switch (source->type) {
		case TP_NULL:
			target = object_redefine(target, source->type, sizeof(void *));
			target->ptr = source->ptr;
			break;
		case TP_CHAR:
			target = object_redefine(target, source->type, sizeof(char));
			target->num = source->num;
			break;
		case TP_NUMBER:
			target = object_redefine(target, source->type, sizeof(double));
			target->num = source->num;
			break;
		case TP_PARAMS:
		case TP_ARRAY:
			target = object_redefine(target, source->type, sizeof(void *));
			target->ptr = source->ptr;
			break;
		case TP_SCHEMA:
			target = object_redefine(target, source->type, sizeof(schema_t));
			target->ptr = (schema_t *)source->ptr;
			break;
		default:
			printf("ASSIGN, unknown type! %d\n", source->type);
			exit(-1);
	}
	target->type = source->type;
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
