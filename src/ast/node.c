#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "node.h"
#include "../parser/parser.h"

uint64_t node_counter = 0;

static node_t *
node_make_node(position_t position, int32_t kind, void *value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = kind;
	node->position = position;
	node->value = value;
	node->id = node_counter++;
	node->locals = list_create();
	if(!node->locals)
	{
		return NULL;
	}

	return node;
}

node_t *
node_make_id(position_t position, char *value){
	node_basic_t *node_basic;
	if(!(node_basic = (node_basic_t *)malloc(sizeof(node_basic_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(node_basic, 0, sizeof(node_basic_t));
	node_basic->value = value;

	return node_make_node(position, NODE_KIND_ID, node_basic);
}

node_t *
node_make_number(position_t position, char *value)
{
	node_basic_t *node_basic;
	if(!(node_basic = (node_basic_t *)malloc(sizeof(node_basic_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(node_basic, 0, sizeof(node_basic_t));
	node_basic->value = value;
	
	return node_make_node(position, NODE_KIND_NUMBER, node_basic);
}

node_t *
node_make_char(position_t position, char *value)
{
	node_basic_t *node_basic;
	if(!(node_basic = (node_basic_t *)malloc(sizeof(node_basic_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(node_basic, 0, sizeof(node_basic_t));
	node_basic->value = value;
	
	return node_make_node(position, NODE_KIND_CHAR, node_basic);
}

node_t *
node_make_string(position_t position, char *value)
{
	node_basic_t *node_basic;
	if(!(node_basic = (node_basic_t *)malloc(sizeof(node_basic_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(node_basic, 0, sizeof(node_basic_t));
	node_basic->value = value;
	
	return node_make_node(position, NODE_KIND_STRING, node_basic);
}

node_t *
node_make_null(position_t position){
	return node_make_node(position, NODE_KIND_NULL, NULL);
}

node_t *
node_make_true(position_t position){
	return node_make_node(position, NODE_KIND_TRUE, NULL);
}

node_t *
node_make_false(position_t position){
	return node_make_node(position, NODE_KIND_FALSE, NULL);
}

node_t *
node_make_array(position_t position, list_t *list){
	node_array_t *node_array;
	if(!(node_array = (node_array_t *)malloc(sizeof(node_array_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_array_t));
		return NULL;
	}
	memset(node_array, 0, sizeof(node_array_t));
	node_array->list = list;
	
	return node_make_node(position, NODE_KIND_ARRAY, node_array);
}

node_t *
node_make_object_property(position_t position, node_t *name, node_t *value){
	node_object_property_t *node_object_property;
	if(!(node_object_property = (node_object_property_t *)malloc(sizeof(node_object_property_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_object_property_t));
		return NULL;
	}
	memset(node_object_property, 0, sizeof(node_object_property_t));
	node_object_property->name = name;
	node_object_property->value = value;
	
	return node_make_node(position, NODE_KIND_OBJECT_PROPERTY, node_object_property);
}

node_t *
node_make_object(position_t position, list_t *list){
	node_object_t *node_object;
	if(!(node_object = (node_object_t *)malloc(sizeof(node_object_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_object_t));
		return NULL;
	}
	memset(node_object, 0, sizeof(node_object_t));
	node_object->list = list;
	
	return node_make_node(position, NODE_KIND_OBJECT, node_object);
}

node_t *
node_make_composite(position_t position, node_t *base, list_t *type_arguments){
	node_composite_t *node_composite;
	if(!(node_composite = (node_composite_t *)malloc(sizeof(node_composite_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_composite_t));
		return NULL;
	}
	memset(node_composite, 0, sizeof(node_composite_t));
	node_composite->base = base;
	node_composite->type_arguments = type_arguments;
	
	return node_make_node(position, NODE_KIND_COMPOSITE, node_composite);
}






node_t *
node_make_in(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_IN, node_binary);
}

node_t *
node_make_typeof(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_TYPEOF, node_unary);
}

node_t *
node_make_sizeof(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_SIZEOF, node_unary);
}

node_t *
node_make_parenthesis(position_t position, node_t *value)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = value;
	
	return node_make_node(position, NODE_KIND_PARENTHESIS, node_unary);
}

node_t *
node_make_argument(position_t position, list_t *value)
{
	node_argument_t *node_argument;
	if(!(node_argument = (node_argument_t *)malloc(sizeof(node_argument_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_argument_t));
		return NULL;
	}
	memset(node_argument, 0, sizeof(node_argument_t));
	node_argument->value = value;
	
	return node_make_node(position, NODE_KIND_ARGUMENT, node_argument);
}

node_t *
node_make_call(position_t position, node_t *callable, list_t *arguments)
{
	node_call_t *node_call;
	if(!(node_call = (node_call_t *)malloc(sizeof(node_call_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_call_t));
		return NULL;
	}
	memset(node_call, 0, sizeof(node_call_t));
	node_call->callable = callable;
	node_call->arguments = arguments;
	
	return node_make_node(position, NODE_KIND_CALL, node_call);
}

node_t *
node_make_get_slice(position_t position, node_t *name, node_t *start, node_t *stop, node_t *step)
{
	node_get_slice_t *node_get_slice;
	if(!(node_get_slice = (node_get_slice_t *)malloc(sizeof(node_get_slice_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_get_slice_t));
		return NULL;
	}
	memset(node_get_slice, 0, sizeof(node_get_slice_t));
	node_get_slice->name = name;
	node_get_slice->start = start;
	node_get_slice->stop = stop;
	node_get_slice->step = step;
	
	return node_make_node(position, NODE_KIND_GET_SLICE, node_get_slice);
}

node_t *
node_make_get_item(position_t position, node_t *name, node_t *index)
{
	node_get_item_t *node_get_item;
	if(!(node_get_item = (node_get_item_t *)malloc(sizeof(node_get_item_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_get_item_t));
		return NULL;
	}
	memset(node_get_item, 0, sizeof(node_get_item_t));
	node_get_item->name = name;
	node_get_item->index = index;
	
	return node_make_node(position, NODE_KIND_GET_ITEM, node_get_item);
}

node_t *
node_make_get_attr(position_t position, node_t *left, node_t *right)
{
	node_get_attr_t *node_get_attr;
	if(!(node_get_attr = (node_get_attr_t *)malloc(sizeof(node_get_attr_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_get_attr_t));
		return NULL;
	}
	memset(node_get_attr, 0, sizeof(node_get_attr_t));
	node_get_attr->left = left;
	node_get_attr->right = right;
	
	return node_make_node(position, NODE_KIND_GET_ATTR, node_get_attr);
}

node_t *
node_make_tilde(position_t position, node_t *right)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_TILDE, node_unary);
}

node_t *
node_make_not(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_NOT, node_unary);
}

node_t *
node_make_neg(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_NEG, node_unary);
}

node_t *
node_make_pos(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_POS, node_unary);
}

node_t *
node_make_get_value(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_GET_VALUE, node_unary);
}

node_t *
node_make_get_address(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_GET_ADDRESS, node_unary);
}

node_t *
node_make_ellipsis(position_t position, node_t *right){
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = right;
	
	return node_make_node(position, NODE_KIND_ELLIPSIS, node_unary);
}



node_t *
node_make_mul(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_MUL, node_binary);
}

node_t *
node_make_div(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_DIV, node_binary);
}

node_t *
node_make_mod(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_MOD, node_binary);
}

node_t *
node_make_plus(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_PLUS, node_binary);
}

node_t *
node_make_minus(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_MINUS, node_binary);
}

node_t *
node_make_shl(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_SHL, node_binary);
}

node_t *
node_make_shr(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_SHR, node_binary);
}

node_t *
node_make_lt(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_LT, node_binary);
}

node_t *
node_make_le(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_LE, node_binary);
}

node_t *
node_make_gt(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_GT, node_binary);
}

node_t *
node_make_ge(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_GE, node_binary);
}

node_t *
node_make_eq(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_EQ, node_binary);
}

node_t *
node_make_neq(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_NEQ, node_binary);
}

node_t *
node_make_extends(position_t position, node_t *left, node_t *right)
{
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_EXTENDS, node_binary);
}

node_t *
node_make_and(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_AND, node_binary);
}

node_t *
node_make_xor(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_XOR, node_binary);
}

node_t *
node_make_or(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_OR, node_binary);
}

node_t *
node_make_land(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_LAND, node_binary);
}

node_t *
node_make_lor(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_LOR, node_binary);
}

node_t *
node_make_conditional(position_t position, node_t *condition, node_t *true_expression, node_t *false_expression)
{
	node_conditional_t *node_conditional;
	if(!(node_conditional = (node_conditional_t *)malloc(sizeof(node_conditional_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_conditional_t));
		return NULL;
	}
	memset(node_conditional, 0, sizeof(node_conditional_t));
	node_conditional->condition = condition;
	node_conditional->true_expression = true_expression;
	node_conditional->false_expression = false_expression;
	
	return node_make_node(position, NODE_KIND_CONDITIONAL, node_conditional);
}







node_t *
node_make_assign(position_t position, node_t *left, node_t *right)
{
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_ASSIGN, node_binary);
}

node_t *
node_make_define(position_t position, node_t *left, node_t *right)
{
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_DEFINE, node_binary);
}

node_t *
node_make_add_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_ADD_ASSIGN, node_binary);
}

node_t *
node_make_sub_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_SUB_ASSIGN, node_binary);
}

node_t *
node_make_div_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_DIV_ASSIGN, node_binary);
}

node_t *
node_make_mul_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_MUL_ASSIGN, node_binary);
}

node_t *
node_make_mod_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_MOD_ASSIGN, node_binary);
}

node_t *
node_make_and_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_AND_ASSIGN, node_binary);
}

node_t *
node_make_or_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_OR_ASSIGN, node_binary);
}

node_t *
node_make_shl_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_SHL_ASSIGN, node_binary);
}

node_t *
node_make_shr_assign(position_t position, node_t *left, node_t *right){
	node_binary_t *node_binary;
	if(!(node_binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(node_binary, 0, sizeof(node_binary_t));
	node_binary->left = left;
	node_binary->right = right;
	
	return node_make_node(position, NODE_KIND_SHR_ASSIGN, node_binary);
}



node_t *
node_make_if(position_t position, node_t *name, node_t *condition, node_t *then_body, node_t *else_body)
{
	node_if_t *node_if;
	if(!(node_if = (node_if_t *)malloc(sizeof(node_if_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_if_t));
		return NULL;
	}

	memset(node_if, 0, sizeof(node_if_t));
	
	node_if->name = name;
	node_if->condition = condition;
	node_if->then_body = then_body;
	node_if->else_body = else_body;
	
	return node_make_node(position, NODE_KIND_IF, node_if);
}

node_t *
node_make_for_init_list(position_t position, list_t *list){
	node_enumerable_t *node_enumerable;
	if(!(node_enumerable = (node_enumerable_t *)malloc(sizeof(node_enumerable_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_enumerable_t));
		return NULL;
	}
	memset(node_enumerable, 0, sizeof(node_enumerable_t));
	node_enumerable->list = list;
	
	return node_make_node(position, NODE_KIND_FOR_INIT_LIST, node_enumerable);
}

node_t *
node_make_for_step_list(position_t position, list_t *list){
	node_enumerable_t *node_enumerable;
	if(!(node_enumerable = (node_enumerable_t *)malloc(sizeof(node_enumerable_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_enumerable_t));
		return NULL;
	}
	memset(node_enumerable, 0, sizeof(node_enumerable_t));
	node_enumerable->list = list;
	
	return node_make_node(position, NODE_KIND_FOR_STEP_LIST, node_enumerable);
}

node_t *
node_make_for(position_t position, node_t *name, node_t *initializer, node_t *condition, node_t *incrementor, node_t *body)
{
	node_for_t *node_for;
	if(!(node_for = (node_for_t *)malloc(sizeof(node_for_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_for_t));
		return NULL;
	}
	
	memset(node_for, 0, sizeof(node_for_t));

	node_for->name = name;
	node_for->initializer = initializer;
	node_for->condition = condition;
	node_for->incrementor = incrementor;
	node_for->body = body;
	
	return node_make_node(position, NODE_KIND_FOR, node_for);
}

node_t *
node_make_forin(position_t position, node_t *name, node_t *initializer, node_t *expression, node_t *body)
{
	node_forin_t *node_forin;
	if(!(node_forin = (node_forin_t *)malloc(sizeof(node_forin_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_forin_t));
		return NULL;
	}
	memset(node_forin, 0, sizeof(node_forin_t));
	
	node_forin->name = name;
	node_forin->initializer = initializer;
	node_forin->expression = expression;
	node_forin->body = body;
	
	return node_make_node(position, NODE_KIND_FORIN, node_forin);
}

node_t *
node_make_break(position_t position, node_t *expression)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = expression;

	return node_make_node(position, NODE_KIND_BREAK, node_unary);
}

node_t *
node_make_continue(position_t position, node_t *expression)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = expression;

	return node_make_node(position, NODE_KIND_CONTINUE, node_unary);
}

node_t *
node_make_catch(position_t position, node_t *parameter, node_t *body)
{
	node_catch_t *node_catch;
	if(!(node_catch = (node_catch_t *)malloc(sizeof(node_catch_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_catch_t));
		return NULL;
	}
	memset(node_catch, 0, sizeof(node_catch_t));
	node_catch->parameter = parameter;
	node_catch->body = body;
	
	return node_make_node(position, NODE_KIND_CATCH, node_catch);
}

node_t *
node_make_try(position_t position, node_t *body, list_t *catchs)
{
	node_try_t *node_try;
	if(!(node_try = (node_try_t *)malloc(sizeof(node_try_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_try_t));
		return NULL;
	}
	memset(node_try, 0, sizeof(node_try_t));
	node_try->body = body;
	node_try->catchs = catchs;
	
	return node_make_node(position, NODE_KIND_TRY, node_try);
}

node_t *
node_make_return(position_t position, node_t *expression)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = expression;
	
	return node_make_node(position, NODE_KIND_RETURN, node_unary);
}

node_t *
node_make_throw(position_t position, node_t *expression)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = expression;
	
	return node_make_node(position, NODE_KIND_THROW, node_unary);
}


node_t *
node_make_type_arguments(position_t position, list_t *list){
	node_enumerable_t *node_enumerable;
	if(!(node_enumerable = (node_enumerable_t *)malloc(sizeof(node_enumerable_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_enumerable_t));
		return NULL;
	}
	memset(node_enumerable, 0, sizeof(node_enumerable_t));
	node_enumerable->list = list;
	
	return node_make_node(position, NODE_KIND_TYPE_ARGUMENTS, node_enumerable);
}

node_t *
node_make_var(position_t position, node_t *name, node_t *type, node_t *value){
	node_var_t *node_var;
	if(!(node_var = (node_var_t *)malloc(sizeof(node_var_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_var_t));
		return NULL;
	}
	memset(node_var, 0, sizeof(node_var_t));
	node_var->name = name;
	node_var->type = type;
	node_var->value = value;
	
	return node_make_node(position, NODE_KIND_VAR, node_var);
}

node_t *
node_make_const(position_t position, node_t *name, node_t *type, node_t *value){
	node_const_t *node_const;
	if(!(node_const = (node_const_t *)malloc(sizeof(node_const_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_const_t));
		return NULL;
	}
	memset(node_const, 0, sizeof(node_const_t));
	node_const->name = name;
	node_const->type = type;
	node_const->value = value;
	
	return node_make_node(position, NODE_KIND_CONST, node_const);
}

node_t *
node_make_type(position_t position, node_t *name, list_t *type_parameters, list_t *heritages, node_t *body)
{
	node_type_t *node_type;
	if(!(node_type = (node_type_t *)malloc(sizeof(node_type_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_type_t));
		return NULL;
	}
	memset(node_type, 0, sizeof(node_type_t));
	node_type->name = name;
	node_type->type_parameters = type_parameters;
	node_type->body = body;
	node_type->heritages = heritages;
	
	return node_make_node(position, NODE_KIND_TYPE, node_type);
}




node_t *
node_make_parameter(position_t position, node_t *name, node_t *type, node_t *value){
	node_parameter_t *node_parameter;
	if(!(node_parameter = (node_parameter_t *)malloc(sizeof(node_parameter_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_parameter_t));
		return NULL;
	}
	memset(node_parameter, 0, sizeof(node_parameter_t));
	node_parameter->name = name;
	node_parameter->type = type;
	node_parameter->value = value;
	
	return node_make_node(position, NODE_KIND_PARAMETER, node_parameter);
}

node_t *
node_make_field(position_t position, node_t *name, node_t *type)
{
	node_field_t *node_field;
	if(!(node_field = (node_field_t *)malloc(sizeof(node_field_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_field_t));
		return NULL;
	}
	memset(node_field, 0, sizeof(node_field_t));
	node_field->name = name;
	node_field->type = type;
	
	return node_make_node(position, NODE_KIND_FIELD, node_field);
}

node_t *
node_make_type_parameter(position_t position, node_t *name, node_t *extends, node_t *value)
{
	node_type_parameter_t *node_type_parameter;
	if(!(node_type_parameter = (node_type_parameter_t *)malloc(sizeof(node_type_parameter_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_type_parameter_t));
		return NULL;
	}
	memset(node_type_parameter, 0, sizeof(node_type_parameter_t));
	node_type_parameter->name = name;
	node_type_parameter->extends = extends;
	node_type_parameter->value = value;
	
	return node_make_node(position, NODE_KIND_TYPE_PARAMETER, node_type_parameter);
}

node_t *
node_make_func(position_t position, list_t *fields, list_t *type_parameters, node_t *name, list_t *parameters, node_t *body)
{
	node_func_t *node_func;
	if(!(node_func = (node_func_t *)malloc(sizeof(node_func_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_func_t));
		return NULL;
	}
	memset(node_func, 0, sizeof(node_func_t));
	node_func->fields = fields;
	node_func->type_parameters = type_parameters;
	node_func->name = name;
	node_func->parameters = parameters;
	node_func->body = body;
	
	return node_make_node(position, NODE_KIND_FUNC, node_func);
}

node_t *
node_make_method(position_t position, uint64_t flag, node_t *name, list_t *type_parameters, list_t *parameters, node_t *body)
{
	node_method_t *node_method;
	if(!(node_method = (node_method_t *)malloc(sizeof(node_method_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_method_t));
		return NULL;
	}
	memset(node_method, 0, sizeof(node_method_t));
	node_method->flag = flag;
	node_method->name = name;
	node_method->type_parameters = type_parameters;
	node_method->parameters = parameters;
	node_method->body = body;
	
	return node_make_node(position, NODE_KIND_METHOD, node_method);
}

node_t *
node_make_property(position_t position, uint64_t flag, node_t *name, node_t *type, node_t *value)
{
	node_property_t *node_property;
	if(!(node_property = (node_property_t *)malloc(sizeof(node_property_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_property_t));
		return NULL;
	}
	memset(node_property, 0, sizeof(node_property_t));
	node_property->flag = flag;
	node_property->name = name;
	node_property->type = type;
	node_property->value = value;
	
	return node_make_node(position, NODE_KIND_PROPERTY, node_property);
}

node_t *
node_make_heritage(position_t position, node_t *name, node_t *type)
{
	node_heritage_t *node_heritage;
	if(!(node_heritage = (node_heritage_t *)malloc(sizeof(node_heritage_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_heritage_t));
		return NULL;
	}
	memset(node_heritage, 0, sizeof(node_heritage_t));
	node_heritage->name = name;
	node_heritage->type = type;
	
	return node_make_node(position, NODE_KIND_HERITAGE, node_heritage);
}

node_t *
node_make_class(position_t position, uint64_t flag, node_t *name, list_t *type_parameters, list_t *heritages, list_t *body)
{
	node_class_t *node_class;
	if(!(node_class = (node_class_t *)malloc(sizeof(node_class_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_class_t));
		return NULL;
	}
	memset(node_class, 0, sizeof(node_class_t));
	node_class->flag = flag;
	node_class->name = name;
	node_class->type_parameters = type_parameters;
	node_class->heritages = heritages;
	node_class->body = body;
	
	return node_make_node(position, NODE_KIND_CLASS, node_class);
}

node_t *
node_make_enum_member(position_t position, node_t *name, node_t *value){
	node_enum_member_t *node_enum_member;
	if(!(node_enum_member = (node_enum_member_t *)malloc(sizeof(node_enum_member_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_enum_member_t));
		return NULL;
	}
	memset(node_enum_member, 0, sizeof(node_enum_member_t));
	node_enum_member->name = name;
	node_enum_member->value = value;
	
	return node_make_node(position, NODE_KIND_ENUM_MEMBER, node_enum_member);
}

node_t *
node_make_enum(position_t position, uint64_t flag, node_t *name, list_t *body)
{
	node_enum_t *node_enum;
	if(!(node_enum = (node_enum_t *)malloc(sizeof(node_enum_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_enum_t));
		return NULL;
	}
	memset(node_enum, 0, sizeof(node_enum_t));
	node_enum->flag = flag;
	node_enum->name = name;
	node_enum->body = body;
	
	return node_make_node(position, NODE_KIND_ENUM, node_enum);
}

node_t *
node_make_block(position_t position, list_t *list)
{
	node_block_t *node_block;
	if(!(node_block = (node_block_t *)malloc(sizeof(node_block_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(node_block, 0, sizeof(node_block_t));
	node_block->list = list;
	
	return node_make_node(position, NODE_KIND_BLOCK, node_block);
}

node_t *
node_make_import(position_t position, node_t *path, list_t *fields)
{
	node_import_t *node_import;
	if(!(node_import = (node_import_t *)malloc(sizeof(node_import_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_import_t));
		return NULL;
	}
	memset(node_import, 0, sizeof(node_import_t));
	node_import->path = path;
	node_import->fields = fields;
	
	return node_make_node(position, NODE_KIND_IMPORT, node_import);
}

node_t *
node_make_export_block(position_t position, list_t *list)
{
	node_block_t *node_block;
	if(!(node_block = (node_block_t *)malloc(sizeof(node_block_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(node_block, 0, sizeof(node_block_t));
	node_block->list = list;
	
	return node_make_node(position, NODE_KIND_EXPORT_BLOCK, node_block);
}

node_t *
node_make_export(position_t position, node_t *value)
{
	node_unary_t *node_unary;
	if(!(node_unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(node_unary, 0, sizeof(node_unary_t));
	node_unary->right = value;
	
	return node_make_node(position, NODE_KIND_EXPORT, node_unary);
}

node_t *
node_make_module(position_t position, char *path, list_t *members)
{
	node_module_t *node_module;
	if(!(node_module = (node_module_t *)malloc(sizeof(node_module_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_module_t));
		return NULL;
	}
	memset(node_module, 0, sizeof(node_module_t));
	node_module->members = members;
	node_module->path = path;
	
	return node_make_node(position, NODE_KIND_MODULE, node_module);
}
