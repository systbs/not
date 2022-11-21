#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "list.h"
#include "node.h"

node_t *
node_make_id(char *value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ID;
	
	node_id_t *id;
	if(!(id = (node_id_t *)malloc(sizeof(node_id_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_id_t));
		return NULL;
	}
	memset(id, 0, sizeof(node_id_t));
	id->value = value;
	
	node->value = id;
	return node;
}

node_t *
node_make_number(char *value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_NUMBER;
	
	node_number_t *number;
	if(!(number = (node_number_t *)malloc(sizeof(node_number_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_number_t));
		return NULL;
	}
	memset(number, 0, sizeof(node_number_t));
	number->value = value;
	
	node->value = number;
	return node;
}

node_t *
node_make_letters(char *value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LETTERS;
	
	node_letters_t *letters;
	if(!(letters = (node_letters_t *)malloc(sizeof(node_letters_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_letters_t));
		return NULL;
	}
	memset(letters, 0, sizeof(node_letters_t));
	letters->value = value;
	
	node->value = letters;
	return node;
}

node_t *
node_make_null(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_NULL;
	
	return node;
}

node_t *
node_make_this(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_THIS;
	
	return node;
}

node_t *
node_make_super(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SUPER;
	
	return node;
}

node_t *
node_make_list(list_t *expr_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LIST;
	
	node_list_t *list;
	if(!(list = (node_list_t *)malloc(sizeof(node_list_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_list_t));
		return NULL;
	}
	memset(list, 0, sizeof(node_list_t));
	list->expr_list = expr_list;
	
	node->value = list;
	return node;
}

node_t *
node_make_element(node_t *key, node_t *value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ELEMENT;
	
	node_element_t *element;
	if(!(element = (node_element_t *)malloc(sizeof(node_element_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_element_t));
		return NULL;
	}
	memset(element, 0, sizeof(node_element_t));
	element->key = key;
	element->value = value;
	
	node->value = element;
	return node;
}

node_t *
node_make_map(list_t *element_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MAP;
	
	node_map_t *map;
	if(!(map = (node_map_t *)malloc(sizeof(node_map_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_map_t));
		return NULL;
	}
	memset(map, 0, sizeof(node_map_t));
	map->element_list = element_list;
	
	node->value = map;
	return node;
}

node_t *
node_make_as(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_AS;
	
	node_as_t *as;
	if(!(as = (node_as_t *)malloc(sizeof(node_as_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_as_t));
		return NULL;
	}
	memset(as, 0, sizeof(node_as_t));
	as->left = left;
	as->right = right;
	
	node->value = as;
	return node;
}

node_t *
node_make_argument(list_t *value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ARGUMENT;
	
	node_argument_t *argument;
	if(!(argument = (node_argument_t *)malloc(sizeof(node_argument_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_argument_t));
		return NULL;
	}
	memset(argument, 0, sizeof(node_argument_t));
	argument->value = value;
	
	node->value = argument;
	return node;
}

node_t *
node_make_call(node_t *callable, node_t *tag, node_t *argument){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_CALL;
	
	node_call_t *call;
	if(!(call = (node_call_t *)malloc(sizeof(node_call_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_call_t));
		return NULL;
	}
	memset(call, 0, sizeof(node_call_t));
	call->callable = callable;
	call->tag = tag;
	call->argument = argument;
	
	node->value = call;
	return node;
}

node_t *
node_make_get_slice(node_t *name, node_t *start, node_t *stop, node_t *step){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GET_SLICE;
	
	node_get_slice_t *get_slice;
	if(!(get_slice = (node_get_slice_t *)malloc(sizeof(node_get_slice_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_get_slice_t));
		return NULL;
	}
	memset(get_slice, 0, sizeof(node_get_slice_t));
	get_slice->name = name;
	get_slice->start = start;
	get_slice->stop = stop;
	get_slice->step = step;
	
	node->value = get_slice;
	return node;
}

node_t *
node_make_get_item(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GET_SLICE;
	
	node_get_item_t *get_item;
	if(!(get_item = (node_get_item_t *)malloc(sizeof(node_get_item_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_get_item_t));
		return NULL;
	}
	memset(get_item, 0, sizeof(node_get_item_t));
	get_item->left = left;
	get_item->right = right;
	
	node->value = get_item;
	return node;
}

node_t *
node_make_get_attr(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GET_ATTR;
	
	node_get_attr_t *get_attr;
	if(!(get_attr = (node_get_attr_t *)malloc(sizeof(node_get_attr_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_get_attr_t));
		return NULL;
	}
	memset(get_attr, 0, sizeof(node_get_attr_t));
	get_attr->left = left;
	get_attr->right = right;
	
	node->value = get_attr;
	return node;
}

node_t *
node_make_tilde(node_t *left){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_TILDE;
	
	node_unary_t *unary;
	if(!(unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unary_t));
		return NULL;
	}
	memset(unary, 0, sizeof(node_unary_t));
	unary->left = left;
	
	node->value = unary;
	return node;
}

node_t *
node_make_not(node_t *left){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_NOT;
	
	node_unary_t *unary;
	if(!(unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unary_t));
		return NULL;
	}
	memset(unary, 0, sizeof(node_unary_t));
	unary->left = left;
	
	node->value = unary;
	return node;
}

node_t *
node_make_neg(node_t *left){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_NEG;
	
	node_unary_t *unary;
	if(!(unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unary_t));
		return NULL;
	}
	memset(unary, 0, sizeof(node_unary_t));
	unary->left = left;
	
	node->value = unary;
	return node;
}

node_t *
node_make_pos(node_t *left){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_POS;
	
	node_unary_t *unary;
	if(!(unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unary_t));
		return NULL;
	}
	memset(unary, 0, sizeof(node_unary_t));
	unary->left = left;
	
	node->value = unary;
	return node;
}

node_t *
node_make_get_address(node_t *left){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GET_ADDRESS;
	
	node_unary_t *unary;
	if(!(unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unary_t));
		return NULL;
	}
	memset(unary, 0, sizeof(node_unary_t));
	unary->left = left;
	
	node->value = unary;
	return node;
}

node_t *
node_make_get_value(node_t *left){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GET_VALUE;
	
	node_unary_t *unary;
	if(!(unary = (node_unary_t *)malloc(sizeof(node_unary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unary_t));
		return NULL;
	}
	memset(unary, 0, sizeof(node_unary_t));
	unary->left = left;
	
	node->value = unary;
	return node;
}

node_t *
node_make_mul(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MUL;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_div(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_DIV;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_mod(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MOD;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_plus(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_PLUS;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_minus(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MINUS;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_shl(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SHL;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_shr(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SHR;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_lt(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LT;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_le(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LE;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_gt(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GT;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_ge(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GE;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_eq(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_EQ;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_neq(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_NEQ;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_and(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_AND;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_xor(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_XOR;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_or(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_OR;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_land(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LAND;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_lor(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LOR;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_conditional(node_t *expr, node_t *true_expr, node_t *false_expr){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_CONDITIONAL;
	
	node_conditional_t *conditional;
	if(!(conditional = (node_conditional_t *)malloc(sizeof(node_conditional_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_conditional_t));
		return NULL;
	}
	memset(conditional, 0, sizeof(node_conditional_t));
	conditional->expr = expr;
	conditional->true_expr = true_expr;
	conditional->false_expr = false_expr;
	
	node->value = conditional;
	return node;
}

node_t *
node_make_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_add_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ADD_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_sub_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SUB_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_div_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_DIV_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_mul_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MUL_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_mod_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MOD_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_and_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_AND_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_or_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_OR_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_shl_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SHL_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_shr_assign(node_t *left, node_t *right){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SHR_ASSIGN;
	
	node_binary_t *binary;
	if(!(binary = (node_binary_t *)malloc(sizeof(node_binary_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_binary_t));
		return NULL;
	}
	memset(binary, 0, sizeof(node_binary_t));
	binary->left = left;
	binary->right = right;
	
	node->value = binary;
	return node;
}

node_t *
node_make_unpack(void *expr){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_UNPACK;
	
	node_unpack_t *unpack;
	if(!(unpack = (node_unpack_t *)malloc(sizeof(node_unpack_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_unpack_t));
		return NULL;
	}
	memset(unpack, 0, sizeof(node_unpack_t));
	unpack->expr = expr;
	
	node->value = unpack;
	return node;
}

node_t *
node_make_if(node_t *expr, node_t *then_block_stmt, node_t *else_block_stmt){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_IF;
	
	node_if_t *nif;
	if(!(nif = (node_if_t *)malloc(sizeof(node_if_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_if_t));
		return NULL;
	}
	memset(nif, 0, sizeof(node_if_t));
	nif->expr = expr;
	nif->then_block_stmt = then_block_stmt;
	nif->else_block_stmt = else_block_stmt;
	
	node->value = nif;
	return node;
}

node_t *
node_make_while(node_t *expr, node_t *block_stmt){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_WHILE;
	
	node_while_t *nwhile;
	if(!(nwhile = (node_while_t *)malloc(sizeof(node_while_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_while_t));
		return NULL;
	}
	memset(nwhile, 0, sizeof(node_while_t));
	nwhile->expr = expr;
	nwhile->block_stmt = block_stmt;
	
	node->value = nwhile;
	return node;
}

node_t *
node_make_break(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_BREAK;
	
	return node;
}

node_t *
node_make_continue(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_CONTINUE;
	
	return node;
}

node_t *
node_make_return(node_t *expr){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_RETURN;
	
	node_return_t *nreturn;
	if(!(nreturn = (node_return_t *)malloc(sizeof(node_return_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_return_t));
		return NULL;
	}
	memset(nreturn, 0, sizeof(node_return_t));
	nreturn->expr = expr;
	
	node->value = nreturn;
	return node;
}

node_t *
node_make_char(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_CHAR;

	return node;
}

node_t *
node_make_string(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_STRING;

	return node;
}

node_t *
node_make_int(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_INT;

	return node;
}

node_t *
node_make_long(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_LONG;

	return node;
}

node_t *
node_make_tag_list(list_t *list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_TAG_LIST;
	
	node_tag_list_t *tag_list;
	if(!(tag_list = (node_tag_list_t *)malloc(sizeof(node_tag_list_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_tag_list_t));
		return NULL;
	}
	memset(tag_list, 0, sizeof(node_tag_list_t));
	tag_list->list = list;
	
	node->value = tag_list;
	return node;
}

node_t *
node_make_tag(node_t *name, node_t *tag_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_TAG;
	
	node_tag_t *tag;
	if(!(tag = (node_tag_t *)malloc(sizeof(node_tag_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_tag_t));
		return NULL;
	}
	memset(tag, 0, sizeof(node_tag_t));
	tag->name = name;
	tag->tag_list = tag_list;
	
	node->value = tag;
	return node;
}

node_t *
node_make_pointer(node_t *item){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_POINTER;
	
	node_pointer_t *pointer;
	if(!(pointer = (node_pointer_t *)malloc(sizeof(node_pointer_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_pointer_t));
		return NULL;
	}
	memset(pointer, 0, sizeof(node_pointer_t));
	pointer->item = item;
	
	node->value = pointer;
	return node;
}

node_t *
node_make_array(node_t *item){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ARRAY;
	
	node_array_t *array;
	if(!(array = (node_array_t *)malloc(sizeof(node_array_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_array_t));
		return NULL;
	}
	memset(array, 0, sizeof(node_array_t));
	array->item = item;
	
	node->value = array;
	return node;
}

node_t *
node_make_float(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_FLOAT;

	return node;
}

node_t *
node_make_double(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_DOUBLE;

	return node;
}

node_t *
node_make_any(){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ANY;

	return node;
}

node_t *
node_make_tuple(list_t *type_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_TUPLE;
	
	node_tuple_t *tuple;
	if(!(tuple = (node_tuple_t *)malloc(sizeof(node_tuple_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_tuple_t));
		return NULL;
	}
	memset(tuple, 0, sizeof(node_tuple_t));
	tuple->type_list = type_list;
	
	node->value = tuple;
	return node;
}

node_t *
node_make_union(list_t *type_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_UNION;
	
	node_union_t *node_union;
	if(!(node_union = (node_union_t *)malloc(sizeof(node_union_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_union_t));
		return NULL;
	}
	memset(node_union, 0, sizeof(node_union_t));
	node_union->type_list = type_list;
	
	node->value = node_union;
	return node;
}

node_t *
node_make_var(node_t *name, node_t *label, node_t *expr){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_VAR;
	
	node_var_t *var;
	if(!(var = (node_var_t *)malloc(sizeof(node_var_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_var_t));
		return NULL;
	}
	memset(var, 0, sizeof(node_var_t));
	var->name = name;
	var->label = label;
	var->expr = expr;
	
	node->value = var;
	return node;
}

node_t *
node_make_directory(list_t *wise_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_DIRECTORY;
	
	node_directory_t *directory;
	if(!(directory = (node_directory_t *)malloc(sizeof(node_directory_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_directory_t));
		return NULL;
	}
	memset(directory, 0, sizeof(node_directory_t));
	directory->wise_list = wise_list;
	
	node->value = directory;
	return node;
}

node_t *
node_make_var_list(list_t *list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_VAR_LIST;
	
	node_var_list_t *var_list;
	if(!(var_list = (node_var_list_t *)malloc(sizeof(node_var_list_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_var_list_t));
		return NULL;
	}
	memset(var_list, 0, sizeof(node_var_list_t));
	var_list->list = list;
	
	node->value = var_list;
	return node;
}

node_t *
node_make_parameter(node_t *name, node_t *label, node_t *expr){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_PARAMETER;
	
	node_parameter_t *parameter;
	if(!(parameter = (node_parameter_t *)malloc(sizeof(node_parameter_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_parameter_t));
		return NULL;
	}
	memset(parameter, 0, sizeof(node_parameter_t));
	parameter->name = name;
	parameter->label = label;
	parameter->expr = expr;
	
	node->value = parameter;
	return node;
}

node_t *
node_make_parameter_list(list_t *list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_PARAMETER_LIST;
	
	node_parameter_list_t *parameter_list;
	if(!(parameter_list = (node_parameter_list_t *)malloc(sizeof(node_parameter_list_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_parameter_list_t));
		return NULL;
	}
	memset(parameter_list, 0, sizeof(node_parameter_list_t));
	parameter_list->list = list;
	
	node->value = parameter_list;
	return node;
}

node_t *
node_make_generic(node_t *name, node_t *label, node_t *default_value){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GENERIC;
	
	node_generic_t *generic;
	if(!(generic = (node_generic_t *)malloc(sizeof(node_generic_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_generic_t));
		return NULL;
	}
	memset(generic, 0, sizeof(node_generic_t));
	generic->name = name;
	generic->label = label;
	generic->default_value = default_value;
	
	node->value = generic;
	return node;
}

node_t *
node_make_generic_list(list_t *list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_GENERIC_LIST;
	
	node_generic_list_t *generic_list;
	if(!(generic_list = (node_generic_list_t *)malloc(sizeof(node_generic_list_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_generic_list_t));
		return NULL;
	}
	memset(generic_list, 0, sizeof(node_generic_list_t));
	generic_list->list = list;
	
	node->value = generic_list;
	return node;
}

node_t *
node_make_fn(node_t *name, node_t *generic_list, node_t *parameter_list, node_t *return_type, node_t *block_stmt){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_FN;
	
	node_fn_t *fn;
	if(!(fn = (node_fn_t *)malloc(sizeof(node_fn_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_fn_t));
		return NULL;
	}
	memset(fn, 0, sizeof(node_fn_t));
	fn->name = name;
	fn->generic_list = generic_list;
	fn->parameter_list = parameter_list;
	fn->return_type = return_type;
	fn->block_stmt = block_stmt;
	
	node->value = fn;
	return node;
}

node_t *
node_make_super_list(list_t *list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_SUPER_LIST;
	
	node_super_list_t *super_list;
	if(!(super_list = (node_super_list_t *)malloc(sizeof(node_super_list_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_super_list_t));
		return NULL;
	}
	memset(super_list, 0, sizeof(node_super_list_t));
	super_list->list = list;
	
	node->value = super_list;
	return node;
}

node_t *
node_make_class_block(list_t *list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_CLASS_BLOCK;
	
	node_class_block_t *class_block;
	if(!(class_block = (node_class_block_t *)malloc(sizeof(node_class_block_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_class_block_t));
		return NULL;
	}
	memset(class_block, 0, sizeof(node_class_block_t));
	class_block->list = list;
	
	node->value = class_block;
	return node;
}

node_t *
node_make_class(node_t *name, node_t *generic_list, node_t *super_list, node_t *block_stmt){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_CLASS;
	
	node_class_t *class;
	if(!(class = (node_class_t *)malloc(sizeof(node_class_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_class_t));
		return NULL;
	}
	memset(class, 0, sizeof(node_class_t));
	class->name = name;
	class->generic_list = generic_list;
	class->super_list = super_list;
	class->block_stmt = block_stmt;
	
	node->value = class;
	return node;
}

node_t *
node_make_enum_block(list_t *enum_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ENUM_BLOCK;
	
	node_enum_block_t *enum_block;
	if(!(enum_block = (node_enum_block_t *)malloc(sizeof(node_enum_block_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_enum_block_t));
		return NULL;
	}
	memset(enum_block, 0, sizeof(node_enum_block_t));
	enum_block->enum_list = enum_list;
	
	node->value = enum_block;
	return node;
}

node_t *
node_make_enum(node_t *name, node_t *block_stmt){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_ENUM;
	
	node_enum_t *enum_block;
	if(!(enum_block = (node_enum_t *)malloc(sizeof(node_enum_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_enum_t));
		return NULL;
	}
	memset(enum_block, 0, sizeof(node_enum_t));
	enum_block->name = name;
	enum_block->block_stmt = block_stmt;
	
	node->value = enum_block;
	return node;
}

node_t *
node_make_block(list_t *stmt_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_BLOCK;
	
	node_block_t *block;
	if(!(block = (node_block_t *)malloc(sizeof(node_block_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_block_t));
		return NULL;
	}
	memset(block, 0, sizeof(node_block_t));
	block->stmt_list = stmt_list;
	
	node->value = block;
	return node;
}

node_t *
node_make_import(node_t *block, node_t *from){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_IMPORT;
	
	node_import_t *import;
	if(!(import = (node_import_t *)malloc(sizeof(node_import_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_import_t));
		return NULL;
	}
	memset(import, 0, sizeof(node_import_t));
	import->block = block;
	import->from = from;
	
	node->value = import;
	return node;
}

node_t *
node_make_extern(node_t *block_stmt){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_EXTERN;
	
	node_extern_t *node_extern;
	if(!(node_extern = (node_extern_t *)malloc(sizeof(node_extern_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_extern_t));
		return NULL;
	}
	memset(node_extern, 0, sizeof(node_extern_t));
	node_extern->block_stmt = block_stmt;
	
	node->value = node_extern;
	return node;
}

node_t *
node_make_module(list_t *stmt_list){
	node_t *node;
	if(!(node = (node_t *)malloc(sizeof(node_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));
	node->kind = NODE_KIND_MODULE;
	
	node_module_t *module;
	if(!(module = (node_module_t *)malloc(sizeof(node_module_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(node_module_t));
		return NULL;
	}
	memset(module, 0, sizeof(node_module_t));
	module->stmt_list = stmt_list;
	
	node->value = module;
	return node;
}

uint64_t
node_get_position(node_t *node){
	return node->position;
}

uint64_t
node_get_line(node_t *node){
	return node->line;
}

uint64_t
node_get_column(node_t *node){
	return node->column;
}

char *
node_get_path(node_t *node){
	return node->path;
}
