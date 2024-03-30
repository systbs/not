#pragma once
#ifndef __NODE_H__
#define __NODE_H__

typedef struct node {
	int32_t kind;
	uint64_t id;
	uint64_t flag;
	position_t position;

	void *value;

	struct node *parent;
	struct node *scope;
} node_t;

typedef enum node_flag {
	NODE_FLAG_NONE 			= 0,
	NODE_FLAG_TEMPORARY 	= 1 << 0,
	NODE_FLAG_NEW 			= 1 << 1,
	NODE_FLAG_DERIVE		= 1 << 2,
} node_flag_t;

typedef enum node_kind {
	NODE_KIND_ID,
	NODE_KIND_NUMBER,
	NODE_KIND_CHAR,
	NODE_KIND_STRING,
	NODE_KIND_NULL,
	NODE_KIND_TRUE,
	NODE_KIND_FALSE,
	NODE_KIND_INFINITY,
	NODE_KIND_ARRAY,
	NODE_KIND_OBJECT,
	NODE_KIND_PSEUDONYM,
	NODE_KIND_SET,

	NODE_KIND_IN,

	NODE_KIND_TYPEOF,
	NODE_KIND_SIZEOF,
	NODE_KIND_PARENTHESIS,
	
	NODE_KIND_CALL,
	NODE_KIND_ITEM,
	NODE_KIND_ATTRIBUTE,
	
	NODE_KIND_CAST,
	NODE_KIND_TILDE,
	NODE_KIND_NOT,
	NODE_KIND_NEG,
	NODE_KIND_POS,
	NODE_KIND_ELLIPSIS,
	
	NODE_KIND_POW,
	NODE_KIND_EPI,

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
	
	NODE_KIND_INITIALIZER,
	NODE_KIND_INCREMENTOR,

	NODE_KIND_IF,
	NODE_KIND_FOR,
	NODE_KIND_FORIN,
	NODE_KIND_BREAK,
	NODE_KIND_CONTINUE,
	NODE_KIND_CATCH,
	NODE_KIND_CATCHS,
	NODE_KIND_TRY,
	NODE_KIND_RETURN,
	NODE_KIND_THROW,
	
	NODE_KIND_VAR,
	NODE_KIND_TYPE,
	NODE_KIND_LAMBDA,
	NODE_KIND_FUN,
	NODE_KIND_CLASS,
	NODE_KIND_METHOD,
	NODE_KIND_PAIR,
	NODE_KIND_ENTIERY,
	NODE_KIND_PROPERTY,
	NODE_KIND_PROPERTIES,
	NODE_KIND_ARGUMENT,
	NODE_KIND_ARGUMENTS,
	NODE_KIND_CONCEPT,
	NODE_KIND_CONCEPTS,
	NODE_KIND_PARAMETER,
	NODE_KIND_PARAMETERS,
	NODE_KIND_FIELD,
	NODE_KIND_FIELDS,
	NODE_KIND_GENERIC,
	NODE_KIND_GENERICS,
	NODE_KIND_HERITAGE,
	NODE_KIND_HERITAGES,
	NODE_KIND_MEMBER,
	NODE_KIND_MEMBERS,
	NODE_KIND_ENUM,
	NODE_KIND_BLOCK,
	NODE_KIND_BODY,
	NODE_KIND_PACKAGE,
	NODE_KIND_PACKAGES,
	NODE_KIND_IMPORT,
	NODE_KIND_MODULE,
	NODE_KIND_DOC
} node_kind_t;

typedef struct node_basic {
	char *value;
} node_basic_t;

typedef struct node_array {
	list_t *list;
} node_array_t;

typedef struct node_object {
	list_t *list;
} node_object_t;



typedef struct node_carrier {
	node_t *base;
	node_t *data;
} node_carrier_t;

typedef struct node_unary {
	node_t *right;
} node_unary_t;

typedef struct node_binary {
	node_t *left;
	node_t *right;
} node_binary_t;

typedef struct node_triple {
	node_t *base;
	node_t *left;
	node_t *right;
} node_triple_t;




typedef struct node_block {
	list_t *list;
} node_block_t;

typedef struct node_if {
	node_t *condition;
	node_t *then_body;
	node_t *else_body;
} node_if_t;

typedef struct node_for {
	uint64_t flag;
	node_t *key;
	node_t *initializer;
	node_t *condition;
	node_t *incrementor;
	node_t *body;
} node_for_t;

typedef struct node_forin {
	uint64_t flag;
	node_t *key;
	node_t *initializer;
	node_t *expression;
	node_t *body;
} node_forin_t;

typedef struct node_catch {
	node_t *parameters;
	node_t *body;
} node_catch_t;

typedef struct node_try {
	node_t *key;
	node_t *generics;
	node_t *body;
	node_t *catchs;
} node_try_t;

typedef struct node_var {
	uint64_t flag;
	node_t *key;
	node_t *type;
	node_t *value;

	node_t *value_update;
} node_var_t;

typedef struct node_parameter {
	uint64_t flag;
	node_t *key;
	node_t *type;
	node_t *value;

	node_t *value_update;
} node_parameter_t;

typedef struct node_field {
	node_t *key;
	node_t *value;
} node_field_t;

typedef struct node_argument {
	node_t *key;
	node_t *value;
} node_argument_t;

typedef struct node_heritage {
	node_t *key;
	node_t *type;

	node_t *value_update;
} node_heritage_t;

typedef struct node_generic {
	node_t *key;
	node_t *type;
	node_t *value;

	node_t *value_update;
} node_generic_t;

typedef struct node_lambda {
	node_t *key;
	node_t *generics;
	node_t *parameters;
	node_t *body;
} node_lambda_t;

typedef struct node_func {
	uint64_t flag;
	node_t *generics;
	node_t *key;
	node_t *parameters;
	node_t *result;
	node_t *body;
} node_fun_t;

typedef struct node_property {
	uint64_t flag;
	node_t *key;
	node_t *type;
	node_t *value;

	node_t *get;
	node_t *set;

	node_t *value_update;
} node_property_t;

typedef struct node_entiery {
	uint64_t flag;
	node_t *key;
	node_t *type;
	node_t *value;

	node_t *value_update;
} node_entiery_t;

typedef struct node_pair {
	node_t *key;
	node_t *value;
} node_pair_t;

typedef struct node_class {
	uint64_t flag;
	node_t *key;
	node_t *heritages;
	node_t *generics;
	node_t *block;
} node_class_t;

typedef struct node_member {
	node_t *key;
	node_t *value;
} node_member_t;

typedef struct node_enum {
	uint64_t flag;
	node_t *key;
	node_t *block;
} node_enum_t;

typedef struct node_package {
	node_t *key;
	node_t *generics;
	node_t *route;
} node_package_t;

typedef struct node_import {
	node_t *path;
	node_t *packages;
} node_import_t;

typedef struct node_module {
	char *path;
	list_t *items;
} node_module_t;



void
node_destroy(node_t *node);

node_t *
node_create(node_t *scope, node_t *parent, position_t position);

node_t *
node_clone(node_t *parent, node_t *node);

void
node_prug(node_t *node);

node_t *
node_make_id(node_t *node, char *value);

node_t *
node_make_number(node_t *node, char *value);

node_t *
node_make_char(node_t *node, char *value);

node_t *
node_make_string(node_t *node, char *value);

node_t *
node_make_null(node_t *node);

node_t *
node_make_true(node_t *node);

node_t *
node_make_false(node_t *node);

node_t *
node_make_infinity(node_t *node);

node_t *
node_make_this(node_t *node);

node_t *
node_make_array(node_t *node, list_t *expr_list);

node_t *
node_make_object(node_t *node, list_t *properties);

node_t *
node_make_pseudonym(node_t *node, node_t *base, node_t *arguments);


node_t *
node_make_typeof(node_t *node, node_t *right);

node_t *
node_make_sizeof(node_t *node, node_t *right);

node_t *
node_make_parenthesis(node_t *node, node_t *right);




node_t *
node_make_argument(node_t *node, node_t *name, node_t *value);

node_t *
node_make_arguments(node_t *node, list_t *list);

node_t *
node_make_concept(node_t *node, node_t *name, node_t *value);

node_t *
node_make_concepts(node_t *node, list_t *list);

node_t *
node_make_call(node_t *node, node_t *name, node_t *arguments);

node_t *
node_make_item(node_t *node, node_t *base, node_t *arguments);

node_t *
node_make_attribute(node_t *node, node_t *left, node_t *right);



node_t *
node_make_tilde(node_t *node, node_t *right);

node_t *
node_make_not(node_t *node, node_t *right);

node_t *
node_make_neg(node_t *node, node_t *right);

node_t *
node_make_pos(node_t *node, node_t *right);

node_t *
node_make_ellipsis(node_t *node, node_t *right);



node_t *
node_make_pow(node_t *node, node_t *left, node_t *right);

node_t *
node_make_epi(node_t *node, node_t *left, node_t *right);

node_t *
node_make_mul(node_t *node, node_t *left, node_t *right);

node_t *
node_make_div(node_t *node, node_t *left, node_t *right);

node_t *
node_make_mod(node_t *node, node_t *left, node_t *right);

node_t *
node_make_plus(node_t *node, node_t *left, node_t *right);

node_t *
node_make_minus(node_t *node, node_t *left, node_t *right);

node_t *
node_make_shl(node_t *node, node_t *left, node_t *right);

node_t *
node_make_shr(node_t *node, node_t *left, node_t *right);

node_t *
node_make_lt(node_t *node, node_t *left, node_t *right);

node_t *
node_make_le(node_t *node, node_t *left, node_t *right);

node_t *
node_make_gt(node_t *node, node_t *left, node_t *right);

node_t *
node_make_ge(node_t *node, node_t *left, node_t *right);

node_t *
node_make_eq(node_t *node, node_t *left, node_t *right);

node_t *
node_make_neq(node_t *node, node_t *left, node_t *right);

node_t *
node_make_and(node_t *node, node_t *left, node_t *right);

node_t *
node_make_xor(node_t *node, node_t *left, node_t *right);

node_t *
node_make_or(node_t *node, node_t *left, node_t *right);

node_t *
node_make_land(node_t *node, node_t *left, node_t *right);

node_t *
node_make_lor(node_t *node, node_t *left, node_t *right);



node_t *
node_make_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_add_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_sub_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_mul_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_div_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_mod_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_and_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_or_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_shl_assign(node_t *node, node_t *left, node_t *right);

node_t *
node_make_shr_assign(node_t *node, node_t *left, node_t *right);


node_t *
node_make_initializer(node_t *node, list_t *list);

node_t *
node_make_incrementor(node_t *node, list_t *list);

node_t *
node_make_if(node_t *node, node_t *condition, node_t *then_body, node_t *else_body);

node_t *
node_make_for(node_t *node, uint64_t flag, node_t *name, node_t *initializer, node_t *condition, node_t *incrementor, node_t *body);

node_t *
node_make_forin(node_t *node, uint64_t flag, node_t *name, node_t *initializer, node_t *expression, node_t *body);

node_t *
node_make_break(node_t *node, node_t *expression);

node_t *
node_make_continue(node_t *node, node_t *expression);

node_t *
node_make_catch(node_t *node, node_t *parameters, node_t *body);

node_t *
node_make_catchs(node_t *node, list_t *list);

node_t *
node_make_try(node_t *node, node_t *key, node_t *generics, node_t *body, node_t *catchs);

node_t *
node_make_return(node_t *node, node_t *expr);

node_t *
node_make_set(node_t *node, list_t *properties);

node_t *
node_make_var(node_t *node, uint64_t flag, node_t *name, node_t *type, node_t *value);

node_t *
node_make_parameter(node_t *node, uint64_t flag, node_t *name, node_t *type, node_t *value);

node_t *
node_make_parameters(node_t *node, list_t *list);

node_t *
node_make_field(node_t *node, node_t *key, node_t *value);

node_t *
node_make_fields(node_t *node, list_t *list);

node_t *
node_make_generic(node_t *node, node_t *key, node_t *type, node_t *value);

node_t *
node_make_generics(node_t *node, list_t *list);

node_t *
node_make_func(node_t *node,uint64_t flag,node_t *key, node_t *generics, node_t *parameters, node_t *result, node_t *block);

node_t *
node_make_lambda(node_t *node, node_t *key, node_t *generics, node_t *parameters, node_t *body);

node_t *
node_make_fn(node_t *node, node_t *parameters, node_t *result);

node_t *
node_make_heritage(node_t *node, node_t *key, node_t *type);

node_t *
node_make_heritages(node_t *node, list_t *list);

node_t *
node_make_property(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value, node_t *set, node_t *get);

node_t *
node_make_pair(node_t *node, node_t *key, node_t *value);

node_t *
node_make_entiery(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value);

node_t *
node_make_class(node_t *node, uint64_t flag, node_t *name, node_t *generics, node_t *heritages, node_t *block);

node_t *
node_make_member(node_t *node, node_t *name, node_t *value);

node_t *
node_make_members(node_t *node, list_t *list);

node_t *
node_make_enum(node_t *node, uint64_t flag, node_t *name, node_t *members);

node_t *
node_make_body(node_t *node, list_t *objects);

node_t *
node_make_block(node_t *node, list_t *stmt);

node_t *
node_make_package(node_t *node, node_t *key, node_t *generics, node_t *route);

node_t *
node_make_packages(node_t *node, list_t *list);

node_t *
node_make_import(node_t *node, node_t *path, node_t *fields);

node_t *
node_make_module(node_t *node, char *path, list_t *items);

#endif /* __NODE_H__ */
