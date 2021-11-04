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

variable_t *
layout_vwp(layout_t *layout, char *identifier)
{
	variable_t *var;
	if(!!(var = variable_findlst(layout->variables, identifier))){
		return var;
	}
    itable_t *b;
	for(b = layout->extends->begin; (b != layout->extends->end); b = b->next){
        if(!!(var = layout_vwp((layout_t *)b->value, identifier))){
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
        if(!!(var = layout_vwp((layout_t *)b->value, identifier))){
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