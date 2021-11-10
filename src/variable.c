#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <dirent.h>
#include <stdarg.h>

#include "types.h"
#include "utils.h"
#include "array.h"
#include "table.h"
#include "lexer.h"
#include "memory.h"
#include "object.h"
#include "variable.h"
#include "schema.h"
#include "parser.h"
#include "data.h"

variable_t *
variable_define(char *identifier)
{
    variable_t *var;
	validate_format(!!(var = qalam_malloc(sizeof(variable_t))), 
		"variable not true defined");
	var->identifier = identifier;
	return var;
}

variable_t *
variable_findlst(table_t *variables, char *identifier)
{
	itable_t *c;
	for(c = variables->begin; c != variables->end; c = c->next){
		variable_t *var = (variable_t *)c->value;
		if(strncmp(identifier, var->identifier, max(strlen(identifier), strlen(var->identifier))) == 0){
			return var;
		}
	}
	return nullptr;
}

//find by content/object
variable_t *
variable_fcnt(table_t *variables, object_t *object)
{
	itable_t *c;
	for(c = variables->begin; c != variables->end; c = c->next){
		variable_t *var = (variable_t *)c->value;
		if(!!var->object){
			if((long_t)object == (long_t)var->object){
				return var;
			}
		}
	}
	return nullptr;
}

void *
variable_content(variable_t *var){
	return var ? var->object : nullptr;
}

