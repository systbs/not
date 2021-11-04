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
#include "layout.h"
#include "parser.h"
#include "data.h"

layout_t *
layout_create(schema_t *schema)
{
    layout_t *layout = (layout_t *)qalam_malloc(sizeof(layout_t));

    layout->identifier = uuid(32);

    layout->schema = schema;

	layout->frame = table_create();
    layout->parameters = table_create();
    layout->variables = table_create();
	layout->extends = table_create();

	if(!!schema){
		itable_t *b;
		for(b = schema->parameters->begin; b != schema->parameters->end; b = b->next){
			variable_t *var = (variable_t *)b->value;
			validate_format(!!(var = variable_define((char *)var->identifier)),
				"unable to alloc var");
			var->object = object_define(OTP_NULL, sizeof(ptr_t));
			table_rpush(layout->variables, (tbval_t)var);
			table_rpush(layout->parameters, (tbval_t)var->object);
		}
	}

	layout->parent = nullptr;

    return layout;
}

layout_t *
layout_fpt(table_t *tbl, char *identifier) 
{
	itable_t *u;
	for(u = tbl->begin; u && (u != tbl->end); u = u->next){
		layout_t *layout = (layout_t *)u->value;
		if(strncmp(identifier, layout->identifier, max(strlen(identifier), strlen(layout->identifier))) == 0){
			return layout;
		}
	}
	return nullptr;
}

//find variable by id without parent
variable_t *
layout_var_wp(layout_t *layout, char *identifier)
{
	variable_t *var;
	if(!!(var = variable_findlst(layout->variables, identifier))){
		return var;
	}
    itable_t *b;
	for(b = layout->extends->begin; (b != layout->extends->end); b = b->next){
        if(!!(var = layout_var_wp((layout_t *)b->value, identifier))){
            return var;
        }
    }
	return nullptr;
}

variable_t *
layout_variable(layout_t *layout, char *identifier)
{
	variable_t *var;
	if(!!(var = variable_findlst(layout->variables, identifier))){
		return var;
	}
    itable_t *b;
	for(b = layout->extends->begin; (b != layout->extends->end); b = b->next){
        if(!!(var = layout_var_wp((layout_t *)b->value, identifier))){
            return var;
        }
    }
    // search in parent vars
	if(!!layout->parent){
		if(!!(var = layout_variable(layout->parent, identifier))){
            return var;
        }
	}
	return nullptr;
}

//find variable by content without parent
variable_t *
layout_var_cwp(layout_t *layout, object_t *object)
{
	variable_t *var;
	if(!!(var = variable_fcnt(layout->variables, object))){
		return var;
	}
    itable_t *b;
	for(b = layout->extends->begin; (b != layout->extends->end); b = b->next){
        if(!!(var = layout_var_cwp((layout_t *)b->value, object))){
            return var;
        }
    }
	return nullptr;
}

//get variable by content
variable_t *
layout_fcnt(layout_t *layout, object_t *object)
{
	variable_t *var;
	if(!!(var = variable_fcnt(layout->variables, object))){
		return var;
	}
    itable_t *b;
	for(b = layout->extends->begin; (b != layout->extends->end); b = b->next){
        if(!!(var = layout_var_cwp((layout_t *)b->value, object))){
            return var;
        }
    }
    // search in parent vars
	if(!!layout->parent){
		if(!!(var = layout_fcnt(layout->parent, object))){
            return var;
        }
	}
	return nullptr;
}