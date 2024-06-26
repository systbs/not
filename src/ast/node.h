#pragma once
#ifndef __NODE_H__
#define __NODE_H__ 1

typedef struct not_node
{
	int32_t kind;
	uint64_t id;
	not_position_t position;

	void *value;

	struct not_node *previous;
	struct not_node *next;

	struct not_node *parent;
} not_node_t;

typedef enum not_node_kind
{
	NODE_KIND_RAW = 0,

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
} not_node_kind_t;

typedef struct not_node_basic
{
	char *value;
} not_node_basic_t;

typedef struct not_node_block
{
	not_node_t *items;
} not_node_block_t;

typedef struct not_node_carrier
{
	not_node_t *base;
	not_node_t *data;
} not_node_carrier_t;

typedef struct not_node_unary
{
	not_node_t *right;
} not_node_unary_t;

typedef struct not_node_binary
{
	not_node_t *left;
	not_node_t *right;
} not_node_binary_t;

typedef struct not_node_triple
{
	not_node_t *base;
	not_node_t *left;
	not_node_t *right;
} not_node_triple_t;

typedef struct not_node_if
{
	not_node_t *condition;
	not_node_t *then_body;
	not_node_t *else_body;
} not_node_if_t;

typedef struct not_node_for
{
	not_node_t *key;
	not_node_t *initializer;
	not_node_t *condition;
	not_node_t *incrementor;
	not_node_t *body;
} not_node_for_t;

typedef struct not_node_forin
{
	not_node_t *key;
	not_node_t *field;
	not_node_t *value;
	not_node_t *iterator;
	not_node_t *body;
} not_node_forin_t;

typedef struct not_node_catch
{
	not_node_t *parameters;
	not_node_t *body;
	not_node_t *next;
} not_node_catch_t;

typedef struct not_node_try
{
	not_node_t *body;
	not_node_t *catchs;
} not_node_try_t;

typedef struct not_node_var
{
	uint64_t flag;
	not_node_t *key;
	not_node_t *type;
	not_node_t *value;
} not_node_var_t;

typedef struct not_node_parameter
{
	uint64_t flag;
	not_node_t *key;
	not_node_t *type;
	not_node_t *value;
} not_node_parameter_t;

typedef struct not_node_field
{
	not_node_t *key;
	not_node_t *value;
} not_node_field_t;

typedef struct not_node_argument
{
	not_node_t *key;
	not_node_t *value;
} not_node_argument_t;

typedef struct not_node_heritage
{
	not_node_t *key;
	not_node_t *type;
} not_node_heritage_t;

typedef struct not_node_generic
{
	not_node_t *key;
	not_node_t *type;
	not_node_t *value;
} not_node_generic_t;

typedef struct not_node_body
{
	not_node_t *declaration;
} not_node_body_t;

typedef struct not_node_lambda
{
	not_node_t *key;
	not_node_t *generics;
	not_node_t *parameters;
	not_node_t *body;
	not_node_t *result;
} not_node_lambda_t;

typedef struct not_node_fun
{
	uint64_t flag;
	not_node_t *notes;
	not_node_t *generics;
	not_node_t *key;
	not_node_t *parameters;
	not_node_t *result;
	not_node_t *body;
} not_node_fun_t;

typedef struct not_node_property
{
	uint64_t flag;
	not_node_t *notes;
	not_node_t *key;
	not_node_t *type;
	not_node_t *value;
} not_node_property_t;

typedef struct not_node_entity
{
	uint64_t flag;
	not_node_t *key;
	not_node_t *type;
	not_node_t *value;
} not_node_entity_t;

typedef struct not_node_pair
{
	not_node_t *key;
	not_node_t *value;
} not_node_pair_t;

typedef struct not_node_class
{
	uint64_t flag;
	not_node_t *notes;
	not_node_t *key;
	not_node_t *heritages;
	not_node_t *generics;
	not_node_t *block;
} not_node_class_t;

typedef struct not_node_note
{
	not_node_t *key;
	not_node_t *arguments;

	not_node_t *next;
} not_node_note_t;

typedef struct not_node_package
{
	not_node_t *key;
	not_node_t *value;
} not_node_package_t;

typedef struct not_node_using
{
	char *base;
	not_node_t *path;
	not_node_t *packages;
} not_node_using_t;

const char *
not_node_kind_as_string(not_node_t *node);

void not_node_destroy(not_node_t *node);

not_node_t *
not_node_create(not_node_t *parent, not_position_t position);

void not_node_remove(not_node_t *node);

not_node_t *
not_node_make_id(not_node_t *node, char *value);

not_node_t *
not_node_make_number(not_node_t *node, char *value);

not_node_t *
not_node_make_char(not_node_t *node, char *value);

not_node_t *
not_node_make_string(not_node_t *node, char *value);

not_node_t *
not_node_make_null(not_node_t *node);

not_node_t *
not_node_make_kint(not_node_t *node);

not_node_t *
not_node_make_kfloat(not_node_t *node);

not_node_t *
not_node_make_kchar(not_node_t *node);

not_node_t *
not_node_make_kstring(not_node_t *node);

not_node_t *
not_node_make_this(not_node_t *node);

not_node_t *
not_node_make_self(not_node_t *node);

not_node_t *
not_node_make_tuple(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_object(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_pseudonym(not_node_t *node, not_node_t *base, not_node_t *arguments);

not_node_t *
not_node_make_typeof(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_sizeof(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_parenthesis(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_throw(not_node_t *node, not_node_t *arguments);

not_node_t *
not_node_make_argument(not_node_t *node, not_node_t *name, not_node_t *value);

not_node_t *
not_node_make_arguments(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_call(not_node_t *node, not_node_t *name, not_node_t *arguments);

not_node_t *
not_node_make_array(not_node_t *node, not_node_t *base, not_node_t *arguments);

not_node_t *
not_node_make_attribute(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_tilde(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_not(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_neg(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_pos(not_node_t *node, not_node_t *right);

not_node_t *
not_node_make_pow(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_epi(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_mul(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_div(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_mod(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_plus(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_minus(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_shl(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_shr(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_lt(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_le(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_gt(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_ge(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_eq(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_neq(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_and(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_xor(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_or(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_land(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_lor(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_instanceof(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_conditional(not_node_t *node, not_node_t *base, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_add_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_sub_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_mul_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_div_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_epi_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_mod_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_pow_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_and_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_or_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_shl_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_shr_assign(not_node_t *node, not_node_t *left, not_node_t *right);

not_node_t *
not_node_make_if(not_node_t *node, not_node_t *condition, not_node_t *then_body, not_node_t *else_body);

not_node_t *
not_node_make_for(not_node_t *node, not_node_t *name, not_node_t *initializer, not_node_t *condition, not_node_t *incrementor, not_node_t *body);

not_node_t *
not_node_make_forin(not_node_t *node, not_node_t *key, not_node_t *field, not_node_t *value, not_node_t *iterator, not_node_t *body);

not_node_t *
not_node_make_break(not_node_t *node, not_node_t *expression);

not_node_t *
not_node_make_continue(not_node_t *node, not_node_t *expression);

not_node_t *
not_node_make_catch(not_node_t *node, not_node_t *parameters, not_node_t *body);

not_node_t *
not_node_make_try(not_node_t *node, not_node_t *body, not_node_t *catchs);

not_node_t *
not_node_make_return(not_node_t *node, not_node_t *expr);

not_node_t *
not_node_make_var(not_node_t *node, uint64_t flag, not_node_t *name, not_node_t *type, not_node_t *value);

not_node_t *
not_node_make_parameter(not_node_t *node, uint64_t flag, not_node_t *name, not_node_t *type, not_node_t *value);

not_node_t *
not_node_make_parameters(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_field(not_node_t *node, not_node_t *key, not_node_t *value);

not_node_t *
not_node_make_fields(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_generic(not_node_t *node, not_node_t *key, not_node_t *type, not_node_t *value);

not_node_t *
not_node_make_generics(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_func(not_node_t *node, not_node_t *note, uint64_t flag, not_node_t *key, not_node_t *generics, not_node_t *parameters, not_node_t *result, not_node_t *body);

not_node_t *
not_node_make_lambda(not_node_t *node, not_node_t *key, not_node_t *generics, not_node_t *parameters, not_node_t *body, not_node_t *result);

not_node_t *
not_node_make_heritage(not_node_t *node, not_node_t *key, not_node_t *type);

not_node_t *
not_node_make_heritages(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_property(not_node_t *node, not_node_t *note, uint64_t flag, not_node_t *key, not_node_t *type, not_node_t *value);

not_node_t *
not_node_make_pair(not_node_t *node, not_node_t *key, not_node_t *value);

not_node_t *
not_node_make_entity(not_node_t *node, uint64_t flag, not_node_t *key, not_node_t *type, not_node_t *value);

not_node_t *
not_node_make_set(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_class(not_node_t *node, not_node_t *note, uint64_t flag, not_node_t *name, not_node_t *generics, not_node_t *heritages, not_node_t *block);

not_node_t *
not_node_make_note(not_node_t *node, not_node_t *key, not_node_t *arguments);

not_node_t *
not_node_make_notes(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_package(not_node_t *node, not_node_t *key, not_node_t *value);

not_node_t *
not_node_make_packages(not_node_t *node, not_node_t *items);

not_node_t *
not_node_make_using(not_node_t *node, char *base, not_node_t *path, not_node_t *packages);

not_node_t *
not_node_make_body(not_node_t *node, not_node_t *declaration);

not_node_t *
not_node_make_module(not_node_t *node, not_node_t *items);

#endif /* __NODE_H__ */
