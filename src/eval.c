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

#ifdef WIN32
	#include <conio.h>
#else
	#include "conio.h"
#endif

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


typedef struct thread {
	layout_t *layout;

	table_t *frame;
	table_t *board;

	table_t *variables;

	object_t *object;
} thread_t;

thread_t *
thread_create(){
	thread_t *tr;
	validate_format(!!(tr = qalam_malloc(sizeof(thread_t))), 
		"unable to malloc root thread");

	tr->layout = layout_create(nullptr);

	tr->frame = table_create();
	tr->board = table_create();

	tr->variables = table_create();

	tr->object = nullptr;
	return tr;
}

//OR LOR XOR AND LAND EQ NE LT LE GT GE LSHIFT RSHIFT ADD SUB MUL DIV MOD
iarray_t *
call(thread_t *tr, schema_t *schema, iarray_t *adrs) {
	object_t *object;
	validate_format(!!(object = object_define(OTP_ADRS, sizeof(long_t))), 
		"unable to alloc memory!");
	object->ptr = (tbval_t)adrs;

	table_rpush(tr->layout->frame, (tbval_t)object);
	table_rpush(tr->board, (tbval_t)layout_create(schema));

	return (iarray_t *)schema->start;
}



iarray_t *
thread_or(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");
	
	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));
	
	object_t *object;
	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) || oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_lor(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) | (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_xor(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");

	*(long_t *)object->ptr = (long_t)oget(esp) ^ (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_and(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) && oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_land(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) & (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_eq(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) == oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_ne(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) != oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_lt(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
	*(long_t *)object->ptr = oget(esp) < oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_le(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) <= oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_gt(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) > oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_ge(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) >= oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_lshift(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)),
		"in << operator used object %s && %s", 
		object_tas(esp), object_tas(tr->object)
	);

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) << (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_rshift(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)),
		"in >> operator used object %s && %s", 
		object_tas(esp), object_tas(tr->object)
	);

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) >> (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_add(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = oget(esp) + oget(tr->object);
	} else {
		validate_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = oget(esp) + oget(tr->object);
	}

	tr->object = object;

	return c->next;
}

iarray_t *
thread_sub(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = oget(esp) - oget(tr->object);
	} else {
		validate_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = oget(esp) - oget(tr->object);
	}

	tr->object = object;

	return c->next;
}

iarray_t *
thread_mul(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = oget(esp) * oget(tr->object);
	} else {
		validate_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = oget(esp) * oget(tr->object);
	}

	tr->object = object;

	return c->next;
}

iarray_t *
thread_div(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"frame is empty");

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = (long_t)(oget(esp) / oget(tr->object));
	} else {
		validate_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = (double_t)(oget(esp) / oget(tr->object));
	}

	tr->object = object;

	return c->next;
}

iarray_t *
thread_mod(thread_t *tr, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)table_content(table_rpop(tr->layout->frame)))){
		printf("[%%], you can use '%%' between two object!\n");
		exit(-1);
	}

	validate_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = (long_t)((long_t)oget(esp) % (long_t)oget(tr->object));
	} else {
		validate_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = (double_t)((long_t)oget(esp) % (long_t)oget(tr->object));
	}

	tr->object = object;

	return c->next;
}

iarray_t *
thread_comma(thread_t *tr, iarray_t *c){
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"[,] you can use ',' between two object!");
	object_t *obj;
	if (esp->type != OTP_PARAMS) {
		validate_format(!!(obj = object_define(OTP_PARAMS, sizeof(table_t))), 
			"unable to alloc memory!");
		table_t *tbl = table_create();
		table_rpush(tbl, (tbval_t)esp);
		table_rpush(tbl, (tbval_t)tr->object);
		obj->ptr = tbl;
		tr->object = obj;
	} else {
		table_rpush((table_t *)esp->ptr, (tbval_t)tr->object);
		tr->object = esp;
	}
	return c->next;
}

/* 
	VAR IMM DATA SD LD PUSH JMP JZ JNZ CENT CLEV CALL ENT LEV THIS SUPER DOT
	SIZEOF TYPEOF DELETE INSERT COUNT BREAK NEW REF RET
*/

iarray_t *
thread_imm(thread_t *tr, iarray_t *c) {
	// load immediate value to object
	c = c->next;
	arval_t value = c->value;
	c = c->next;
	
	if (c->value == TP_VAR) {
		variable_t *var;
		if(!!(var = layout_variable(tr->layout, (char *)value))){
			validate_format(!!(tr->object = variable_content(var)),
				"[IMM] variable dont have content");
			return c->next;
		}
		schema_t *schema;
		if(!!(schema = schema_branches(tr->layout->schema, (char *)value))){
			object_t *object;
			validate_format(!!(object = object_define(OTP_SCHEMA, sizeof(schema_t))), 
				"[IMM] schema object not created");
			object->ptr = schema;
			tr->object = object;
			return c->next;
		}
		var = variable_define((char *)value);
		var->object = object_define(OTP_NULL, sizeof(ptr_t));
		table_rpush(tr->layout->variables, (tbval_t)var);
		table_rpush(tr->variables, (tbval_t)var);
		validate_format(!!(tr->object = variable_content(var)),
			"[IMM] variable dont have content");
		return c->next;
	}
	else if(c->value == TP_ARRAY){
        object_t *object;
        validate_format(!!(object = object_define(OTP_ARRAY, sizeof(table_t))), 
            "unable to alloc memory!");
        object->ptr = data_from((char_t *)value);
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_NUMBER){
        object_t *object;
		double_t num = utils_atof((char *)value);
		if(num - (long_t)num != 0){
			validate_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
            "unable to alloc memory!");
			*(double_t *)object->ptr = num;
		} else {
			validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
            "unable to alloc memory!");
			*(long_t *)object->ptr = (long_t)num;
		}
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_IMM){
        object_t *object;
        validate_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
            "unable to alloc memory!");
        *(long_t *)object->ptr = (long_t)value;
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_NULL){
        object_t *object;
        validate_format(!!(object = object_define(OTP_NULL, sizeof(ptr_t))), 
            "unable to alloc memory!");
        object->ptr = nullptr;
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_SCHEMA){
        object_t *object;
        validate_format(!!(object = object_define(OTP_SCHEMA, sizeof(schema_t))), 
            "unable to alloc memory!");
        object->ptr = (schema_t *)value;
		tr->object = object;
		return c->next;
	}

	validate_format( !!(c->value == TP_NULL), "[IMM] unknown type");

	return c->next;
}

iarray_t *
thread_sd(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))), 
		"save data, bad pop data");
	if(esp->type == OTP_SCHEMA){
		return call(tr, (schema_t *)esp->ptr, c->next);
	}
	else if(esp->type == OTP_PARAMS){
		validate_format((tr->object->type == OTP_PARAMS), 
			"[SD] after assign operator must be use PARAMS type");
		itable_t *a, *b, *e, *d;
		table_t *ta, *tb, *tc;
		variable_t *var_a, *var_e;
		ta = (table_t *)esp->ptr;
		tb = (table_t *)tr->variables;
		tc = (table_t *)tr->object->ptr;
		for(a = ta->begin; a != ta->end; a = a->next){
			for(b = tb->begin; b != tb->end; b = b->next){
				var_a = (variable_t *)b->value;
				if((tbval_t)a->value == (tbval_t)var_a->object){
					goto sd_find_object_a;
				}
			}
		}
		goto sd_find_end;

		sd_find_object_a:
		for(e = tc->begin; e != tc->end; e = e->next){
			for(d = tb->begin; d != tb->end; d = d->next){
				var_e = (variable_t *)d->value;
				if(((tbval_t)e->value == (tbval_t)var_e->object)){
					goto sd_find_object_e;
				}
			}
		}
		goto sd_find_end;

		sd_find_object_e:
		if(strncmp(var_a->identifier, var_e->identifier, max(strlen(var_a->identifier), strlen(var_e->identifier))) == 0){
			object_assign((object_t *)a->value, (object_t *)e->value);
		}

		sd_find_end:
		tr->object = esp;
		return c->next;
	}else {
		object_assign(esp, tr->object);
		tr->object = esp;
		return c->next;
	}
}

iarray_t *
thread_ld(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!(esp = (object_t *)table_content(table_rpop(tr->layout->parameters))),
		"load data, bad pop data!");
	object_assign(tr->object, esp);
	return c->next;
}

iarray_t *
thread_push(thread_t *tr, iarray_t *c) {
	// push the value of object onto the frames
	table_rpush(tr->layout->frame, (tbval_t)tr->object);
	return c->next;
}

iarray_t *
thread_jmp(thread_t *tr, iarray_t *c) {
	// jump to the address
	c = c->next;
	return (iarray_t *)c->value;
}

iarray_t *
thread_jz(thread_t *tr, iarray_t *c) {
	// jump if object is zero
	c = c->next;
	return (long_t)oget(tr->object) ? c->next : (iarray_t *)c->value;
}

iarray_t *
thread_jnz(thread_t *tr, iarray_t *c) {
	// jump if object is not zero
	c = c->next;
	return (long_t)oget(tr->object) ? (iarray_t *)c->value : c->next;
}



iarray_t *
thread_this(thread_t *tr, iarray_t *c) {
	object_t *object;
	validate_format(!!(object = object_define(OTP_LAYOUT, sizeof(layout_t))), 
		"unable to alloc memory!");
	object->ptr = (tbval_t)tr->layout;
	tr->object = object;
	return c->next;
}

iarray_t *
thread_super(thread_t *tr, iarray_t *c) {
	object_t *object;
	validate_format(!!(object = object_define(OTP_LAYOUT, sizeof(layout_t))), 
		"unable to alloc memory!");
	object->ptr = (tbval_t)tr->layout->parent ? tr->layout->parent : tr->layout;
	tr->object = object;
	return c->next;
}


iarray_t *
thread_cell(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
			"bad get operation!");

	validate_format((esp->type == OTP_ARRAY) || (esp->type == OTP_PARAMS), 
		"[CELL] for %s type, get array operator only use for ARRAY/PARAMS type", object_tas(esp));

	validate_format(!!(tr->object->type == OTP_ARRAY), 
		"[CELL] for %s type, argument must be an array", object_tas(tr->object));

	object_t *obj;
	validate_format(!!(obj = (object_t *)table_content(table_lpop((table_t *)tr->object->ptr))), 
		"[CELL] object not retrived");

	validate_format(!!object_isnum(obj), 
		"[CELL] for %s type, index must be a number type", object_tas(obj));

	tr->object = (object_t *)table_content(table_at((table_t *)esp->ptr, oget(obj)));
	return c->next;
}

iarray_t *
thread_call(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
			"bad call operation!");

	validate_format((esp->type == OTP_SCHEMA), 
		"[CALL] for %s type, eval operator only use for SCHEMA type", 
		object_tas(esp));
		
	return call(tr, (schema_t *)esp->ptr, c->next);
}

iarray_t *
thread_ent(thread_t *tr, iarray_t *c) {
	table_rpush(tr->frame, (tbval_t)tr->layout);

	layout_t *layout;
	layout = (layout_t *)table_content(table_rpop(tr->board));
	layout->parent = tr->layout;
	tr->layout = layout;

	tr->layout->object = object_define(OTP_NULL, sizeof(ptr_t));

	if(!!tr->object){
		if(tr->object->type == OTP_PARAMS){
			object_t *object;
			while (!!(object = (object_t *)table_content(table_lpop((table_t *)tr->object->ptr)))) {
				table_rpush(tr->layout->parameters, (tbval_t)object);
			}
			tr->object = nullptr;
		} else {
			table_rpush(tr->layout->parameters, (tbval_t)tr->object);
			tr->object = nullptr;
		}
	}

	variable_t *var;
	var = variable_define("params");
	var->object = object_define(OTP_PARAMS, sizeof(table_t));
	table_rpush(tr->layout->variables, (tbval_t)var);
	var->object->ptr = tr->layout->parameters;

	return c->next;
}

iarray_t *
thread_extend(thread_t *tr, iarray_t *c){
	itable_t *b, *n;
	for(b = tr->layout->schema->extends->begin; b != tr->layout->schema->extends->end; b = b->next){
		schema_t *schema = (schema_t *)b->value;
		int found = 0;
		for(n = tr->layout->extends->begin; n != tr->layout->extends->end; n = n->next){
			layout_t *layout = (layout_t *)n->value;
			if(strncmp(layout->schema->identifier, layout->schema->identifier, max(strlen(layout->schema->identifier), strlen(layout->schema->identifier))) == 0){
				found = 1;
				break;
			}
		}
		if(found){
			continue;
		}
		layout_t *layout = layout_create(schema);
		object_t *object;
		validate_format(!!(object = object_define(OTP_ADRS, sizeof(long_t))), 
			"unable to alloc memory!");
		object->ptr = (tbval_t)c;
		table_rpush(tr->layout->frame, (tbval_t)object);
		table_rpush(tr->layout->extends, (tbval_t)layout);
		table_rpush(tr->board, (tbval_t)layout);
		return (iarray_t *)schema->start;
	}
	
	return c->next;
}

iarray_t *
thread_lev(thread_t *tr, iarray_t *c) {
	tr->object = tr->layout->object;
	tr->layout = (layout_t *)table_content(table_rpop(tr->frame));

	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"wrong! bad leave function, not found adrs");

	validate_format((esp->type == OTP_ADRS), 
		"wrong! object not from type address");
	
	iarray_t * adrs;
	adrs = (iarray_t *)esp->ptr;

	return adrs;
}

iarray_t *
thread_ret(thread_t *tr, iarray_t *c) {
	tr->layout->object = tr->object;
	return c->next;
}

iarray_t *
thread_scheming(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->layout->frame))),
		"[CLS] missing object");

	validate_format((esp->type == OTP_SCHEMA) || (esp->type == OTP_NULL), 
		"[CLS] scheming type use only for null or schema type, %s", object_tas(esp));

	if((esp->type == OTP_NULL)){
		validate_format(!!(esp = object_redefine(esp, OTP_SCHEMA, sizeof(schema_t))), 
			"[CLS] redefine object for type schema");
		if(tr->object->type == OTP_PARAMS) {
			object_t *object;
			validate_format(!!(object = (object_t *)table_content(table_rpop((table_t *)tr->object->ptr))), 
				"[CLS] pop schema fram list ignored");
			validate_format((object->type == OTP_SCHEMA), 
				"[CLS] scheming type use only for schema type, %s", 
				object_tas(object)
			);
			esp->ptr = object->ptr;
		} else {
			validate_format((tr->object->type == OTP_SCHEMA), 
				"[CLS] scheming type use only for schema type, %s", 
				object_tas(tr->object)
			);
			esp->ptr = tr->object->ptr;
			tr->object = esp;
			return c->next;
		}
	}

	schema_t *schema;
	validate_format(!!(schema = (schema_t *)esp->ptr),
		"[CLS] schema is null");

	if(tr->object->type == OTP_PARAMS){
		table_t *tbl = (table_t *)tr->object->ptr;
		itable_t *b;
		for(b = tbl->begin; b != tbl->end; b = b->next){
			object_t *object = (object_t *)b->value;
			validate_format((object->type == OTP_SCHEMA), 
				"[CLS] schema type for scheming operator is required %s", object_tas(object));
			table_rpush(schema->extends, (tbval_t)object->ptr);
		}
	} 
	else if(tr->object->type == OTP_SCHEMA) {
		validate_format((tr->object->type == OTP_SCHEMA), 
			"[CLS] schema type for scheming operator is required %s", object_tas(tr->object));
		table_rpush(schema->extends, (tbval_t)tr->object->ptr);
	}

	tr->object = esp;
	return c->next;
}

iarray_t *
thread_sim(thread_t *tr, iarray_t *c){
	validate_format(!!((tr->object->type == OTP_SCHEMA) || (tr->object->type == OTP_LAYOUT)), 
		"[SIM] object is empty");
	if(tr->object->type == OTP_SCHEMA){
		schema_t *schema;
		validate_format(!!(schema = (schema_t *)tr->object->ptr), 
			"[SIM] object is empty");
		table_rpush(tr->frame, (tbval_t)tr->layout);
		tr->layout = layout_create(schema);
		return c->next;
	}
	else {
		layout_t *layout;
		validate_format(!!(layout = (layout_t *)tr->object->ptr), 
			"[SIM] object is empty");
		table_rpush(tr->frame, (tbval_t)tr->layout);
		tr->layout = layout;
		return c->next;
	}
}

iarray_t *
thread_rel(thread_t *tr, iarray_t *c){
	validate_format(!!(tr->layout = (layout_t *)table_content(table_rpop(tr->frame))), 
		"layout rel back frame is empty");
	return c->next;
}

iarray_t *
thread_print(thread_t *tr, iarray_t *c) {
	object_t *esp;
	table_t *tbl;
	if(tr->object->type == OTP_PARAMS){
		tbl = (table_t *)tr->object->ptr;
	} else {
		tbl = table_create();
		table_rpush(tbl, (tbval_t)tr->object);
	}

	while((esp = (object_t *)table_content(table_rpop(tbl)))){
		if(esp->type == OTP_NULL){
			printf("%s ", object_tas(esp));
			continue;
		}
		else if(esp->type == OTP_CHAR){
			printf("%d ", *(char_t *)esp->ptr);
			continue;
		}
		else if(esp->type == OTP_ARRAY){
			table_t *formated;

			formated = data_format((table_t *)esp->ptr, tbl);

			itable_t *t;
			while((t = table_lpop(formated))){
				object_t *obj = (object_t *)t->value;
				if(*(char_t *)obj->ptr == '\\'){

					t = table_lpop(formated);
					obj = (object_t *)t->value;

					if(*(char_t *)obj->ptr == 'n'){
						printf("\n");
						continue;
					}
					else
					if(*(char_t *)obj->ptr == 't'){
						printf("\t");
						continue;
					}
					else
					if(*(char_t *)obj->ptr == 'v'){
						printf("\v");
						continue;
					}
					else
					if(*(char_t *)obj->ptr == 'a'){
						printf("\a");
						continue;
					}
					else
					if(*(char_t *)obj->ptr == 'b'){
						printf("\b");
						continue;
					}

					printf("\\");
					continue;
				}

				printf("%c", *(char_t *)obj->ptr);
			}

			continue;
		}
		else if(esp->type == OTP_LONG){
			printf("%ld", *(long_t *)esp->ptr);
			continue;
		}
		else if(esp->type == OTP_DOUBLE){
			printf("%.6f", *(double_t *)esp->ptr);
			continue;
		}
		else if(esp->type == OTP_SCHEMA) {
			printf("SCHEMA \n");
		}
		else {
			printf("%ld", *(long_t *)esp->ptr);
		}
	}
	return c->next;
}

iarray_t *
thread_bscp(thread_t *tr, iarray_t *c) {
	table_rpush(tr->layout->scope, (tbval_t)tr->layout->variables);
	tr->layout->variables = table_create();
	return c->next;
}

iarray_t *
thread_escp(thread_t *tr, iarray_t *c) {
	tr->layout->variables = (table_t *)table_content(table_rpop(tr->layout->scope));
	return c->next;
}

iarray_t *
thread_array(thread_t *tr, iarray_t *c){
	validate_format(!!tr->object,
		"[ARRAY] object is required!");
	object_t *obj;
	validate_format(!!(obj = object_define(OTP_ARRAY, sizeof(table_t))), 
		"unable to alloc memory!");
	table_t *tbl = table_create();
	if(tr->object->type == OTP_PARAMS){
		table_t *tt = (table_t *)tr->object->ptr;
		itable_t *a;
		for(a = tt->begin; a != tt->end; a = a->next){
			table_rpush(tbl, (tbval_t)a->value);
		}
	} else {
		table_rpush(tbl, (tbval_t)tr->object);
	}
	obj->ptr = tbl;
	tr->object = obj;
	return c->next;
}


iarray_t *
decode(thread_t *tr, iarray_t *c) {

	//printf("thread %ld\n", c->value);

	switch (c->value) {
		case NUL:
			return c->next;
			break;
		case BLP:
			return c->next;
			break;
		case ELP:
			return c->next;
			break;
		case BSCP:
			return thread_bscp(tr, c);
			break;
		case ESCP:
			return thread_escp(tr, c);
			break;

		case OR:
			return thread_or(tr, c);
			break;
		case LOR:
			return thread_lor(tr, c);
			break;
		case XOR:
			return thread_xor(tr, c);
			break;
		case AND:
			return thread_and(tr, c);
			break;
		case LAND:
			return thread_land(tr, c);
			break;
		case EQ:
			return thread_eq(tr, c);
			break;
		case NE:
			return thread_ne(tr, c);
			break;
		case LT:
			return thread_lt(tr, c);
			break;
		case LE:
			return thread_le(tr, c);
			break;
		case GT:
			return thread_gt(tr, c);
			break;
		case GE:
			return thread_ge(tr, c);
			break;
		case LSHIFT:
			return thread_lshift(tr, c);
			break;
		case RSHIFT:
			return thread_rshift(tr, c);
			break;
		case ADD:
			return thread_add(tr, c);
			break;
		case SUB:
			return thread_sub(tr, c);
			break;
		case MUL:
			return thread_mul(tr, c);
			break;
		case DIV:
			return thread_div(tr, c);
			break;
		case MOD:
			return thread_mod(tr, c);
			break;

			// VAR IMM DATA SD LD PUSH JMP JZ JNZ CENT CLEV CALL ENT LEV THIS SUPER DOT
			// SIZEOF TYPEOF DELETE INSERT COUNT BREAK NEW REF RET
		case IMM:
			return thread_imm(tr, c);
			break;
		case SD:
			return thread_sd(tr, c);
			break;
		case LD:
			return thread_ld(tr, c);
			break;
		case PUSH:
			return thread_push(tr, c);
			break;
		case JMP:
			return thread_jmp(tr, c);
			break;
		case JZ:
			return thread_jz(tr, c);
			break;
		case JNZ:
			return thread_jnz(tr, c);
			break;
		case COMMA:
			return thread_comma(tr, c);
			break;
		case CALL:
			return thread_call(tr, c);
			break;
		case CELL:
			return thread_cell(tr, c);
			break;
		case ENT:
			return thread_ent(tr, c);
			break;
		case EXTND:
			return thread_extend(tr, c);
			break;
		case LEV:
			return thread_lev(tr, c);
			break;
		case REL:
			return thread_rel(tr, c);
			break;
		case SIM:
			return thread_sim(tr, c);
			break;
		case CLS:
			return thread_scheming(tr, c);
			break;
		case RET:
			return thread_ret(tr, c);
			break;

		case THIS:
			return thread_this(tr, c);
			break;
		case SUPER:
			return thread_super(tr, c);
			break;
		case PRTF:
			return thread_print(tr, c);
			break;
		case ARRAY:
			return thread_array(tr, c);
			break;

		case EXIT:
			return c->next;
			break;

		default:
			printf("unknown instruction %ld\n", c->value);
            exit(-1);
	}

	return c;
}


void
eval(schema_t *root, array_t *code)
{
	thread_t *tr = thread_create();

	iarray_t *adrs = array_rpush(code, EXIT);

	iarray_t *c = call(tr, root, adrs);

	do {
		c = decode(tr, c);
	} while (c != code->end);
}




int
main(int argc, char **argv)
{
	argc--;
    argv++;

    // parse arguments
    if (argc < 1) {
        printf("usage: file ...\n");
        return -1;
    }

    arval_t i;
    FILE *fd;

	char destination [ MAX_PATH ];

#ifdef WIN32
	if(**argv != '\\'){
#else
	if(**argv != '/'){
#endif
		char cwd[MAX_PATH];
		if (getcwd(cwd, sizeof(cwd)) == NULL) {
			perror("getcwd() error");
			return -1;
		}
		utils_combine ( destination, cwd, *argv );
	} else {
		strcpy(destination, *argv);
	}

    if (!(fd = fopen(destination, "rb"))) {
        printf("could not open(%s)\n", destination);
        return -1;
    }

    argc--;
    argv++;

    // Current position
    arval_t pos = ftell(fd);
    // Go to end
    fseek(fd, 0, SEEK_END);
    // read the position which is the size
    arval_t chunk = ftell(fd);
    // restore original position
    fseek(fd, pos, SEEK_SET);

    char *buf;

    if (!(buf = malloc(chunk + 1))) {
        printf("could not malloc(%ld) for buf area\n", chunk);
        return -1;
    }

    // read the source file
    if ((i = fread(buf, 1, chunk, fd)) < chunk) {
        printf("read returned %ld\n", i);
        return -1;
    }

    buf[i] = '\0';

    fclose(fd);

    table_t *tokens = table_create();
    lexer(tokens, buf);
    qalam_free(buf);

    array_t *code = array_create();
    parser_t *parser = parse(tokens, code);

    eval(parser->schema, code);

    return 0;
}
