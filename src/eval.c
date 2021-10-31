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
#include "parser.h"
#include "data.h"



typedef struct thread {
	schema_t *schema;

	table_t *frame;
	table_t *board;

	object_t *object;
} thread_t;

thread_t *
thread_create(schema_t *schema){
	thread_t *tr = qalam_malloc(sizeof(thread_t));

	tr->schema = schema_duplicate(schema);

	tr->object = nullptr;
	tr->frame = table_create();
	tr->board = table_create();

	return tr;
}

//OR LOR XOR AND LAND EQ NE LT LE GT GE LSHIFT RSHIFT ADD SUB MUL DIV MOD
iarray_t *
call(thread_t *tr, schema_t *schema, iarray_t *adrs) {
	object_t *object;
	validate_format(!!(object = object_define(TP_ADRS, sizeof(long64_t))), 
		"unable to alloc memory!");
	object->ptr = (tbval_t)adrs;

	table_rpush(tr->schema->frame, (tbval_t)object);
	table_rpush(tr->board, (tbval_t)schema_duplicate(schema));

	return (iarray_t *)schema->start;
}



iarray_t *
thread_or(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr || *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_lor(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr | *(long64_t *)tr->object->ptr;

	// table_rpush(tr->pool, (tbval_t)tr->object);

	tr->object = object;

	return c->next;
}

iarray_t *
thread_xor(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(double_t))), 
		"unable to alloc memory!");

	*(long64_t *)object->ptr = *(long64_t *)esp->ptr ^ *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_and(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr && *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_land(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr & *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_eq(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr == *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_ne(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr != *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_lt(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");

	*(long64_t *)object->ptr = *(long64_t *)esp->ptr < *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_le(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr <= *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_gt(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr > *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_ge(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr >= *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_lshift(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr << *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_rshift(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr >> *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_add(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(double64_t))), 
		"unable to alloc memory!");
	
	*(double64_t *)object->ptr = *(double64_t *)esp->ptr + *(double64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_sub(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(double64_t))), 
		"unable to alloc memory!");
	
	*(double64_t *)object->ptr = *(double64_t *)esp->ptr - *(double64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_mul(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(double_t))), 
		"unable to alloc memory!");
	
	*(double64_t *)object->ptr = *(double64_t *)esp->ptr * *(double64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_div(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"frame is empty");

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(double64_t))), 
		"unable to alloc memory!");

	*(double64_t *)object->ptr = *(double64_t *)esp->ptr / *(double64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_mod(thread_t *tr, iarray_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)table_content(table_rpop(tr->schema->frame)))){
		printf("[%%], you can use '%%' between two object!\n");
		exit(-1);
	}

	validate_format(esp->type == TP_NUMBER && tr->object->type == TP_NUMBER, 
		"object type is wrong %d && %d", esp->type, tr->object->type);

	object_t *object;

	validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
		"unable to alloc memory!");
	
	*(long64_t *)object->ptr = *(long64_t *)esp->ptr % *(long64_t *)tr->object->ptr;

	tr->object = object;

	return c->next;
}

iarray_t *
thread_comma(thread_t *tr, iarray_t *c){
	object_t *esp;
	if(!(esp = (object_t *)table_content(table_rpop(tr->schema->frame)))){
		printf("[,] you can use ',' between two object!\n");
		exit(-1);
	}
	object_t *obj;
	if (esp->type != TP_PARAMS) {
		validate_format(!!(obj = object_define(TP_PARAMS, sizeof(table_t))), 
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

iarray_t *
thread_sim(thread_t *tr, iarray_t *c){
	validate_format(!!(tr->object->type = TP_SCHEMA), 
		"[SIM] object is empty");
	schema_t *schema;
	validate_format(!!(schema = (schema_t *)tr->object->ptr), 
		"[SIM] object is empty");
	if(schema->type == SCHEMA_DUP){
		table_rpush(tr->frame, (tbval_t)tr->schema);
		tr->schema = schema;
		return c->next;
	}
	else if(schema->type == SCHEMA_PRIMARY){
		table_rpush(tr->frame, (tbval_t)tr->schema);
		tr->schema = schema_duplicate(schema);
		return c->next;
	}
	return c->next;
}

iarray_t *
thread_rel(thread_t *tr, iarray_t *c){
	validate_format(!!(tr->schema = (schema_t *)table_content(table_rpop(tr->frame))), 
		"schema rel back frame is empty");
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
	if(c->value == TP_VAR){
		variable_t *var;
		if(!(var = schema_variable(tr->schema, (char *)value))){
			schema_t *schema;
			if(!!(schema = schema_branches(tr->schema, (char *)value))){
				object_t *object;
				validate_format(!!(object = object_define(TP_SCHEMA, sizeof(schema_t))), 
					"[IMM] schema object not created");
				tr->object = object;
				return c->next;
			}
			var = variable_define((char *)value);
			var->object = object_define(TP_NULL, sizeof(ptr_t));
			table_rpush(tr->schema->variables, (tbval_t)var);
		}
		validate_format(!!(tr->object = variable_content(var)),
			"[IMM] variable dnot have content");
		return c->next;
	}
	else if(c->value == TP_ARRAY){
        object_t *object;
        validate_format(!!(object = object_define(TP_ARRAY, sizeof(table_t))), 
            "unable to alloc memory!");
        object->ptr = data_from((char_t *)value);
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_NUMBER){
        object_t *object;
        validate_format(!!(object = object_define(TP_NUMBER, sizeof(double64_t))), 
            "unable to alloc memory!");
        *(double64_t *)object->ptr = utils_atof((char *)value);
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_IMM){
        object_t *object;
        validate_format(!!(object = object_define(TP_NUMBER, sizeof(long64_t))), 
            "unable to alloc memory!");
        *(long64_t *)object->ptr = (long64_t)value;
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_NULL){
        object_t *object;
        validate_format(!!(object = object_define(TP_NULL, sizeof(ptr_t))), 
            "unable to alloc memory!");
        object->ptr = nullptr;
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_SCHEMA){
        object_t *object;
        validate_format(!!(object = object_define(TP_SCHEMA, sizeof(schema_t))), 
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
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))), 
		"save data, bad pop data");
	if(esp->type != TP_SCHEMA){
		object_assign(esp, tr->object);
		return c->next;
	}
	return call(tr, (schema_t *)esp->ptr, c->next);
}

iarray_t *
thread_ld(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!(esp = (object_t *)table_content(table_rpop(tr->schema->parameters))),
		"load data, bad pop data!");
	tr->object = esp;
	return c->next;
}

iarray_t *
thread_push(thread_t *tr, iarray_t *c) {
	// push the value of object onto the frames
	table_rpush(tr->schema->frame, (tbval_t)tr->object);
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
	return (*(double_t *)tr->object->ptr) ? c->next : (iarray_t *)c->value;
}

iarray_t *
thread_jnz(thread_t *tr, iarray_t *c) {
	// jump if object is not zero
	c = c->next;
	return (*(double_t *)tr->object->ptr) ? (iarray_t *)c->value : c->next;
}



iarray_t *
thread_this(thread_t *tr, iarray_t *c) {
	object_t *object;
	validate_format(!!(object = object_define(TP_SCHEMA, sizeof(schema_t))), 
		"unable to alloc memory!");
	object->ptr = (tbval_t)tr->schema;
	tr->object = object;
	return c->next;
}

iarray_t *
thread_super(thread_t *tr, iarray_t *c) {
	object_t *object;
	validate_format(!!(object = object_define(TP_SCHEMA, sizeof(schema_t))), 
		"unable to alloc memory!");
	object->ptr = (tbval_t)tr->schema->parent;
	tr->object = object;
	return c->next;
}

iarray_t *
thread_cgt(thread_t *tr, iarray_t *c) {
	validate_format((tr->object->type == TP_SCHEMA), 
		"cgt, diffrent type of object %d\n", tr->object->type);

	schema_t *schema;
	validate_format(!!(schema = (schema_t *)tr->object->ptr), 
		"schema not defined");

	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))), 
		"cgt, null argument\n");

	if(esp->type == TP_PARAMS){
		table_t *tbl = (table_t *)esp->ptr;
		itable_t *b;
		for(b = tbl->begin; b != tbl->end; b = b->next){
			table_rpush(schema->parameters, (tbval_t)b->value);
		}
	} else {
		table_rpush(schema->parameters, (tbval_t)esp);
	}

	return c->next;
}




iarray_t *
thread_call(thread_t *tr, iarray_t *c) {
	object_t *esp;
	if(tr->object->type == TP_PARAMS){
		validate_format(!!(esp = (object_t *)table_content(table_lpop((table_t *)tr->object->ptr))),
			"bad call operation!");
	} else {
		esp = tr->object;
		tr->object = nullptr;
	}

	validate_format((esp->type == TP_SCHEMA), 
		"[CALL] for %s type, eval operator only use for SCHEMA type", 
		object_typeAsString(esp->type));
		
	return call(tr, (schema_t *)esp->ptr, c->next);
}

iarray_t *
thread_ent(thread_t *tr, iarray_t *c) {
	table_rpush(tr->frame, (tbval_t)tr->schema);

	tr->schema = (schema_t *)table_content(table_rpop(tr->board));

	tr->schema->object = object_define(TP_NULL, sizeof(ptr_t));

	validate_format((tr->schema->type == SCHEMA_DUP), 
		"schema before called");

	itable_t *b;
	for(b = tr->schema->parameters->begin; b != tr->schema->parameters->end; b = b->next){
		object_t *target = object_clone((object_t *)b->value);
		if(!!tr->object){
			if(tr->object->type == TP_PARAMS){
				object_t *source = (object_t *)table_rpop((table_t *)tr->object->ptr);
				if(source){
					object_assign(target, source);
				}
			}else{
				object_assign(target, tr->object);
				tr->object = nullptr;
			}
		}
	}

	return c->next;
}

iarray_t *
thread_head(thread_t *tr, iarray_t *c){
	if(!tr->schema->stack){
		tr->schema->stack = table_create();
	}
	table_t *extends = tr->schema->extends;
	itable_t *b;
	for(b = extends->begin; b != extends->end; b = b->next){
		schema_t *schema = (schema_t *)b->value;
		if(!schema_fpt(tr->schema->stack, schema->identifier)){
			table_rpush(tr->schema->stack, (tbval_t)schema);
			schema = schema_duplicate(schema);
			schema->variables = tr->schema->variables;

			object_t *object;
			validate_format(!!(object = object_define(TP_ADRS, sizeof(long64_t))), 
				"unable to alloc memory!");
			object->ptr = (tbval_t)c;

			table_rpush(tr->schema->frame, (tbval_t)object);
			table_rpush(tr->board, (tbval_t)schema);

			return (iarray_t *)schema->start;
		}
	}
	table_destroy(tr->schema->stack);
	return c->next;
}

iarray_t *
thread_lev(thread_t *tr, iarray_t *c) {
	tr->object = tr->schema->object;
	tr->schema = (schema_t *)table_content(table_rpop(tr->frame));

	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"wrong! bad leave function, not found adrs");

	validate_format((esp->type == TP_ADRS), 
		"wrong! object not from type address");
	
	iarray_t * adrs;
	adrs = (iarray_t *)esp->ptr;

	return adrs;
}

iarray_t *
thread_ret(thread_t *tr, iarray_t *c) {
	tr->schema->object = tr->object;
	return c->next;
}

iarray_t *
thread_def(thread_t *tr, iarray_t *c) {
	object_t *esp;
	validate_format(!!(esp = (object_t *)table_content(table_rpop(tr->schema->frame))),
		"[DEF] missing object");

	validate_format((esp->type == TP_SCHEMA) || (esp->type == TP_NULL) || (esp->type == TP_NUMBER), 
		"[DEF] def type use only for null or schema type, %s", object_typeAsString(esp->type));

	if(((esp->type == TP_NULL) || (esp->type == TP_NUMBER)) && (tr->object->type == TP_NUMBER)){
		esp = object_redefine(
			esp, 
			esp->type, 
			*(double_t *)tr->object->ptr
		);
		tr->object = esp;
		return c->next;
	}

	if(esp->type == TP_NULL){
		validate_format(!!(esp = object_redefine(esp, TP_SCHEMA, sizeof(schema_t))), 
			"[DEF] redefine object for type schema");

		if(tr->object->type == TP_PARAMS) {
			object_t *object;
			validate_format(!!(object = (object_t *)table_content(table_rpop((table_t *)tr->object->ptr))), 
				"[DEF] pop schema fram list ignored");
			validate_format((object->type == TP_SCHEMA), 
				"[DEF] def type use only for schema type, %s", 
				object_typeAsString(object->type)
			);
			esp->ptr = object->ptr;
		} else {
			validate_format((tr->object->type == TP_SCHEMA), 
				"[DEF] def type use only for schema type, %s", 
				object_typeAsString(tr->object->type)
			);
			esp->ptr = tr->object->ptr;
			tr->object = esp;
			return c->next;
		}
	}

	schema_t *schema;
	validate_format(!!(schema = (schema_t *)esp->ptr),
		"[DEF] schema is null");

	if(tr->object->type == TP_PARAMS){
		table_t *tbl = (table_t *)tr->object->ptr;
		itable_t *b;
		for(b = tbl->begin; b != tbl->end; b = b->next){
			object_t *object = (object_t *)b->value;
			validate_format((object->type == TP_SCHEMA), 
				"[DEF] schema type for def operator is required %s", object_typeAsString(object->type));
			table_rpush(schema->extends, (tbval_t)object->ptr);
		}
	} 
	else if(tr->object->type == TP_SCHEMA) {
		validate_format((tr->object->type == TP_SCHEMA), 
			"[DEF] schema type for def operator is required %s", object_typeAsString(tr->object->type));
		table_rpush(schema->extends, (tbval_t)tr->object->ptr);
	}
	else if(tr->object->type == TP_NUMBER) {
		schema->object = object_redefine(
			schema->object, 
			schema->object->type, 
			*(double_t *)tr->object->ptr
		);
	}

	tr->object = esp;
	return c->next;
}

iarray_t *
thread_print(thread_t *tr, iarray_t *c) {
	object_t *esp;
	table_t *tbl;

	if(tr->object->type == TP_PARAMS){
		tbl = (table_t *)tr->object->ptr;
	} else {
		tbl = table_create();
		table_rpush(tbl, (tbval_t)tr->object);
	}

	arval_t i = table_count(tbl);

	while((esp = (object_t *)table_content(table_rpop(tbl)))){
		if(esp->type == TP_NULL){
			printf("%s ", object_typeAsString(TP_NULL));
			continue;
		}
		else if(esp->type == TP_CHAR){
			printf("%d ", *(char_t *)esp->ptr);
			continue;
		}
		else if(esp->type == TP_ARRAY){
			table_t *formated;
			if(i > 0){
				formated = data_format((table_t *)esp->ptr, tbl);
			}
			else {
				formated = (table_t *)esp->ptr;
			}

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
		else if(esp->type == TP_NUMBER){
			if((*(double64_t *)esp->ptr - *(long64_t *)esp->ptr) != 0){
				printf("%.16Lf", *(double64_t *)esp->ptr);
			}else{
				printf("%lld", *(long64_t *)esp->ptr);
			}
			continue;
		}
		else if(esp->type == TP_SCHEMA) {
			printf("SCHEMA \n");
		}
		else {
			printf("%.16Lf", *(double64_t *)esp->ptr);
		}
	}
	printf("\n");

	return c->next;
}





iarray_t *
decode(thread_t *tr, iarray_t *c) {

	// printf("thread %lld\n", c->value);

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
		case FOOT:
			return c->next;
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
		case ENT:
			return thread_ent(tr, c);
			break;
		case HEAD:
			return thread_head(tr, c);
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
		case CGT:
			return thread_cgt(tr, c);
			break;
		case DEF:
			return thread_def(tr, c);
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

		case EXIT:
			return c->next;
			break;

		default:
			printf("unknown instruction %lld\n", c->value);
            exit(-1);
	}

	return c;
}

void
eval(schema_t *root, array_t *code)
{
	thread_t *tr = thread_create(root);

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
        printf("could not malloc(%lld) for buf area\n", chunk);
        return -1;
    }

    // read the source file
    if ((i = fread(buf, 1, chunk, fd)) < chunk) {
        printf("read returned %lld\n", i);
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
