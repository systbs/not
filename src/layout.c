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
#include "list.h"
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

    layout->identifier = uuid();

    layout->schema = schema;

	// save previous object in frame then retrive it
	layout->frame = list_create();
	// save previous variables in scope
	layout->scope = list_create();
    layout->parameters = list_create();
    layout->variables = list_create();
	layout->extends = list_create();

	if(!!schema){
		ilist_t *b;
		for(b = schema->variables->begin; b != schema->variables->end; b = b->next){
			variable_t *var = (variable_t *)b->value;
			assert_format(!!(var = variable_define((char *)var->identifier)),
				"unable to alloc var");
			var->object = object_define(OTP_NULL, sizeof(ptr_t));
			list_rpush(layout->variables, (list_value_t)var);
			list_rpush(layout->parameters, (list_value_t)object_clone(var->object));
		}
	}

	layout->parent = nullptr;

    return layout;
}

layout_t *
layout_fpt(list_t *lst, char *identifier) 
{
	ilist_t *u;
	for(u = lst->begin; u && (u != lst->end); u = u->next){
		layout_t *layout = (layout_t *)u->value;
		if(strncmp(identifier, layout->identifier, max(strlen(identifier), strlen(layout->identifier))) == 0){
			return layout;
		}
	}
	return nullptr;
}

//find variable by id without parent
variable_t *
layout_vwp(layout_t *layout, char *identifier)
{
	variable_t *var;
	if(!!(var = variable_findlst(layout->variables, identifier))){
		return var;
	}
    ilist_t *b;
	for(b = layout->scope->begin; (b != layout->scope->end); b = b->next){
        if(!!(var = variable_findlst((list_t *)b->value, identifier))){
            return var;
        }
    }
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
	if(!!(var = layout_vwp(layout, identifier))){
		return var;
	}
    ilist_t *b;
	for(b = layout->extends->begin; (b != layout->extends->end); b = b->next){
        if(!!(var = layout_vwp((layout_t *)b->value, identifier))){
            return var;
        }
    }
	if(!!layout->parent){
		if(!!(var = layout_variable(layout->parent, identifier))){
            return var;
        }
	}
	return nullptr;
}
