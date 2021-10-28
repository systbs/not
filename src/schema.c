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

	schema->type = SCHEMA_PRIMARY;
    
	schema->branches = table_create();
    schema->parameters = table_create();
    schema->variables = table_create();
	schema->extends = table_create();

    schema->subschema = parent;

    return schema;
}


schema_t *
schema_duplicate(schema_t *source){
	validate_format((source->type == SCHEMA_PRIMARY),
		"you only create duplicate from primary schema");
		
    schema_t *schema = (schema_t *)qalam_malloc(sizeof(schema_t));
	schema->type = SCHEMA_DUP;

    schema->identifier = source->identifier;

	schema->start = source->start;
	schema->end = source->end;
    
	schema->branches = source->branches;
	schema->subschema = source->subschema;
	schema->duplicate = source->duplicate ? source->duplicate : table_create();

    schema->parameters = table_create();
	itable_t *b;
	for(b = source->parameters->begin; b != source->parameters->end; b = b->next){
		table_rpush(schema->parameters, (value_p)object_clone((object_t *)b->value));
	}

    schema->variables = table_create();
	for(b = source->variables->begin; b != source->variables->end; b = b->next){
		table_rpush(schema->variables, (value_p)object_clone((object_t *)b->value));
	}

	schema->extends = table_create();
	for(b = source->extends->begin; b != source->extends->end; b = b->next){
		table_rpush(schema->extends, (value_p)b->value);
	}

	schema->packet = table_create();
	schema->frame = table_create();

	table_rpush(schema->duplicate, (value_p)schema);

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
	if((var = variable_findlst(schema->variables, identifier))){
		return var;
	}
    // search dynamic vars
	if(schema->parent != nullptr){
		if((var = schema_variable(schema->parent, identifier))){
			return var;
		}
	}
    // search in static vars
    if(schema->subschema != nullptr){
		if((var = schema_variable(schema->subschema, identifier))){
			return var;
		}
	}
	return nullptr;
}




