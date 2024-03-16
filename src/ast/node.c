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

uint64_t node_counter = 0;

void
node_destroy(node_t *node)
{
  free (node);
}

node_t *
node_create(node_t *scope, node_t *parent, position_t position)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));
	if(node == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));

	node->id = node_counter++;
	node->position = position;
	node->parent = parent;
	node->scope = scope;
	node->attachments = list_create();
	node->flag = NODE_FLAG_NONE;

	return node;
}

static void
node_update(node_t *node, int32_t kind, void *value)
{
	node->value = value;
	node->kind = kind;
}

node_t *
node_clone(node_t *source)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));
	if(node == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));

	node->id = source->id;
	node->position = source->position;
	node->parent = source->parent;
	node->scope = source->scope;
	node->value = source->value;
	node->kind = source->kind;
	node->attachments = source->attachments;
	node->flag = NODE_FLAG_TEMPORARY;

	return node;
}

node_t *
node_make_id(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;

	node_update(node, NODE_KIND_ID, basic);
	return node;
}

node_t *
node_make_number(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;
	
	node_update(node, NODE_KIND_NUMBER, basic);
	return node;
}

node_t *
node_make_char(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;
	
	node_update(node, NODE_KIND_CHAR, basic);
	return node;
}

node_t *
node_make_string(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;
	
	node_update(node, NODE_KIND_STRING, basic);
	return node;
}

node_t *
node_make_null(node_t *node)
{
	node_update(node, NODE_KIND_NULL, NULL);
	return node;
}

node_t *
node_make_true(node_t *node)
{
	node_update(node, NODE_KIND_TRUE, NULL);
	return node;
}

node_t *
node_make_false(node_t *node)
{
	node_update(node, NODE_KIND_FALSE, NULL);
	return node;
}

node_t *
node_make_infinity(node_t *node)
{
	node_update(node, NODE_KIND_INFINITY, NULL);
	return node;
}

node_t *
node_make_this(node_t *node){
	node_update(node, NODE_KIND_THIS, NULL);
	return node;
}

node_t *
node_make_array(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_ARRAY, basic);
	return node;
}

node_t *
node_make_object(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_OBJECT, basic);
	return node;
}

node_t *
node_make_composite(node_t *node, node_t *base, node_t *arguments)
{
	node_carrier_t *basic = (node_carrier_t *)malloc(sizeof(node_carrier_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_carrier_t));
	basic->base = base;
	basic->arguments = arguments;
	
	node_update(node, NODE_KIND_COMPOSITE, basic);
	return node;
}






node_t *
node_make_in(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_IN, basic);
	return node;
}

node_t *
node_make_typeof(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_TYPEOF, basic);
	return node;
}

node_t *
node_make_sizeof(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_SIZEOF, basic);
	return node;
}

node_t *
node_make_parenthesis(node_t *node, node_t *value)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = value;
	
	node_update(node, NODE_KIND_PARENTHESIS, basic);
	return node;
}

node_t *
node_make_call(node_t *node, node_t *base, node_t *arguments)
{
	node_carrier_t *basic = (node_carrier_t *)malloc(sizeof(node_carrier_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_carrier_t));
	basic->base = base;
	basic->arguments = arguments;
	
	node_update(node, NODE_KIND_CALL, basic);
	return node;
}

node_t *
node_make_item(node_t *node, node_t *base, node_t *arguments)
{
	node_carrier_t *basic = (node_carrier_t *)malloc(sizeof(node_carrier_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_carrier_t));
	basic->base = base;
	basic->arguments = arguments;
	
	node_update(node, NODE_KIND_GET_ITEM, basic);
	return node;
}

node_t *
node_make_attribute(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_ATTRIBUTE, basic);
	return node;
}

node_t *
node_make_tilde(node_t *node, node_t *right)
{
	node_unary_t *basic;
	if(!(basic = (node_unary_t *)malloc(sizeof(node_unary_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_TILDE, basic);
	return node;
}

node_t *
node_make_not(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_NOT, basic);
	return node;
}

node_t *
node_make_neg(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_NEG, basic);
	return node;
}

node_t *
node_make_pos(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_POS, basic);
	return node;
}

node_t *
node_make_ellipsis(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_ELLIPSIS, basic);
	return node;
}



node_t *
node_make_pow(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_POW, basic);
	return node;
}

node_t *
node_make_epi(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_EPI, basic);
	return node;
}

node_t *
node_make_mul(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MUL, basic);
	return node;
}

node_t *
node_make_div(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_DIV, basic);
	return node;
}

node_t *
node_make_mod(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MOD, basic);
	return node;
}

node_t *
node_make_plus(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_PLUS, basic);
	return node;
}

node_t *
node_make_minus(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MINUS, basic);
	return node;
}

node_t *
node_make_shl(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHL, basic);
	return node;
}

node_t *
node_make_shr(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHR, basic);
	return node;
}

node_t *
node_make_lt(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LT, basic);
	return node;
}

node_t *
node_make_le(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LE, basic);
	return node;
}

node_t *
node_make_gt(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_GT, basic);
	return node;
}

node_t *
node_make_ge(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_GE, basic);
	return node;
}

node_t *
node_make_eq(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_EQ, basic);
	return node;
}

node_t *
node_make_neq(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_NEQ, basic);
	return node;
}

node_t *
node_make_and(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_AND, basic);
	return node;
}

node_t *
node_make_xor(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_XOR, basic);
	return node;
}

node_t *
node_make_or(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_OR, basic);
	return node;
}

node_t *
node_make_land(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LAND, basic);
	return node;
}

node_t *
node_make_lor(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LOR, basic);
	return node;
}

node_t *
node_make_conditional(node_t *node, node_t *condition, node_t *true_expression, node_t *false_expression)
{
	node_triple_t *node_triple;
	if(!(node_triple = (node_triple_t *)malloc(sizeof(node_triple_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_triple_t));
		return NULL;
	}
	memset(node_triple, 0, sizeof(node_triple_t));
	node_triple->base = condition;
	node_triple->left = true_expression;
	node_triple->right = false_expression;
	
	node_update(node, NODE_KIND_CONDITIONAL, node_triple);
	return node;
}


node_t *
node_make_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_ASSIGN, basic);
	return node;
}

node_t *
node_make_define(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_DEFINE, basic);
	return node;
}

node_t *
node_make_add_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_ADD_ASSIGN, basic);
	return node;
}

node_t *
node_make_sub_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SUB_ASSIGN, basic);
	return node;
}

node_t *
node_make_div_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_DIV_ASSIGN, basic);
	return node;
}

node_t *
node_make_mul_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MUL_ASSIGN, basic);
	return node;
}

node_t *
node_make_mod_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MOD_ASSIGN, basic);
	return node;
}

node_t *
node_make_and_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_AND_ASSIGN, basic);
	return node;
}

node_t *
node_make_or_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_OR_ASSIGN, basic);
	return node;
}

node_t *
node_make_shl_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHL_ASSIGN, basic);
	return node;
}

node_t *
node_make_shr_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHR_ASSIGN, basic);
	return node;
}



node_t *
node_make_if(node_t *node, node_t *key, node_t *condition, node_t *then_body, node_t *else_body)
{
	node_if_t *basic;
	if(!(basic = (node_if_t *)malloc(sizeof(node_if_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_if_t));
		return NULL;
	}

	memset(basic, 0, sizeof(node_if_t));
	
	basic->key = key;
	basic->condition = condition;
	basic->then_body = then_body;
	basic->else_body = else_body;
	
	node_update(node, NODE_KIND_IF, basic);
	return node;
}

node_t *
node_make_for(node_t *node, uint64_t flag, node_t *key, node_t *initializer, node_t *condition, node_t *incrementor, node_t *body)
{
	node_for_t *basic;
	if(!(basic = (node_for_t *)malloc(sizeof(node_for_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_for_t));
		return NULL;
	}
	
	memset(basic, 0, sizeof(node_for_t));

	basic->flag = flag;
	basic->key = key;
	basic->initializer = initializer;
	basic->condition = condition;
	basic->incrementor = incrementor;
	basic->body = body;
	
	node_update(node, NODE_KIND_FOR, basic);
	return node;
}

node_t *
node_make_forin(node_t *node, uint64_t flag, node_t *key, node_t *initializer, node_t *expression, node_t *body)
{
	node_forin_t *basic;
	if(!(basic = (node_forin_t *)malloc(sizeof(node_forin_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_forin_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_forin_t));
	
	basic->flag = flag;
	basic->key = key;
	basic->initializer = initializer;
	basic->expression = expression;
	basic->body = body;
	
	node_update(node, NODE_KIND_FORIN, basic);
	return node;
}

node_t *
node_make_break(node_t *node, node_t *expression)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = expression;

	node_update(node, NODE_KIND_BREAK, basic);
	return node;
}

node_t *
node_make_continue(node_t *node, node_t *expression)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = expression;

	node_update(node, NODE_KIND_CONTINUE, basic);
	return node;
}

node_t *
node_make_catch(node_t *node, node_t *parameters, node_t *body)
{
	node_catch_t *basic;
	if(!(basic = (node_catch_t *)malloc(sizeof(node_catch_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_catch_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_catch_t));
	basic->parameters = parameters;
	basic->body = body;
	
	node_update(node, NODE_KIND_CATCH, basic);
	return node;
}

node_t *
node_make_catchs(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_CATCHS, basic);
	return node;
}

node_t *
node_make_try(node_t *node, node_t *body, node_t *catchs)
{
	node_try_t *basic;
	if(!(basic = (node_try_t *)malloc(sizeof(node_try_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_try_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_try_t));
	basic->body = body;
	basic->catchs = catchs;
	
	node_update(node, NODE_KIND_TRY, basic);
	return node;
}

node_t *
node_make_return(node_t *node, node_t *expression)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = expression;
	
	node_update(node, NODE_KIND_RETURN, basic);
	return node;
}

node_t *
node_make_throw(node_t *node, node_t *arguments)
{
	node_throw_t *basic;
	if(!(basic = (node_throw_t *)malloc(sizeof(node_throw_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_throw_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_throw_t));
	basic->arguments = arguments;
	
	node_update(node, NODE_KIND_THROW, basic);
	return node;
}

node_t *
node_make_var(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value){
	node_var_t *basic;
	if(!(basic = (node_var_t *)malloc(sizeof(node_var_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_var_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_var_t));
	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_VAR, basic);
	return node;
}

node_t *
node_make_argument(node_t *node, node_t *key, node_t *value)
{
	node_argument_t *basic = (node_argument_t *)malloc(sizeof(node_argument_t));
	if(basic == NULL){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_argument_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_argument_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_ARGUMENT, basic);
	return node;
}

node_t *
node_make_arguments(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_ARGUMENTS, basic);
	return node;
}

node_t *
node_make_datatype(node_t *node, node_t *key, node_t *value)
{
	node_datatype_t *basic = (node_datatype_t *)malloc(sizeof(node_datatype_t));
	if(basic == NULL){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_datatype_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_datatype_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_DATATYPE, basic);
	return node;
}

node_t *
node_make_datatypes(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_DATATYPES, basic);
	return node;
}

node_t *
node_make_parameter(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value){
	node_parameter_t *basic;
	if(!(basic = (node_parameter_t *)malloc(sizeof(node_parameter_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_parameter_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_parameter_t));
	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_PARAMETER, basic);
	return node;
}

node_t *
node_make_parameters(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_PARAMETERS, basic);
	return node;
}

node_t *
node_make_field(node_t *node, node_t *key, node_t *type)
{
	node_field_t *basic;
	if(!(basic = (node_field_t *)malloc(sizeof(node_field_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_field_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_field_t));
	basic->key = key;
	basic->type = type;
	
	node_update(node, NODE_KIND_FIELD, basic);
	return node;
}

node_t *
node_make_fields(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_FIELDS, basic);
	return node;
}

node_t *
node_make_generic(node_t *node, node_t *key, node_t *type, node_t *value)
{
	node_generic_t *basic;
	if(!(basic = (node_generic_t *)malloc(sizeof(node_generic_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_generic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_generic_t));
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_GENERIC, basic);
	return node;
}

node_t *
node_make_generics(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_GENERICS, basic);
	return node;
}

node_t *
node_make_func(node_t *node, uint64_t flag, node_t *key, node_t *generics, node_t *parameters, node_t *result, node_t *body)
{
	node_func_t *basic;
	if(!(basic = (node_func_t *)malloc(sizeof(node_func_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_func_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_func_t));
	basic->flag = flag;
	basic->key = key;
	basic->generics = generics;
	basic->parameters = parameters;
	basic->result = result;
	basic->body = body;
	
	node_update(node, NODE_KIND_FUNC, basic);
	return node;
}

node_t *
node_make_lambda(node_t *node, node_t *parameters, node_t *body)
{
	node_lambda_t *basic;
	if(!(basic = (node_lambda_t *)malloc(sizeof(node_lambda_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_lambda_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_lambda_t));
	basic->parameters = parameters;
	basic->body = body;
	
	node_update(node, NODE_KIND_LAMBDA, basic);
	return node;
}

node_t *
node_make_property(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value)
{
	node_property_t *basic;
	if(!(basic = (node_property_t *)malloc(sizeof(node_property_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_property_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_property_t));
	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_PROPERTY, basic);
	return node;
}

node_t *
node_make_properties(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_PROPERTIES, basic);
	return node;
}

node_t *
node_make_heritage(node_t *node, node_t *key, node_t *type)
{
	node_heritage_t *basic;
	if(!(basic = (node_heritage_t *)malloc(sizeof(node_heritage_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_heritage_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_heritage_t));
	basic->key = key;
	basic->type = type;
	
	node_update(node, NODE_KIND_HERITAGE, basic);
	return node;
}

node_t *
node_make_heritages(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_HERITAGES, basic);
	return node;
}

node_t *
node_make_class(node_t *node, uint64_t flag, node_t *key, node_t *generics, node_t *heritages, node_t *block)
{
	node_class_t *basic;
	if(!(basic = (node_class_t *)malloc(sizeof(node_class_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_class_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_class_t));
	basic->flag = flag;
	basic->key = key;
	basic->generics = generics;
	basic->heritages = heritages;
	basic->block = block;
	
	node_update(node, NODE_KIND_CLASS, basic);
	return node;
}

node_t *
node_make_member(node_t *node, node_t *key, node_t *value){
	node_member_t *basic;
	if(!(basic = (node_member_t *)malloc(sizeof(node_member_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_member_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_member_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_MEMBER, basic);
	return node;
}

node_t *
node_make_members(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_MEMBERS, basic);
	return node;
}

node_t *
node_make_enum(node_t *node, uint64_t flag, node_t *key, node_t *members)
{
	node_enum_t *basic;
	if(!(basic = (node_enum_t *)malloc(sizeof(node_enum_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_enum_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_enum_t));
	basic->flag = flag;
	basic->key = key;
	basic->members = members;
	
	node_update(node, NODE_KIND_ENUM, basic);
	return node;
}

node_t *
node_make_block(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_BLOCK, basic);
	return node;
}

node_t *
node_make_body(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_BODY, basic);
	return node;
}

node_t *
node_make_import(node_t *node, node_t *path, node_t *fields)
{
	node_import_t *basic;
	if(!(basic = (node_import_t *)malloc(sizeof(node_import_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_import_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_import_t));
	basic->path = path;
	basic->fields = fields;
	
	node_update(node, NODE_KIND_IMPORT, basic);
	return node;
}

node_t *
node_make_module(node_t *node, char *path, node_t *block)
{
	node_module_t *basic = (node_module_t *)malloc(sizeof(node_module_t));
	if(basic == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(node_module_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_module_t));
	basic->block = block;
	basic->path = path;
	
	node_update(node, NODE_KIND_MODULE, basic);
	return node;
}
