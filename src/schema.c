#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
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

schema_t *
schema_create(schema_t *parent){
    schema_t *schema = (schema_t *)qalam_malloc(sizeof(schema_t));

    schema->identifier = uuid(32);

	schema->branches = table_create();
	schema->extends = table_create();
	schema->variables = table_create();

    schema->parent = parent;

    return schema;
}


schema_t *
schema_fpt(table_t *tbl, char *identifier) {
	itable_t *u;
	for(u = tbl->begin; u && (u != tbl->end); u = u->next){
		schema_t *schema = (schema_t *)u->value;
		if(strncmp(identifier, schema->identifier, max(strlen(identifier), strlen(schema->identifier))) == 0){
			return schema;
		}
	}
	return nullptr;
}

schema_t *
schema_branches(schema_t *schema, char *identifier) {
	schema_t *temp;
	if((temp = schema_fpt(schema->branches, identifier))){
		return temp;
	}
	return nullptr;
}

variable_t *
schema_variable(schema_t *schema, char *identifier)
{
	variable_t *var;
	if(!!(var = variable_findlst(schema->variables, identifier))){
		return var;
	}
    // search dynamic vars
	if(schema->parent != nullptr){
		if(!!(var = schema_variable(schema->parent, identifier))){
			return var;
		}
	}
    // search in static vars
    if(schema->parent != nullptr){
		if(!!(var = schema_variable(schema->parent, identifier))){
			return var;
		}
	}
	return nullptr;
}




