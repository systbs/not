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
#include <unistd.h>
#include <sys/wait.h>

#ifdef WIN32
	#include <conio.h>
#else
	#include "conio.h"
#endif

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


typedef struct vm {
	layout_t *layout;

	list_t *frame;
	list_t *board;

	list_t *variables;
	long_t dotted;

	object_t *object;
} vm_t;

vm_t *
ircode_create(){
	vm_t *tr;
	assert_format(!!(tr = qalam_malloc(sizeof(vm_t))), 
		"unable to malloc root thread");

	tr->layout = layout_create(nullptr);

	tr->frame = list_create();
	tr->board = list_create();
	tr->dotted = 0;

	tr->variables = list_create();

	tr->object = nullptr;
	return tr;
}

//OR LOR XOR AND LAND EQ NE LT LE GT GE LSHIFT RSHIFT ADD SUB MUL DIV MOD
ilist_t *
call(vm_t *tr, schema_t *schema, ilist_t *adrs) {
	object_t *object;
	assert_format(!!(object = object_define(OTP_ADRS, sizeof(long_t))), 
		"unable to alloc memory!");
	object->ptr = (ptr_t)adrs;

	list_rpush(tr->layout->frame, (list_value_t)object);
	list_rpush(tr->board, (list_value_t)layout_create(schema));

	return (ilist_t *)schema->start;
}

ilist_t *
decode(vm_t *tr, ilist_t *c);

object_t *
import(schema_t *schema, list_t *code)
{
	vm_t *tr = ircode_create();
	ilist_t *adrs = list_rpush(code, NUL);

	ilist_t *c = call(tr, schema, adrs);
	
	do {
		c = decode(tr, c);
	} while (c != code->end);

	return tr->object;
}


ilist_t *
ircode_or(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");
	
	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "||")), 
			"[OR] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[OR] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "||")), 
			"[OR] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[OR] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));
	
	object_t *object;
	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) || oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_lor(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "|")), 
			"[LOR] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LOR] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "|")), 
			"[LOR] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LOR] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) | (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_xor(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "^")), 
			"[XOR] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[XOR] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "^")), 
			"[XOR] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[XOR] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");

	*(long_t *)object->ptr = (long_t)oget(esp) ^ (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_and(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "&&")), 
			"[AND] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[AND] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "&&")), 
			"[AND] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[AND] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) && oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_land(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "&")), 
			"[LAND] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LAND] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "&")), 
			"[LAND] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LAND] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) & (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_eq(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");

	*(long_t *)object->ptr = oget(esp) == oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_ne(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (oget(esp) != oget(tr->object));

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_lt(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "<")), 
			"[LT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "<")), 
			"[LT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
	*(long_t *)object->ptr = oget(esp) < oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_le(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "<=")), 
			"[LE] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LE] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "<=")), 
			"[LE] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LE] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) <= oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_gt(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, ">")), 
			"[GT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[GT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, ">")), 
			"[GT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[GT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) > oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_ge(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, ">=")), 
			"[GE] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[GE] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, ">=")), 
			"[GE] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[GE] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = oget(esp) >= oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_lshift(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "<<")), 
			"[LSHIFT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LSHIFT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "<<")), 
			"[LSHIFT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[LSHIFT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)),
		"in << operator used object %s && %s", 
		object_tas(esp), object_tas(tr->object)
	);

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) << (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_rshift(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, ">>")), 
			"[RSHIFT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[RSHIFT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, ">>")), 
			"[RSHIFT] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[RSHIFT] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)),
		"in >> operator used object %s && %s", 
		object_tas(esp), object_tas(tr->object)
	);

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	
	*(long_t *)object->ptr = (long_t)oget(esp) >> (long_t)oget(tr->object);

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_add(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "+")), 
			"[ADD] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[ADD] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "+")), 
			"[ADD] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[ADD] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = oget(esp) + oget(tr->object);
	} else {
		assert_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = oget(esp) + oget(tr->object);
	}

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_sub(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "-")), 
			"[SUB] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[SUB] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "-")), 
			"[SUB] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[SUB] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = oget(esp) - oget(tr->object);
	} else {
		assert_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = oget(esp) - oget(tr->object);
	}

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_mul(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "*")), 
			"[MUL] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[MUL] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "*")), 
			"[MUL] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[MUL] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = oget(esp) * oget(tr->object);
	} else {
		assert_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = oget(esp) * oget(tr->object);
	}

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_div(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"frame is empty");

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "/")), 
			"[DIV] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[DIV] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "/")), 
			"[DIV] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[DIV] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = (long_t)(oget(esp) / oget(tr->object));
	} else {
		assert_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = (double_t)(oget(esp) / oget(tr->object));
	}

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_mod(vm_t *tr, ilist_t *c) {
	object_t *esp;
	if(!(esp = (object_t *)list_content(list_rpop(tr->layout->frame)))){
		printf("[%%], you can use '%%' between two object!\n");
		exit(-1);
	}

	if (esp->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)esp->ptr;

		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;
		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "%")), 
			"[MOD] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[MOD] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	else if (tr->object->type == OTP_LAYOUT) {
		layout_t *layout = (layout_t *)tr->object->ptr;
		
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);

		object_t *object = object_define(OTP_TUPLE, sizeof(list_t));
		object->ptr = tbl;

		tr->object = object;

		variable_t *var;
		assert_format(!!(var = layout_vwp(layout, "%")), 
			"[MOD] operator not defined");

		object = variable_content(var);

		assert_format(!!(object->type == OTP_SCHEMA), 
			"[MOD] operator is diffrent type of schema");

		return call(tr, (schema_t *)object->ptr, c->next);
	}

	assert_format(!!(object_isnum(esp) && object_isnum(tr->object)),
		"wrong in object type %s && %s", object_tas(esp), object_tas(tr->object));

	object_t *object;

	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
	if((esp->type == OTP_LONG) && (tr->object->type == OTP_LONG)){
		assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = (long_t)((long_t)oget(esp) % (long_t)oget(tr->object));
	} else {
		assert_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
			"unable to alloc memory!");
		*(double_t *)object->ptr = (double_t)((long_t)oget(esp) % (long_t)oget(tr->object));
	}

	tr->object = object;

	return c->next;
}

ilist_t *
ircode_comma(vm_t *tr, ilist_t *c){
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"[,] you can use ',' between two object!");
	object_t *obj;
	if (esp->type != OTP_TUPLE) {
		assert_format(!!(obj = object_define(OTP_TUPLE, sizeof(list_t))), 
			"unable to alloc memory!");
		list_t *tbl = list_create();
		list_rpush(tbl, (list_value_t)esp);
		list_rpush(tbl, (list_value_t)tr->object);
		obj->ptr = tbl;
		tr->object = obj;
	} else {
		list_rpush((list_t *)esp->ptr, (list_value_t)tr->object);
		tr->object = esp;
	}
	return c->next;
}

/* 
	VAR IMM DATA SD LD PUSH JMP JZ JNZ CENT CLEV CALL ENT LEV THIS SUPER DOT
	SIZEOF TYPEOF DELETE INSERT COUNT BREAK NEW REF RET
*/

ilist_t *
ircode_imm(vm_t *tr, ilist_t *c) {
	// load immediate value to object
	c = c->next;
	long_t value = c->value;
	c = c->next;
	
	if (c->value == TP_VAR) {
		variable_t *var;
		if(tr->dotted){
			if(!!(var = layout_vwp(tr->layout, (char *)value))){
				assert_format(!!(tr->object = variable_content(var)),
					"[IMM] variable dont have content");
				return c->next;
			}
		} else if(!!(var = layout_variable(tr->layout, (char *)value))){
			assert_format(!!(tr->object = variable_content(var)),
				"[IMM] variable dont have content");
			return c->next;
		}
		schema_t *schema;
		if(!!(schema = schema_branches(tr->layout->schema, (char *)value))){
			object_t *object;
			assert_format(!!(object = object_define(OTP_SCHEMA, sizeof(schema_t))), 
				"[IMM] schema object not created");
			object->ptr = schema;
			tr->object = object;
			return c->next;
		}
		var = variable_define((char *)value);
		var->object = object_define(OTP_NULL, sizeof(ptr_t));
		list_rpush(tr->layout->variables, (list_value_t)var);
		list_rpush(tr->variables, (list_value_t)var);
		assert_format(!!(tr->object = variable_content(var)),
			"[IMM] variable dont have content");
		return c->next;
	}
	else if(c->value == TP_ARRAY){
        object_t *object;
        assert_format(!!(object = object_define(OTP_ARRAY, sizeof(list_t))), 
            "unable to alloc memory!");
        object->ptr = data_from((char_t *)value);
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_NUMBER){
        object_t *object;
		double_t num = utils_atof((char *)value);
		if(num - (long_t)num != 0){
			assert_format(!!(object = object_define(OTP_DOUBLE, sizeof(double_t))), 
            "unable to alloc memory!");
			*(double_t *)object->ptr = num;
		} else {
			assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
            "unable to alloc memory!");
			*(long_t *)object->ptr = (long_t)num;
		}
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_IMM){
        object_t *object;
        assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
            "unable to alloc memory!");
        *(long_t *)object->ptr = (long_t)value;
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_NULL){
        object_t *object;
        assert_format(!!(object = object_define(OTP_NULL, sizeof(ptr_t))), 
            "unable to alloc memory!");
        object->ptr = nullptr;
		tr->object = object;
		return c->next;
	}
	else if(c->value == TP_SCHEMA){
        object_t *object;
        assert_format(!!(object = object_define(OTP_SCHEMA, sizeof(schema_t))), 
            "unable to alloc memory!");
        object->ptr = (schema_t *)value;
		tr->object = object;
		return c->next;
	}

	assert_format( !!(c->value == TP_NULL), "[IMM] unknown type");

	return c->next;
}

ilist_t *
ircode_sd(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))), 
		"save data, bad pop data");
	if(esp->type == OTP_SCHEMA){
		return call(tr, (schema_t *)esp->ptr, c->next);
	}
	else if(esp->type == OTP_TUPLE){
		assert_format((tr->object->type == OTP_TUPLE) || (tr->object->type == OTP_LAYOUT), 
			"[SD] after assign operator must be use TUPLE type");
		if(tr->object->type == OTP_LAYOUT){

			ilist_t *a, *b, *e;
			list_t *ta, *tb, *tc;
			variable_t *var_a, *var_e;
			ta = (list_t *)esp->ptr;
			tb = tr->variables;

			layout_t *layout;
			layout = (layout_t *)tr->object->ptr;

			tc = layout->variables;

			a = ta->begin;
			ircode_sd_label_step1_layout:
			for(; a != ta->end; a = a->next){
				for(b = tb->begin; b != tb->end; b = b->next){
					var_a = (variable_t *)b->value;
					if((list_value_t)a->value == (list_value_t)var_a->object){
						goto ircode_sd_label_step2_layout;
					}
				}
			}
			goto ircode_sd_label_return_layout;

			ircode_sd_label_step2_layout:
			for(e = tc->begin; e != tc->end; e = e->next){
				var_e = (variable_t *)e->value;
				if(strncmp(var_a->identifier, var_e->identifier, max(strlen(var_a->identifier), strlen(var_e->identifier))) == 0){
					object_assign((object_t *)a->value, var_e->object);
					a = a->next;
					goto ircode_sd_label_step1_layout;
				}
			}

			a = a->next;
			goto ircode_sd_label_step1_layout;

			ircode_sd_label_return_layout:
			tr->object = esp;
			return c->next;
		}
		else {
			ilist_t *a, *b, *e, *d;
			list_t *ta, *tb, *tc;
			variable_t *var_a, *var_e;
			ta = (list_t *)esp->ptr;
			tb = (list_t *)tr->variables;
			tc = (list_t *)tr->object->ptr;

			a = ta->begin;
			ircode_sd_label_step1:
			for(; a != ta->end; a = a->next){
				for(b = tb->begin; b != tb->end; b = b->next){
					var_a = (variable_t *)b->value;
					if((list_value_t)a->value == (list_value_t)var_a->object){
						e = tc->begin;
						goto ircode_sd_label_step2;
					}
				}
			}
			goto ircode_sd_label_return;

			ircode_sd_label_step2:
			for(; e != tc->end; e = e->next){
				for(d = tb->begin; d != tb->end; d = d->next){
					var_e = (variable_t *)d->value;
					if(((list_value_t)e->value == (list_value_t)var_e->object)){
						goto ircode_sd_label_step3;
					}
				}
			}

			a = a->next;
			goto ircode_sd_label_step1;

			ircode_sd_label_step3:
			if(strncmp(var_a->identifier, var_e->identifier, max(strlen(var_a->identifier), strlen(var_e->identifier))) == 0){
				object_assign((object_t *)a->value, (object_t *)e->value);
				a = a->next;
				goto ircode_sd_label_step1;
			}

			e = e->next;
			goto ircode_sd_label_step2;

			ircode_sd_label_return:
			tr->object = esp;
			return c->next;
		}
	} else {
		object_assign(esp, tr->object);
		tr->object = esp;
		return c->next;
	}
}

ilist_t *
ircode_ld(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
			"bad retrive parameters");
	if(esp->type == OTP_TUPLE){
		if(tr->object->type == OTP_TUPLE){
			list_t *tt = (list_t *)tr->object->ptr;
			list_t *ts = (list_t *)esp->ptr;
			ilist_t *a, *b;
			for(a = tt->begin, b = ts->begin; a != tt->end; a = a->next){
				if(b == ts->end){
					break;
				}
				object_assign((object_t *)a->value, (object_t *)b->value);
			}
		} else {
			object_t *object;
			assert_format (!!(object = (object_t *)list_content(list_lpop((list_t *)esp->ptr))),
				"bad retrive parameters");
			object_assign(tr->object, object);
		}
	} else {
		if(tr->object->type == OTP_TUPLE){
			object_t *object;
			assert_format (!!(object = (object_t *)list_content(list_lpop((list_t *)tr->object->ptr))),
				"bad retrive parameters");
			object_assign(object, esp);
		} else {
			object_assign(tr->object, esp);
		}
	}
	
	return c->next;
}

ilist_t *
ircode_push(vm_t *tr, ilist_t *c) {
	// push the value of object onto the frames
	list_rpush(tr->layout->frame, (list_value_t)tr->object);
	return c->next;
}

ilist_t *
ircode_jmp(vm_t *tr, ilist_t *c) {
	// jump to the address
	c = c->next;
	return (ilist_t *)c->value;
}

ilist_t *
ircode_jz(vm_t *tr, ilist_t *c) {
	// jump if object is zero
	c = c->next;
	long_t res = 1;
	if(tr->object->type == OTP_TUPLE){
		ilist_t * a;
		list_t *ta = tr->object->ptr;
		for(a = ta->begin; a != ta->end; a = a->next){
			object_t *obj = (object_t *)a->value;
			if(object_isnum(obj)){
				res = res && (!!oget(obj));
			} else {
				res = res && (!!obj->ptr);
			}
		}
	}
	else if(tr->object->type == OTP_LONG || tr->object->type == OTP_DOUBLE){
		res = res && ((long_t)oget(tr->object));
	}
	else {
		res = res && !!(tr->object->ptr);
	}
	return res ? c->next : (ilist_t *)c->value;
}

ilist_t *
ircode_jnz(vm_t *tr, ilist_t *c) {
	// jump if object is not zero
	c = c->next;
	long_t res = 1;
	if(tr->object->type == OTP_TUPLE){
		ilist_t * a;
		list_t *ta = tr->object->ptr;
		for(a = ta->begin; a != ta->end; a = a->next){
			object_t *obj = (object_t *)a->value;
			if(object_isnum(obj)){
				res = res && (!!oget(obj));
			} else {
				res = res && (!!obj->ptr);
			}
		}
	}
	else if(tr->object->type == OTP_LONG || tr->object->type == OTP_DOUBLE){
		res = res && ((long_t)oget(tr->object));
	}
	else {
		res = res && !!(tr->object->ptr);
	}
	return res ? (ilist_t *)c->value : c->next;
}



ilist_t *
vm_this(vm_t *tr, ilist_t *c) {
	object_t *object;
	assert_format(!!(object = object_define(OTP_LAYOUT, sizeof(layout_t))), 
		"unable to alloc memory!");
	object->ptr = (ptr_t)tr->layout;
	tr->object = object;
	return c->next;
}

ilist_t *
ircode_super(vm_t *tr, ilist_t *c) {
	object_t *object;
	assert_format(!!(object = object_define(OTP_LAYOUT, sizeof(layout_t))), 
		"unable to alloc memory!");
	object->ptr = (ptr_t)(!!tr->layout->parent ? tr->layout->parent : tr->layout);
	tr->object = object;
	return c->next;
}


ilist_t *
ircode_cell(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
			"bad get operation!");

	assert_format((esp->type == OTP_ARRAY) || (esp->type == OTP_TUPLE), 
		"[CELL] for %s type, get array operator only use for ARRAY/TUPLE type", object_tas(esp));

	assert_format(!!(tr->object->type == OTP_ARRAY), 
		"[CELL] for %s type, argument must be an array", object_tas(tr->object));

	object_t *obj;
	assert_format(!!(obj = (object_t *)list_content(list_lpop((list_t *)tr->object->ptr))), 
		"[CELL] object not retrived");

	assert_format(!!object_isnum(obj), 
		"[CELL] for %s type, index must be a number type", object_tas(obj));

	tr->object = (object_t *)list_content(list_at((list_t *)esp->ptr, oget(obj)));
	return c->next;
}

ilist_t *
ircode_call(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"bad call operation!");

	assert_format((esp->type == OTP_SCHEMA), 
		"[CALL] for %s type, eval operator only use for SCHEMA type", object_tas(esp));
		
	return call(tr, (schema_t *)esp->ptr, c->next);
}

ilist_t *
ircode_ent(vm_t *tr, ilist_t *c) {
	list_rpush(tr->frame, (list_value_t)tr->layout);

	layout_t *layout;
	layout = (layout_t *)list_content(list_rpop(tr->board));
	layout->parent = (layout_t *)layout->schema->root;
	tr->layout = layout;

	tr->layout->object = object_define(OTP_NULL, sizeof(ptr_t));

	return c->next;
}

ilist_t *
ircode_extend(vm_t *tr, ilist_t *c){
	ilist_t *b, *n;
	for(b = tr->layout->schema->extends->begin; b != tr->layout->schema->extends->end; b = b->next){
		schema_t *schema = (schema_t *)b->value;
		int found = 0;
		for(n = tr->layout->extends->begin; n != tr->layout->extends->end; n = n->next){
			layout_t *layout = (layout_t *)n->value;
			if(strncmp(layout->schema->identifier, schema->identifier, max(strlen(layout->schema->identifier), strlen(schema->identifier))) == 0){
				found = 1;
				break;
			}
		}
		if(found){
			continue;
		}
		layout_t *layout = layout_create(schema);
		object_t *object;
		assert_format(!!(object = object_define(OTP_ADRS, sizeof(long_t))), 
			"unable to alloc memory!");
		object->ptr = (ptr_t)c;
		list_rpush(tr->layout->frame, (list_value_t)object);
		list_rpush(tr->layout->extends, (list_value_t)layout);
		list_rpush(tr->board, (list_value_t)layout);
		return (ilist_t *)schema->start;
	}
	
	return c->next;
}

ilist_t *
ircode_lev(vm_t *tr, ilist_t *c) {
	tr->object = tr->layout->object;
	tr->layout = (layout_t *)list_content(list_rpop(tr->frame));

	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"wrong! bad leave function, not found adrs");

	assert_format((esp->type == OTP_ADRS), 
		"wrong! object not from type address");
	
	ilist_t * adrs;
	adrs = (ilist_t *)esp->ptr;

	return adrs;
}

ilist_t *
ircode_ret(vm_t *tr, ilist_t *c) {
	tr->layout->object = tr->object;
	return c->next;
}

ilist_t *
ircode_scheming(vm_t *tr, ilist_t *c) {
	object_t *esp;
	assert_format(!!(esp = (object_t *)list_content(list_rpop(tr->layout->frame))),
		"[CLS] missing object");

	assert_format((esp->type == OTP_SCHEMA) || (esp->type == OTP_NULL) || (esp->type == OTP_ARRAY), 
		"[CLS] scheming type use only for null or schema type, %s", object_tas(esp));

	if((esp->type == OTP_ARRAY)){
		char_t *schema_name = data_to((list_t *)esp->ptr);
		
		assert_format(!!(esp = object_redefine(esp, OTP_SCHEMA, sizeof(schema_t))), 
			"[CLS] redefine object for type schema");

		variable_t *var;
		var = variable_define(schema_name);
		var->object = esp;
		list_rpush(tr->layout->variables, (list_value_t)var);
		list_rpush(tr->variables, (list_value_t)var);
	}

	if((esp->type == OTP_NULL)){
		assert_format(!!(esp = object_redefine(esp, OTP_SCHEMA, sizeof(schema_t))), 
			"[CLS] redefine object for type schema");
		if(tr->object->type == OTP_TUPLE) {
			object_t *object;
			assert_format(!!(object = (object_t *)list_content(list_rpop((list_t *)tr->object->ptr))), 
				"[CLS] pop schema fram list ignored");
			assert_format((object->type == OTP_SCHEMA), 
				"[CLS] scheming type use only for schema type, %s", 
				object_tas(object)
			);
			esp->ptr = object->ptr;
		} else {
			assert_format((tr->object->type == OTP_SCHEMA), 
				"[CLS] scheming type use only for schema type, %s", object_tas(tr->object)
			);
			esp->ptr = tr->object->ptr;
			schema_t *schema;
			assert_format(!!(schema = (schema_t *)esp->ptr),
				"[CLS] schema is null");
			schema->root = tr->layout;
			tr->object = esp;
			return c->next;
		}
	}

	schema_t *schema;
	assert_format(!!(schema = (schema_t *)esp->ptr),
		"[CLS] schema is null");

	schema->root = tr->layout;

	if(tr->object->type == OTP_TUPLE){
		list_t *tbl = (list_t *)tr->object->ptr;
		ilist_t *b;
		for(b = tbl->begin; b != tbl->end; b = b->next){
			object_t *object = (object_t *)b->value;
			assert_format((object->type == OTP_SCHEMA), 
				"[CLS] schema type for scheming operator is required %s", object_tas(object));
			list_rpush(schema->extends, (list_value_t)object->ptr);
		}
	} 
	else if(tr->object->type == OTP_SCHEMA) {
		assert_format((tr->object->type == OTP_SCHEMA), 
			"[CLS] schema type for scheming operator is required %s", object_tas(tr->object));
		list_rpush(schema->extends, (list_value_t)tr->object->ptr);
	}

	tr->object = esp;
	return c->next;
}

ilist_t *
ircode_sim(vm_t *tr, ilist_t *c){
	assert_format(!!((tr->object->type == OTP_SCHEMA) || (tr->object->type == OTP_LAYOUT)), 
		"[SIM] object is empty");
	tr->dotted = 1;
	if(tr->object->type == OTP_SCHEMA){
		schema_t *schema;
		assert_format(!!(schema = (schema_t *)tr->object->ptr), 
			"[SIM] object is empty");
		list_rpush(tr->frame, (list_value_t)tr->layout);
		tr->layout = layout_create(schema);
		return c->next;
	}
	else {
		layout_t *layout;
		assert_format(!!(layout = (layout_t *)tr->object->ptr), 
			"[SIM] object is empty");
		list_rpush(tr->frame, (list_value_t)tr->layout);
		tr->layout = layout;
		return c->next;
	}
}

ilist_t *
ircode_rel(vm_t *tr, ilist_t *c){
	tr->dotted = 0;
	assert_format(!!(tr->layout = (layout_t *)list_content(list_rpop(tr->frame))), 
		"layout rel back frame is empty");
	return c->next;
}

ilist_t *
ircode_print(vm_t *tr, ilist_t *c) {
	list_t *tbl;
	if(tr->object->type == OTP_TUPLE){
		tbl = (list_t *)tr->object->ptr;
	} else {
		tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);
	}

	object_t *eax;
	while((eax = (object_t *)list_content(list_lpop(tbl)))){
		if(eax->type == OTP_NULL){
			printf("%s ", object_tas(eax));
			continue;
		}
		else if(eax->type == OTP_CHAR){
			printf("%d ", *(char_t *)eax->ptr);
			continue;
		}
		else if(eax->type == OTP_ARRAY){
			list_t *formated;

			formated = data_format((list_t *)eax->ptr, tbl);

			ilist_t *t;
			while((t = list_lpop(formated))){
				object_t *obj = (object_t *)t->value;
				if(*(char_t *)obj->ptr == '\\'){

					t = list_lpop(formated);
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
		else if(eax->type == OTP_LONG){
			printf("%ld", *(long_t *)eax->ptr);
			continue;
		}
		else if(eax->type == OTP_DOUBLE){
			printf("%.6f", *(double_t *)eax->ptr);
			continue;
		}
		else if(eax->type == OTP_SCHEMA) {
			printf("SCHEMA \n");
		}
		else {
			printf("%ld", *(long_t *)eax->ptr);
		}
	}
	return c->next;
}

ilist_t *
ircode_bscp(vm_t *tr, ilist_t *c) {
	list_rpush(tr->layout->scope, (list_value_t)tr->layout->variables);
	tr->layout->variables = list_create();
	return c->next;
}

ilist_t *
ircode_escp(vm_t *tr, ilist_t *c) {
	tr->layout->variables = (list_t *)list_content(list_rpop(tr->layout->scope));
	return c->next;
}

ilist_t *
ircode_array(vm_t *tr, ilist_t *c){
	assert_format(!!tr->object,
		"[ARRAY] object is required!");
	object_t *obj;
	assert_format(!!(obj = object_define(OTP_ARRAY, sizeof(list_t))), 
		"unable to alloc memory!");
	list_t *tbl = list_create();
	if(tr->object->type == OTP_TUPLE){
		list_t *tt = (list_t *)tr->object->ptr;
		ilist_t *a;
		for(a = tt->begin; a != tt->end; a = a->next){
			list_rpush(tbl, (list_value_t)a->value);
		}
	} else {
		list_rpush(tbl, (list_value_t)tr->object);
	}
	obj->ptr = tbl;
	tr->object = obj;
	return c->next;
}

ilist_t *
ircode_tuple(vm_t *tr, ilist_t *c){
	assert_format(!!tr->object,
		"[TUPLE] object is required!");
	object_t *obj;
	assert_format(!!(obj = object_define(OTP_TUPLE, sizeof(list_t))), 
		"unable to alloc memory!");
	list_t *tbl = list_create();
	if(tr->object->type == OTP_TUPLE){
		list_t *tt = (list_t *)tr->object->ptr;
		ilist_t *a;
		for(a = tt->begin; a != tt->end; a = a->next){
			list_rpush(tbl, (list_value_t)a->value);
		}
	} else {
		list_rpush(tbl, (list_value_t)tr->object);
	}
	obj->ptr = tbl;
	tr->object = obj;
	return c->next;
}

ilist_t *
ircode_import(vm_t *tr, ilist_t *c){
	assert_format(!!(tr->object->type == OTP_ARRAY), 
		"[IMPORT] import function need to an array");

	char *destination;
	destination = data_to((list_t *)tr->object->ptr);

    FILE *fd;
	assert_format (!!(fd = fopen(destination, "rb")),
		"[IMPORT] could not open(%s)\n", destination);

    // Current position
    long_t pos = ftell(fd);
    // Go to end
    fseek(fd, 0, SEEK_END);
    // read the position which is the size
    long_t chunk = ftell(fd);
    // restore original position
    fseek(fd, pos, SEEK_SET);

    char *buf;

    assert_format (!!(buf = malloc(chunk + 1)),
		"could not malloc(%ld) for buf area\n", chunk);

    // read the source file
	long_t i = fread(buf, 1, chunk, fd);
    assert_format (!!(i >= chunk), "read returned %ld\n", i);

    buf[i] = '\0';

    fclose(fd);

	list_t *tokens;
	assert_format(!!(tokens = list_create()), 
		"[IMPORT] tokens list not created");

    lexer(tokens, buf);

    qalam_free(buf);

    list_t *code;
	assert_format(!!(code = list_create()), 
		"[IMPORT] code array not created");
	
    parser_t *parser;
	assert_format(!!(parser = parse(tokens, code)), 
		"[IMPORT] parser is aborted");

    tr->object = import(parser->schema, code);

	return c->next;
}

ilist_t *
ircode_fork(vm_t *tr, ilist_t *c) {
	object_t *object;
	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	*(long_t *)object->ptr = fork();
	tr->object = object;
	return c->next;
}

ilist_t *
ircode_getpid(vm_t *tr, ilist_t *c) {
	object_t *object;
	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	*(long_t *)object->ptr = (long_t)getpid();
	tr->object = object;
	return c->next;
}

ilist_t *
ircode_wait(vm_t *tr, ilist_t *c) {
	object_t *object;
	assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
		"unable to alloc memory!");
	*(long_t *)object->ptr = wait(nullptr);
	tr->object = object;
	return c->next;
}

ilist_t *
ircode_sleep(vm_t *tr, ilist_t *c) {
	list_t *tbl;
	if(tr->object->type == OTP_TUPLE){
		tbl = (list_t *)tr->object->ptr;
	} else {
		tbl = list_create();
		list_rpush(tbl, (list_value_t)tr->object);
	}

	list_t *tblresp;
	tblresp = list_create();

	object_t *eax;
	while((eax = (object_t *)list_content(list_lpop(tbl)))){
		assert_format((eax->type == OTP_LONG), "Not true validated");
		object_t *object;
		assert_format(!!(object = object_define(OTP_LONG, sizeof(long_t))), 
			"unable to alloc memory!");
		*(long_t *)object->ptr = (long_t)sleep((unsigned int)(*(long_t *)eax->ptr));
		list_rpush(tblresp, (list_value_t)object);
	}

	if(list_count(tblresp) > 1){
		object_t *object;
		assert_format(!!(object = object_define(OTP_TUPLE, sizeof(list_t))), 
			"unable to alloc memory!");
		object->ptr = tblresp;
		tr->object = object;
	} else {
		tr->object = (object_t *)list_content(list_rpop(tblresp));
	}

	return c->next;
}

ilist_t *
decode(vm_t *tr, ilist_t *c) {

	// printf("thread %ld\n", c->value);

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
			return ircode_bscp(tr, c);
			break;
		case ESCP:
			return ircode_escp(tr, c);
			break;

		case OR:
			return ircode_or(tr, c);
			break;
		case LOR:
			return ircode_lor(tr, c);
			break;
		case XOR:
			return ircode_xor(tr, c);
			break;
		case AND:
			return ircode_and(tr, c);
			break;
		case LAND:
			return ircode_land(tr, c);
			break;
		case EQ:
			return ircode_eq(tr, c);
			break;
		case NE:
			return ircode_ne(tr, c);
			break;
		case LT:
			return ircode_lt(tr, c);
			break;
		case LE:
			return ircode_le(tr, c);
			break;
		case GT:
			return ircode_gt(tr, c);
			break;
		case GE:
			return ircode_ge(tr, c);
			break;
		case LSHIFT:
			return ircode_lshift(tr, c);
			break;
		case RSHIFT:
			return ircode_rshift(tr, c);
			break;
		case ADD:
			return ircode_add(tr, c);
			break;
		case SUB:
			return ircode_sub(tr, c);
			break;
		case MUL:
			return ircode_mul(tr, c);
			break;
		case DIV:
			return ircode_div(tr, c);
			break;
		case MOD:
			return ircode_mod(tr, c);
			break;

			// VAR IMM DATA SD LD PUSH JMP JZ JNZ CENT CLEV CALL ENT LEV THIS SUPER DOT
			// SIZEOF TYPEOF DELETE INSERT COUNT BREAK NEW REF RET
		case IMM:
			return ircode_imm(tr, c);
			break;
		case SD:
			return ircode_sd(tr, c);
			break;
		case LD:
			return ircode_ld(tr, c);
			break;
		case PUSH:
			return ircode_push(tr, c);
			break;
		case JMP:
			return ircode_jmp(tr, c);
			break;
		case JZ:
			return ircode_jz(tr, c);
			break;
		case JNZ:
			return ircode_jnz(tr, c);
			break;
		case COMMA:
			return ircode_comma(tr, c);
			break;
		case CALL:
			return ircode_call(tr, c);
			break;
		case CELL:
			return ircode_cell(tr, c);
			break;
		case ENT:
			return ircode_ent(tr, c);
			break;
		case EXD:
			return ircode_extend(tr, c);
			break;
		case LEV:
			return ircode_lev(tr, c);
			break;
		case REL:
			return ircode_rel(tr, c);
			break;
		case SIM:
			return ircode_sim(tr, c);
			break;
		case CLS:
			return ircode_scheming(tr, c);
			break;
		case RET:
			return ircode_ret(tr, c);
			break;

		case THIS:
			return vm_this(tr, c);
			break;
		case SUPER:
			return ircode_super(tr, c);
			break;
		case PRTF:
			return ircode_print(tr, c);
			break;
		case ARRAY:
			return ircode_array(tr, c);
			break;
		case TUPLE:
			return ircode_tuple(tr, c);
			break;
		case IMPORT:
			return ircode_import(tr, c);
			break;
		case EXIT:
			exit(0);
			return c->next;
			break;

		default:
			printf("unknown instruction %ld\n", c->value);
            exit(-1);
	}

	return c;
}


void
eval(schema_t *root, list_t *code)
{
	vm_t *tr = ircode_create();

	ilist_t *adrs = list_rpush(code, EXIT);

	root->root = tr->layout;

	ilist_t *c = call(tr, root, adrs);

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

    long_t i;
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
    long_t pos = ftell(fd);
    // Go to end
    fseek(fd, 0, SEEK_END);
    // read the position which is the size
    long_t chunk = ftell(fd);
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

    list_t *tokens = list_create();
    lexer(tokens, buf);
    qalam_free(buf);
    
    printf("lexer is done!\n");

    list_t *code = list_create();
    parser_t *parser = parse(tokens, code);
    
    printf("parser is done!\n");

    eval(parser->schema, code);

    return 0;
}
