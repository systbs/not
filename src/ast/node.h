#pragma once
#ifndef __NODE_H__
#define __NODE_H__ 1

typedef struct sy_node {
	int32_t kind;
	uint64_t id;
	sy_position_t position;

	void *value;

	struct sy_node *previous;
	struct sy_node *next;

	struct sy_node *parent;
} sy_node_t;

typedef enum sy_node_kind {
	NODE_KIND_ID,

	NODE_KIND_NUMBER,
	NODE_KIND_CHAR,
	NODE_KIND_STRING,

	NODE_KIND_TUPLE,
	NODE_KIND_OBJECT,

	NODE_KIND_PSEUDONYM,
	
	NODE_KIND_NULL,
	NODE_KIND_KINT,
	NODE_KIND_KFLOAT,
	NODE_KIND_KCHAR,
	NODE_KIND_KSTRING,

	NODE_KIND_TYPEOF,
	NODE_KIND_SIZEOF,
	NODE_KIND_PARENTHESIS,
	
	NODE_KIND_CALL,
	NODE_KIND_ARRAY,
	NODE_KIND_ATTRIBUTE,
	
	NODE_KIND_TILDE,
	NODE_KIND_NOT,
	NODE_KIND_NEG,
	NODE_KIND_POS,
	
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

	NODE_KIND_INSTANCEOF,
	NODE_KIND_CONDITIONAL,
	
	NODE_KIND_ASSIGN,
	NODE_KIND_ADD_ASSIGN,
	NODE_KIND_SUB_ASSIGN,
	NODE_KIND_MUL_ASSIGN,
	NODE_KIND_DIV_ASSIGN,
	NODE_KIND_EPI_ASSIGN,
	NODE_KIND_MOD_ASSIGN,
	NODE_KIND_POW_ASSIGN,
	NODE_KIND_AND_ASSIGN,
	NODE_KIND_OR_ASSIGN,
	NODE_KIND_SHL_ASSIGN,
	NODE_KIND_SHR_ASSIGN,

	NODE_KIND_IF,
	NODE_KIND_FOR,
	NODE_KIND_FORIN,
	NODE_KIND_BREAK,
	NODE_KIND_CONTINUE,
	NODE_KIND_CATCH,
	NODE_KIND_TRY,
	NODE_KIND_RETURN,
	NODE_KIND_THROW,
	
	NODE_KIND_VAR,
	NODE_KIND_LAMBDA,
	NODE_KIND_BODY,
	NODE_KIND_FUN,
	NODE_KIND_CLASS,
	NODE_KIND_PAIR,
	NODE_KIND_ENTITY,
	NODE_KIND_SET,
	NODE_KIND_PROPERTY,
	NODE_KIND_NOTE,
	NODE_KIND_NOTES,
	NODE_KIND_ARGUMENT,
	NODE_KIND_ARGUMENTS,
	NODE_KIND_PARAMETER,
	NODE_KIND_PARAMETERS,
	NODE_KIND_FIELD,
	NODE_KIND_FIELDS,
	NODE_KIND_GENERIC,
	NODE_KIND_GENERICS,
	NODE_KIND_HERITAGE,
	NODE_KIND_HERITAGES,
	NODE_KIND_PACKAGE,
	NODE_KIND_PACKAGES,
	NODE_KIND_USING,
	NODE_KIND_MODULE
} sy_node_kind_t;

typedef struct sy_node_basic {
	char *value;
} sy_node_basic_t;

typedef struct sy_node_block {
	sy_node_t *items;
} sy_node_block_t;

typedef struct sy_node_carrier {
	sy_node_t *base;
	sy_node_t *data;
} sy_node_carrier_t;

typedef struct sy_node_unary {
	sy_node_t *right;
} sy_node_unary_t;

typedef struct sy_node_binary {
	sy_node_t *left;
	sy_node_t *right;
} sy_node_binary_t;

typedef struct sy_node_triple {
	sy_node_t *base;
	sy_node_t *left;
	sy_node_t *right;
} sy_node_triple_t;


typedef struct sy_node_if {
	sy_node_t *condition;
	sy_node_t *then_body;
	sy_node_t *else_body;
} sy_node_if_t;

typedef struct sy_node_for {
	sy_node_t *key;
	sy_node_t *initializer;
	sy_node_t *condition;
	sy_node_t *incrementor;
	sy_node_t *body;
} sy_node_for_t;

typedef struct sy_node_forin {
	sy_node_t *key;
	sy_node_t *field;
	sy_node_t *value;
	sy_node_t *iterator;
	sy_node_t *body;
} sy_node_forin_t;

typedef struct sy_node_catch {
	sy_node_t *parameters;
	sy_node_t *body;
	sy_node_t *next;
} sy_node_catch_t;

typedef struct sy_node_try {
	sy_node_t *body;
	sy_node_t *catchs;
} sy_node_try_t;

typedef struct sy_node_var {
	uint64_t flag;
	sy_node_t *key;
	sy_node_t *type;
	sy_node_t *value;
} sy_node_var_t;

typedef struct sy_node_parameter {
	uint64_t flag;
	sy_node_t *key;
	sy_node_t *type;
	sy_node_t *value;
} sy_node_parameter_t;

typedef struct sy_node_field {
	sy_node_t *key;
	sy_node_t *value;
} sy_node_field_t;

typedef struct sy_node_argument {
	sy_node_t *key;
	sy_node_t *value;
} sy_node_argument_t;

typedef struct sy_node_heritage {
	sy_node_t *key;
	sy_node_t *type;
} sy_node_heritage_t;

typedef struct sy_node_generic {
	sy_node_t *key;
	sy_node_t *type;
	sy_node_t *value;
} sy_node_generic_t;

typedef struct sy_node_body {
	sy_node_t *declaration;
} sy_node_body_t;

typedef struct sy_node_lambda {
	sy_node_t *generics;
	sy_node_t *parameters;
	sy_node_t *body;
	sy_node_t *result;
} sy_node_lambda_t;

typedef struct sy_node_fun {
	uint64_t flag;
	sy_node_t *notes;
	sy_node_t *generics;
	sy_node_t *key;
	sy_node_t *parameters;
	sy_node_t *result;
	sy_node_t *body;
} sy_node_fun_t;

typedef struct sy_node_property {
	uint64_t flag;
	sy_node_t *notes;
	sy_node_t *key;
	sy_node_t *type;
	sy_node_t *value;
} sy_node_property_t;

typedef struct sy_node_entity {
	uint64_t flag;
	sy_node_t *key;
	sy_node_t *type;
	sy_node_t *value;
} sy_node_entity_t;

typedef struct sy_node_pair {
	sy_node_t *key;
	sy_node_t *value;
} sy_node_pair_t;

typedef struct sy_node_class {
	uint64_t flag;
	sy_node_t *notes;
	sy_node_t *key;
	sy_node_t *heritages;
	sy_node_t *generics;
	sy_node_t *block;
} sy_node_class_t;

typedef struct sy_node_note {
	sy_node_t *key;
	sy_node_t *arguments;

	sy_node_t *next;
} sy_node_note_t;

typedef struct sy_node_package {
	sy_node_t *key;
	sy_node_t *value;
} sy_node_package_t;

typedef struct sy_node_using {
	sy_node_t *path;
	sy_node_t *packages;
} sy_node_using_t;


const char *
sy_node_kind_as_string(sy_node_t *node);

void
sy_node_destroy(sy_node_t *node);

sy_node_t *
sy_node_create(sy_node_t *parent, sy_position_t position);

void
sy_node_remove(sy_node_t *node);



sy_node_t *
sy_node_make_id(sy_node_t *node, char *value);

sy_node_t *
sy_node_make_number(sy_node_t *node, char *value);

sy_node_t *
sy_node_make_char(sy_node_t *node, char *value);

sy_node_t *
sy_node_make_string(sy_node_t *node, char *value);


sy_node_t *
sy_node_make_null(sy_node_t *node);

sy_node_t *
sy_node_make_kint(sy_node_t *node);

sy_node_t *
sy_node_make_kfloat(sy_node_t *node);

sy_node_t *
sy_node_make_kchar(sy_node_t *node);

sy_node_t *
sy_node_make_kstring(sy_node_t *node);


sy_node_t *
sy_node_make_this(sy_node_t *node);

sy_node_t *
sy_node_make_self(sy_node_t *node);


sy_node_t *
sy_node_make_tuple(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_object(sy_node_t *node, sy_node_t *items);


sy_node_t *
sy_node_make_pseudonym(sy_node_t *node, sy_node_t *base, sy_node_t *arguments);

sy_node_t *
sy_node_make_typeof(sy_node_t *node, sy_node_t *right);

sy_node_t *
sy_node_make_sizeof(sy_node_t *node, sy_node_t *right);

sy_node_t *
sy_node_make_parenthesis(sy_node_t *node, sy_node_t *right);



sy_node_t *
sy_node_make_throw(sy_node_t *node, sy_node_t *arguments);

sy_node_t *
sy_node_make_argument(sy_node_t *node, sy_node_t *name, sy_node_t *value);

sy_node_t *
sy_node_make_arguments(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_call(sy_node_t *node, sy_node_t *name, sy_node_t *arguments);

sy_node_t *
sy_node_make_array(sy_node_t *node, sy_node_t *base, sy_node_t *arguments);

sy_node_t *
sy_node_make_attribute(sy_node_t *node, sy_node_t *left, sy_node_t *right);



sy_node_t *
sy_node_make_tilde(sy_node_t *node, sy_node_t *right);

sy_node_t *
sy_node_make_not(sy_node_t *node, sy_node_t *right);

sy_node_t *
sy_node_make_neg(sy_node_t *node, sy_node_t *right);

sy_node_t *
sy_node_make_pos(sy_node_t *node, sy_node_t *right);



sy_node_t *
sy_node_make_pow(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_epi(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_mul(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_div(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_mod(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_plus(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_minus(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_shl(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_shr(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_lt(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_le(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_gt(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_ge(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_eq(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_neq(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_and(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_xor(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_or(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_land(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_lor(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_instanceof(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_conditional(sy_node_t *node, sy_node_t *base, sy_node_t *left, sy_node_t *right);


sy_node_t *
sy_node_make_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_add_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_sub_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_mul_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_div_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_epi_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_mod_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_pow_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_and_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_or_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_shl_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);

sy_node_t *
sy_node_make_shr_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right);


sy_node_t *
sy_node_make_if(sy_node_t *node, sy_node_t *condition, sy_node_t *then_body, sy_node_t *else_body);

sy_node_t *
sy_node_make_for(sy_node_t *node, sy_node_t *name, sy_node_t *initializer, sy_node_t *condition, sy_node_t *incrementor, sy_node_t *body);

sy_node_t *
sy_node_make_forin(sy_node_t *node, sy_node_t *key, sy_node_t *field, sy_node_t *value, sy_node_t *iterator, sy_node_t *body);

sy_node_t *
sy_node_make_break(sy_node_t *node, sy_node_t *expression);

sy_node_t *
sy_node_make_continue(sy_node_t *node, sy_node_t *expression);

sy_node_t *
sy_node_make_catch(sy_node_t *node, sy_node_t *parameters, sy_node_t *body);

sy_node_t *
sy_node_make_try(sy_node_t *node, sy_node_t *body, sy_node_t *catchs);

sy_node_t *
sy_node_make_return(sy_node_t *node, sy_node_t *expr);

sy_node_t *
sy_node_make_var(sy_node_t *node, uint64_t flag, sy_node_t *name, sy_node_t *type, sy_node_t *value);

sy_node_t *
sy_node_make_parameter(sy_node_t *node, uint64_t flag, sy_node_t *name, sy_node_t *type, sy_node_t *value);

sy_node_t *
sy_node_make_parameters(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_field(sy_node_t *node, sy_node_t *key, sy_node_t *value);

sy_node_t *
sy_node_make_fields(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_generic(sy_node_t *node, sy_node_t *key, sy_node_t *type, sy_node_t *value);

sy_node_t *
sy_node_make_generics(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_func(sy_node_t *node, sy_node_t *note, uint64_t flag, sy_node_t *key, sy_node_t *generics, sy_node_t *parameters, sy_node_t *result, sy_node_t *body);

sy_node_t *
sy_node_make_lambda(sy_node_t *node, sy_node_t *generics, sy_node_t *parameters, sy_node_t *body, sy_node_t *result);

sy_node_t *
sy_node_make_heritage(sy_node_t *node, sy_node_t *key, sy_node_t *type);

sy_node_t *
sy_node_make_heritages(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_property(sy_node_t *node, sy_node_t *note, uint64_t flag, sy_node_t *key, sy_node_t *type, sy_node_t *value);

sy_node_t *
sy_node_make_pair(sy_node_t *node, sy_node_t *key, sy_node_t *value);

sy_node_t *
sy_node_make_entity(sy_node_t *node, uint64_t flag, sy_node_t *key, sy_node_t *type, sy_node_t *value);

sy_node_t *
sy_node_make_set(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_class(sy_node_t *node, sy_node_t *note, uint64_t flag, sy_node_t *name, sy_node_t *generics, sy_node_t *heritages, sy_node_t *block);

sy_node_t *
sy_node_make_note(sy_node_t *node, sy_node_t *key, sy_node_t *arguments);

sy_node_t *
sy_node_make_notes(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_package(sy_node_t *node, sy_node_t *key, sy_node_t *value);

sy_node_t *
sy_node_make_packages(sy_node_t *node, sy_node_t *items);

sy_node_t *
sy_node_make_using(sy_node_t *node, sy_node_t *path, sy_node_t *packages);

sy_node_t *
sy_node_make_body(sy_node_t *node, sy_node_t *declaration);

sy_node_t *
sy_node_make_module(sy_node_t *node, sy_node_t *items);

#endif /* __NODE_H__ */
