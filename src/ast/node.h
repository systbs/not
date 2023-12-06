#pragma once
#ifndef __NODE_H__
#define __NODE_H__

typedef struct node {
	int32_t kind;
	uint64_t id;
	
	void *value;
	
	position_t position;
	list_t *locals;

	struct node *parent;
} node_t;

typedef enum node_kind {
	NODE_KIND_ID,
	NODE_KIND_NUMBER,
	NODE_KIND_CHAR,
	NODE_KIND_STRING,
	NODE_KIND_NULL,
	NODE_KIND_TRUE,
	NODE_KIND_FALSE,
	NODE_KIND_ARRAY,
	NODE_KIND_OBJECT_PROPERTY,
	NODE_KIND_OBJECT,
	NODE_KIND_COMPOSITE,

	NODE_KIND_TYPE_ARGUMENTS,

	NODE_KIND_IN,

	NODE_KIND_TYPEOF,
	NODE_KIND_SIZEOF,
	NODE_KIND_AWAIT,
	NODE_KIND_PARENTHESIS,
	
	NODE_KIND_ARGUMENT,
	NODE_KIND_BODY_PROPERTY,
	NODE_KIND_BODY,
	NODE_KIND_CALL,
	NODE_KIND_GET_ITEM,
	NODE_KIND_GET_ATTR,
	NODE_KIND_GET_SLICE,
	
	NODE_KIND_CAST,
	NODE_KIND_TILDE,
	NODE_KIND_NOT,
	NODE_KIND_NEG,
	NODE_KIND_POS,
	NODE_KIND_GET_ADDRESS,
	NODE_KIND_GET_VALUE,
	NODE_KIND_ELLIPSIS,
	
	NODE_KIND_MUL,
	NODE_KIND_DIV,
	NODE_KIND_MOD,
	
	NODE_KIND_PLUS,
	NODE_KIND_MINUS,
	
	NODE_KIND_SHL,
	NODE_KIND_SHR,
	
	NODE_KIND_LT,
	NODE_KIND_LE,
	NODE_KIND_GT,
	NODE_KIND_GE,
	
	NODE_KIND_EQ,
	NODE_KIND_NEQ,
	NODE_KIND_EXTENDS,
	
	NODE_KIND_AND,
	NODE_KIND_XOR,
	NODE_KIND_OR,
	NODE_KIND_LAND,
	NODE_KIND_LOR,
	
	NODE_KIND_CONDITIONAL,
	
	NODE_KIND_ASSIGN,
	NODE_KIND_DEFINE,
	NODE_KIND_ADD_ASSIGN,
	NODE_KIND_SUB_ASSIGN,
	NODE_KIND_MUL_ASSIGN,
	NODE_KIND_DIV_ASSIGN,
	NODE_KIND_MOD_ASSIGN,
	NODE_KIND_AND_ASSIGN,
	NODE_KIND_OR_ASSIGN,
	NODE_KIND_SHL_ASSIGN,
	NODE_KIND_SHR_ASSIGN,
	
	NODE_KIND_IF,
	NODE_KIND_FOR_INIT_LIST,
	NODE_KIND_FOR_STEP_LIST,
	NODE_KIND_FOR,
	NODE_KIND_FORIN,
	NODE_KIND_BREAK,
	NODE_KIND_CONTINUE,
	NODE_KIND_CATCH,
	NODE_KIND_CATCH_LIST,
	NODE_KIND_TRY,
	NODE_KIND_RETURN,
	NODE_KIND_THROW,
	
	NODE_KIND_VAR,
	NODE_KIND_CONST,
	NODE_KIND_TYPE,
	NODE_KIND_PARAMETER,
	NODE_KIND_FIELD,
	NODE_KIND_TYPE_PARAMETER,
	NODE_KIND_FUNC,
	NODE_KIND_READONLY,
	NODE_KIND_PROPERTY,
	NODE_KIND_METHOD,
	NODE_KIND_CLASS,
	NODE_KIND_HERITAGE,
	NODE_KIND_ENUM_MEMBER,
	NODE_KIND_ENUM_BLOCK,
	NODE_KIND_ENUM,
	NODE_KIND_BLOCK,
	NODE_KIND_IMPORT,
	NODE_KIND_EXPORT_BLOCK,
	NODE_KIND_EXPORT,
	NODE_KIND_MODULE,
	NODE_KIND_DOC
} node_kind_t;

typedef struct node_basic {
	char *value;
} node_basic_t;

typedef struct node_array {
	list_t *list;
} node_array_t;

typedef struct node_object_property {
	node_t *name;
	node_t *value;
} node_object_property_t;

typedef struct node_object {
	list_t *list;
} node_object_t;

typedef struct node_composite {
	node_t *base;
	list_t *type_arguments;
} node_composite_t;




typedef struct node_argument {
	list_t *value;
} node_argument_t;

typedef struct node_call {
	node_t *callable;
	list_t *arguments;
} node_call_t;

typedef struct node_get_slice {
	node_t *name;
	node_t *start;
	node_t *stop;
	node_t *step;
} node_get_slice_t;

typedef struct node_get_item {
	node_t *name;
	node_t *index;
} node_get_item_t;

typedef struct node_get_attr {
	node_t *left;
	node_t *right;
} node_get_attr_t;


typedef struct node_unary {
	node_t *right;
} node_unary_t;

typedef struct node_binary {
	node_t *left;
	node_t *right;
} node_binary_t;

typedef struct node_conditional {
	node_t *condition;
	node_t *true_expression;
	node_t *false_expression;
} node_conditional_t;





typedef struct node_block {
	list_t *list;
} node_block_t;

typedef struct node_enumerable {
	list_t *list;
} node_enumerable_t;

typedef struct node_if {
	node_t *name;
	node_t *condition;
	node_t *then_body;
	node_t *else_body;
} node_if_t;

typedef struct node_for {
	node_t *name;
	node_t *initializer;
	node_t *condition;
	node_t *incrementor;
	node_t *body;
} node_for_t;

typedef struct node_forin {
	node_t *name;
	node_t *initializer;
	node_t *expression;
	node_t *body;
} node_forin_t;

typedef struct node_catch {
	node_t *parameter;
	node_t *body;
} node_catch_t;

typedef struct node_try {
	node_t *body;
	list_t *catchs;
} node_try_t;


typedef struct node_var {
	node_t *name;
	node_t *type;
	node_t *value;
} node_var_t;

typedef struct node_const {
	node_t *name;
	node_t *type;
	node_t *value;
} node_const_t;

typedef struct node_type {
	node_t *name;
	list_t *type_parameters;
	list_t *heritages;
	node_t *body;
} node_type_t;

typedef struct node_parameter {
	node_t *name;
	node_t *type;
	node_t *value;
} node_parameter_t;

typedef struct node_field {
	node_t *name;
	node_t *type;
} node_field_t;

typedef struct node_heritage {
	node_t *name;
	node_t *type;
} node_heritage_t;

typedef struct node_type_parameter {
	node_t *name;
	node_t *extends;
	node_t *value;
} node_type_parameter_t;

typedef struct node_func {
	list_t *fields;
	list_t *type_parameters;
	node_t *name;
	list_t *parameters;
	node_t *body;
} node_func_t;

typedef struct node_method {
	uint64_t flag;
	node_t *name;
	list_t *type_parameters;
	list_t *parameters;
	node_t *body;
} node_method_t;

typedef struct node_property {
	uint64_t flag;
	node_t *name;
	node_t *type;
	node_t *value;
} node_property_t;

typedef struct node_class {
	uint64_t flag;
	node_t *name;
	list_t *heritages;
	list_t *type_parameters;
	list_t *body;
} node_class_t;

typedef struct node_enum_member {
	node_t *name;
	node_t *value;
} node_enum_member_t;

typedef struct node_enum {
	uint64_t flag;
	node_t *name;
	list_t *body;
} node_enum_t;

typedef struct node_import {
	node_t *path;
	list_t *fields;
} node_import_t;

typedef struct node_module {
	char *path;
	list_t *members;
} node_module_t;




node_t *
node_make_id(position_t position, char *value);

node_t *
node_make_number(position_t position, char *value);

node_t *
node_make_char(position_t position, char *value);

node_t *
node_make_string(position_t position, char *value);

node_t *
node_make_null(position_t position);

node_t *
node_make_true(position_t position);

node_t *
node_make_false(position_t position);

node_t *
node_make_array(position_t position, list_t *expr_list);

node_t *
node_make_object_property(position_t position, node_t *name, node_t *value);

node_t *
node_make_object(position_t position, list_t *property_list);

node_t *
node_make_composite(position_t position, node_t *base, list_t *type_arguments);


node_t *
node_make_in(position_t position, node_t *left, node_t *right);

node_t *
node_make_typeof(position_t position, node_t *right);

node_t *
node_make_sizeof(position_t position, node_t *right);

node_t *
node_make_parenthesis(position_t position, node_t *right);




node_t *
node_make_argument(position_t position, list_t *value);

node_t *
node_make_call(position_t position, node_t *callable, list_t *arguments);

node_t *
node_make_get_slice(position_t position, node_t *name, node_t *start, node_t *stop, node_t *step);

node_t *
node_make_get_item(position_t position, node_t *name, node_t *index);

node_t *
node_make_get_attr(position_t position, node_t *left, node_t *right);

node_t *
node_make_get_address(position_t position, node_t *right);




node_t *
node_make_tilde(position_t position, node_t *right);

node_t *
node_make_not(position_t position, node_t *right);

node_t *
node_make_neg(position_t position, node_t *right);

node_t *
node_make_pos(position_t position, node_t *right);

node_t *
node_make_get_address(position_t position, node_t *right);

node_t *
node_make_get_value(position_t position, node_t *right);

node_t *
node_make_ellipsis(position_t position, node_t *right);




node_t *
node_make_mul(position_t position, node_t *left, node_t *right);

node_t *
node_make_div(position_t position, node_t *left, node_t *right);

node_t *
node_make_mod(position_t position, node_t *left, node_t *right);

node_t *
node_make_plus(position_t position, node_t *left, node_t *right);

node_t *
node_make_minus(position_t position, node_t *left, node_t *right);

node_t *
node_make_shl(position_t position, node_t *left, node_t *right);

node_t *
node_make_shr(position_t position, node_t *left, node_t *right);

node_t *
node_make_lt(position_t position, node_t *left, node_t *right);

node_t *
node_make_le(position_t position, node_t *left, node_t *right);

node_t *
node_make_gt(position_t position, node_t *left, node_t *right);

node_t *
node_make_ge(position_t position, node_t *left, node_t *right);

node_t *
node_make_eq(position_t position, node_t *left, node_t *right);

node_t *
node_make_neq(position_t position, node_t *left, node_t *right);

node_t *
node_make_extends(position_t position, node_t *left, node_t *right);

node_t *
node_make_and(position_t position, node_t *left, node_t *right);

node_t *
node_make_xor(position_t position, node_t *left, node_t *right);

node_t *
node_make_or(position_t position, node_t *left, node_t *right);

node_t *
node_make_land(position_t position, node_t *left, node_t *right);

node_t *
node_make_lor(position_t position, node_t *left, node_t *right);

node_t *
node_make_conditional(position_t position, node_t *condition, node_t *true_expression, node_t *false_expression);







node_t *
node_make_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_define(position_t position, node_t *left, node_t *right);

node_t *
node_make_add_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_sub_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_mul_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_div_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_mod_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_and_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_or_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_shl_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_shr_assign(position_t position, node_t *left, node_t *right);

node_t *
node_make_if(position_t position, node_t *name, node_t *condition, node_t *then_body, node_t *else_body);

node_t *
node_make_for_init_list(position_t position, list_t *init_list);

node_t *
node_make_for_step_list(position_t position, list_t *init_list);

node_t *
node_make_for(position_t position, node_t *name, node_t *initializer, node_t *condition, node_t *incrementor, node_t *body);

node_t *
node_make_forin(position_t position, node_t *name, node_t *initializer, node_t *expression, node_t *body);

node_t *
node_make_break(position_t position, node_t *expression);

node_t *
node_make_continue(position_t position, node_t *expression);

node_t *
node_make_catch(position_t position, node_t *parameter, node_t *body);

node_t *
node_make_try(position_t position, node_t *body, list_t *catchs);

node_t *
node_make_return(position_t position, node_t *expr);

node_t *
node_make_throw(position_t position, node_t *value);

node_t *
node_make_type_arguments(position_t position, list_t *list);

node_t *
node_make_var(position_t position, node_t *name, node_t *type, node_t *value);

node_t *
node_make_const(position_t position, node_t *name, node_t *type, node_t *value);

node_t *
node_make_type(position_t position, node_t *name, list_t *type_parameters, list_t *heritages, node_t *body);




node_t *
node_make_parameter(position_t position, node_t *name, node_t *type, node_t *value);

node_t *
node_make_field(position_t position, node_t *name, node_t *type);

node_t *
node_make_type_parameter(position_t position, node_t *name, node_t *heritage, node_t *value);

node_t *
node_make_func(
	position_t position, 
	list_t *fields,
	list_t *type_parameters, 
	node_t *name, 
	list_t *parameters, 
	node_t *block_stmt);

node_t *
node_make_heritage(position_t position, node_t *name, node_t *type);

node_t *
node_make_method(position_t position, uint64_t flag, node_t *name, list_t *type_parameters, list_t *parameters, node_t *body);

node_t *
node_make_property(position_t position, uint64_t flag, node_t *name, node_t *type, node_t *value);

node_t *
node_make_class(
	position_t position,
	uint64_t flag,
	node_t *name, 
	list_t *type_parameters, 
	list_t *heritages, 
	list_t *body);

node_t *
node_make_enum_member(position_t position, node_t *name, node_t *value);

node_t *
node_make_enum(position_t position, uint64_t flag, node_t *name, list_t *body);

node_t *
node_make_block(position_t position, list_t *stmt_list);

node_t *
node_make_import_paths(position_t position, list_t *list);

node_t *
node_make_import(position_t position, node_t *path, list_t *fields);

node_t *
node_make_export_block(position_t position, list_t *list);

node_t *
node_make_export(position_t position, node_t *value);

node_t *
node_make_module(position_t position, char *path, list_t *members);

#endif /* __NODE_H__ */
