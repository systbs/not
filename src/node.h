#pragma once
#ifndef __NODE_H__
#define __NODE_H__

typedef struct node {
	int32_t kind;
	void *value;
	
	uint64_t position;
	uint64_t line;
	uint64_t column;
	
	char *path;
} node_t;

typedef enum node_kind {
	NODE_KIND_ID,
	NODE_KIND_NUMBER,
	NODE_KIND_LETTERS,
	NODE_KIND_NULL,
	NODE_KIND_THIS,
	NODE_KIND_SUPER,
	NODE_KIND_LIST,
	NODE_KIND_ELEMENT,
	NODE_KIND_MAP,
	
	NODE_KIND_AS,
	
	NODE_KIND_ARGUMENT,
	NODE_KIND_CALL,
	
	NODE_KIND_GET_ITEM,
	NODE_KIND_GET_ATTR,
	NODE_KIND_GET_SLICE,
	
	NODE_KIND_TILDE,
	NODE_KIND_NOT,
	NODE_KIND_NEG,
	NODE_KIND_POS,
	NODE_KIND_GET_ADDRESS,
	NODE_KIND_GET_VALUE,
	
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
	
	NODE_KIND_AND,
	NODE_KIND_XOR,
	NODE_KIND_OR,
	NODE_KIND_LAND,
	NODE_KIND_LOR,
	
	NODE_KIND_CONDITIONAL,
	
	NODE_KIND_UNPACK,
	NODE_KIND_ASSIGN,
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
	NODE_KIND_WHILE,
	NODE_KIND_BREAK,
	NODE_KIND_CONTINUE,
	NODE_KIND_RETURN,
	
	NODE_KIND_CHAR,
	NODE_KIND_STRING,
	NODE_KIND_INT,
	NODE_KIND_LONG,
	NODE_KIND_FLOAT,
	NODE_KIND_DOUBLE,
	NODE_KIND_POINTER,
	NODE_KIND_TAG,
	NODE_KIND_TAG_LIST,
	NODE_KIND_ARRAY,
	NODE_KIND_TUPLE,
	NODE_KIND_UNION,
	NODE_KIND_ANY,
	
	NODE_KIND_VAR,
	NODE_KIND_DIRECTORY,
	NODE_KIND_VAR_LIST,
	NODE_KIND_PARAMETER,
	NODE_KIND_PARAMETER_LIST,
	NODE_KIND_GENERIC,
	NODE_KIND_GENERIC_LIST,
	NODE_KIND_FN,
	NODE_KIND_SUPER_LIST,
	NODE_KIND_CLASS_BLOCK,
	NODE_KIND_CLASS,
	NODE_KIND_ENUM_BLOCK,
	NODE_KIND_ENUM,
	NODE_KIND_BLOCK,
	NODE_KIND_IMPORT,
	NODE_KIND_EXTERN,
	NODE_KIND_MODULE
} node_kind_t;

typedef struct node_id {
	char *value;
} node_id_t;

typedef struct node_number {
	char *value;
} node_number_t;

typedef struct node_letters {
	char *value;
} node_letters_t;

typedef struct node_list {
	list_t *expr_list;
} node_list_t;

typedef struct node_element {
	node_t *key;
	node_t *value;
} node_element_t;

typedef struct node_map {
	list_t *element_list;
} node_map_t;

typedef struct node_as {
	node_t *left;
	node_t *right;
} node_as_t;

typedef struct node_argument {
	list_t *value;
} node_argument_t;

typedef struct node_call {
	node_t *callable;
	node_t *tag;
	node_t *argument;
} node_call_t;

typedef struct node_get_slice {
	node_t *name;
	node_t *start;
	node_t *stop;
	node_t *step;
} node_get_slice_t;

typedef struct node_get_item {
	node_t *left;
	node_t *right;
} node_get_item_t;

typedef struct node_get_attr {
	node_t *left;
	node_t *right;
} node_get_attr_t;

typedef struct node_unary {
	node_t *left;
} node_unary_t;

typedef struct node_binary {
	node_t *left;
	node_t *right;
} node_binary_t;

typedef struct node_conditional {
	node_t *expr;
	node_t *true_expr;
	node_t *false_expr;
} node_conditional_t;

typedef struct node_unpack {
	void *expr;
} node_unpack_t;

typedef struct node_if {
	node_t *expr;
	node_t *then_block_stmt;
	node_t *else_block_stmt;
} node_if_t;

typedef struct node_while {
	node_t *expr;
	node_t *block_stmt;
} node_while_t;

typedef struct node_return {
	node_t *expr;
} node_return_t;

typedef struct node_tag_list {
	list_t *list;
} node_tag_list_t;

typedef struct node_tag {
	node_t *name;
	node_t *tag_list;
} node_tag_t;

typedef struct node_pointer {
	node_t *item;
} node_pointer_t;

typedef struct node_array {
	node_t *item;
} node_array_t;

typedef struct node_tuple {
	list_t *type_list;
} node_tuple_t;

typedef struct node_union {
	list_t *type_list;
} node_union_t;

typedef struct node_var {
	node_t *name;
	node_t *label;
	node_t *expr;
} node_var_t;

typedef struct node_directory {
	list_t *wise_list;
} node_directory_t;

typedef struct node_var_list {
	list_t *list;
} node_var_list_t;

typedef struct node_parameter {
	node_t *name;
	node_t *label;
	node_t *expr;
} node_parameter_t;

typedef struct node_parameter_list {
	list_t *list;
} node_parameter_list_t;

typedef struct node_generic {
	node_t *name;
	node_t *label;
	node_t *default_value;
} node_generic_t;

typedef struct node_generic_list {
	list_t *list;
} node_generic_list_t;

typedef struct node_fn {
	node_t *name;
	node_t *generic_list;
	node_t *parameter_list;
	node_t *return_type;
	node_t *block_stmt;
} node_fn_t;

typedef struct node_super_list {
	list_t *list;
} node_super_list_t;

typedef struct node_class_block {
	list_t *list;
} node_class_block_t;

typedef struct node_class {
	node_t *name;
	node_t *super_list;
	node_t *generic_list;
	node_t *block_stmt;
} node_class_t;

typedef struct node_enum_block {
	list_t *enum_list;
} node_enum_block_t;

typedef struct node_enum {
	node_t *name;
	node_t *block_stmt;
} node_enum_t;

typedef struct node_block {
	list_t *stmt_list;
} node_block_t;

typedef struct node_import {
	node_t *block;
	node_t *from;
} node_import_t;

typedef struct node_extern {
	node_t *block_stmt;
} node_extern_t;

typedef struct node_module {
	list_t *stmt_list;
} node_module_t;

node_t *
node_make_id(char *value);

node_t *
node_make_number(char *value);

node_t *
node_make_letters(char *value);

node_t *
node_make_null();

node_t *
node_make_this();

node_t *
node_make_super();

node_t *
node_make_list(list_t *expr_list);

node_t *
node_make_element(node_t *key, node_t *value);

node_t *
node_make_map(list_t *element_list);

node_t *
node_make_as(node_t *left, node_t *right);

node_t *
node_make_argument(list_t *value);

node_t *
node_make_call(node_t *callable, node_t *tag, node_t *argument);

node_t *
node_make_get_slice(node_t *name, node_t *start, node_t *stop, node_t *step);

node_t *
node_make_get_item(node_t *left, node_t *right);

node_t *
node_make_get_attr(node_t *left, node_t *right);

node_t *
node_make_tilde(node_t *left);

node_t *
node_make_not(node_t *left);

node_t *
node_make_neg(node_t *left);

node_t *
node_make_pos(node_t *left);

node_t *
node_make_get_address(node_t *left);

node_t *
node_make_get_value(node_t *left);

node_t *
node_make_mul(node_t *left, node_t *right);

node_t *
node_make_div(node_t *left, node_t *right);

node_t *
node_make_mod(node_t *left, node_t *right);

node_t *
node_make_plus(node_t *left, node_t *right);

node_t *
node_make_minus(node_t *left, node_t *right);

node_t *
node_make_shl(node_t *left, node_t *right);

node_t *
node_make_shr(node_t *left, node_t *right);

node_t *
node_make_lt(node_t *left, node_t *right);

node_t *
node_make_le(node_t *left, node_t *right);

node_t *
node_make_gt(node_t *left, node_t *right);

node_t *
node_make_ge(node_t *left, node_t *right);

node_t *
node_make_eq(node_t *left, node_t *right);

node_t *
node_make_neq(node_t *left, node_t *right);

node_t *
node_make_and(node_t *left, node_t *right);

node_t *
node_make_xor(node_t *left, node_t *right);

node_t *
node_make_or(node_t *left, node_t *right);

node_t *
node_make_land(node_t *left, node_t *right);

node_t *
node_make_lor(node_t *left, node_t *right);

node_t *
node_make_conditional(node_t *expr, node_t *true_expr, node_t *false_expr);

node_t *
node_make_assign(node_t *left, node_t *right);

node_t *
node_make_add_assign(node_t *left, node_t *right);

node_t *
node_make_sub_assign(node_t *left, node_t *right);

node_t *
node_make_mul_assign(node_t *left, node_t *right);

node_t *
node_make_div_assign(node_t *left, node_t *right);

node_t *
node_make_mod_assign(node_t *left, node_t *right);

node_t *
node_make_and_assign(node_t *left, node_t *right);

node_t *
node_make_or_assign(node_t *left, node_t *right);

node_t *
node_make_shl_assign(node_t *left, node_t *right);

node_t *
node_make_shr_assign(node_t *left, node_t *right);

node_t *
node_make_unpack(void *expr);

node_t *
node_make_if(node_t *expr, node_t *then_block_stmt, node_t *else_block_stmt);

node_t *
node_make_while(node_t *expr, node_t *block_stmt);

node_t *
node_make_break();

node_t *
node_make_continue();

node_t *
node_make_return(node_t *expr);

node_t *
node_make_char();

node_t *
node_make_string();

node_t *
node_make_int();

node_t *
node_make_long();

node_t *
node_make_float();

node_t *
node_make_double();

node_t *
node_make_tag_list(list_t *list);

node_t *
node_make_tag(node_t *name, node_t *tag_list);

node_t *
node_make_pointer(node_t *item);

node_t *
node_make_array(node_t *item);

node_t *
node_make_tuple(list_t *type_list);

node_t *
node_make_union(list_t *type_list);

node_t *
node_make_any();

node_t *
node_make_var(node_t *name, node_t *label, node_t *expr);

node_t *
node_make_directory(list_t *wise_list);

node_t *
node_make_var_list(list_t *list);

node_t *
node_make_parameter(node_t *name, node_t *label, node_t *expr);

node_t *
node_make_parameter_list(list_t *list);

node_t *
node_make_generic(node_t *name, node_t *label, node_t *default_value);

node_t *
node_make_generic_list(list_t *list);

node_t *
node_make_fn(node_t *name, node_t *generic_list, node_t *parameter_list, node_t *return_type, node_t *block_stmt);

node_t *
node_make_super_list(list_t *list);

node_t *
node_make_class_block(list_t *list);

node_t *
node_make_class(node_t *name, node_t *generic_list, node_t *super_list, node_t *block_stmt);

node_t *
node_make_enum_block(list_t *enum_list);

node_t *
node_make_enum(node_t *name, node_t *block_stmt);

node_t *
node_make_block(list_t *stmt_list);

node_t *
node_make_import(node_t *subscript, node_t *from);

node_t *
node_make_extern(node_t *block);

node_t *
node_make_module(list_t *stmt_list);

uint64_t
node_get_position(node_t *node);

uint64_t
node_get_line(node_t *node);

uint64_t
node_get_column(node_t *node);

char *
node_get_path(node_t *node);
#endif //__NODE_H__
