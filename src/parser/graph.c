#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "parser.h"
#include "error.h"
#include "syntax.h"
#include "symbol.h"
#include "graph.h"

#define max(a, b) a > b ? a : b

static int32_t
graph_expression(symbol_t *parent, node_t *node);

static int32_t
graph_expression(symbol_t *parent, node_t *node);

static int32_t
graph_func(symbol_t *parent, node_t *node);

static int32_t
graph_parameter(symbol_t *parent, node_t *node);

static int32_t
graph_type_parameter(symbol_t *parent, node_t *node);

static int32_t
graph_postfix(symbol_t *parent, node_t *node);

static int32_t
graph_block(symbol_t *parent, node_t *node);

static int32_t
graph_const(symbol_t *parent, node_t *node);

static int32_t
graph_var(symbol_t *parent, node_t *node);

static int32_t
graph_type(symbol_t *parent, node_t *node);

static int32_t
graph_prefix(symbol_t *parent, node_t *node);

static int32_t
graph_export(symbol_t *parent, node_t *node);



static int32_t
graph_id(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_ID, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_number(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_NUMBER, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_string(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_STRING, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_char(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_CHAR, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_null(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_FLAG_NULL, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_true(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_FLAG_TRUE, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_false(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FALSE, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_array(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_FLAG_ARRAY, node);
	if(!symbol)
	{
		return 0;
	}

	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = graph_expression(symbol, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
graph_parenthesis(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_PARENTHESIS, node);
	if(!symbol)
	{
		return 0;
	}

	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	int32_t result;
	result = graph_expression(symbol, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_object_property(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_name;
	symbol_t *symbol_value;
	
	symbol = symbol_rpush(parent, SYMBOL_FLAG_OBJECT_PROPERTY, node);
	if(!symbol)
	{
		return 0;
	}

	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	if(node_object_property->name)
	{
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_object_property->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_expression(symbol_name, node_object_property->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_object_property->value)
	{
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_VALUE, node_object_property->value);
		if(!symbol_value)
		{
			return 0;
		}

		result = graph_expression(symbol_value, node_object_property->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_object(symbol_t *parent, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_OBJECT, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = graph_object_property(symbol, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_primary(symbol_t *parent, node_t *node)
{
	int32_t result = 1;
	switch (node->kind)
	{
	case NODE_KIND_ID:
		result = graph_id(parent, node);
		break;

	case NODE_KIND_NUMBER:
		result = graph_number(parent, node);
		break;

	case NODE_KIND_STRING:
		result = graph_string(parent, node);
		break;

	case NODE_KIND_CHAR:
		result = graph_char(parent, node);
		break;

	case NODE_KIND_NULL:
		result = graph_null(parent, node);
		break;

	case NODE_KIND_TRUE:
		result = graph_true(parent, node);
		break;

	case NODE_KIND_FALSE:
		result = graph_false(parent, node);
		break;

	case NODE_KIND_ARRAY:
		result = graph_array(parent, node);
		break;

	case NODE_KIND_OBJECT:
		result = graph_object(parent, node);
		break;

	case NODE_KIND_FUNC:
		result = graph_func(parent, node);
		break;

	case NODE_KIND_PARENTHESIS:
		result = graph_parenthesis(parent, node);
		break;

	default:
		return 0;
	}

	return result;
}

static int32_t
graph_composite(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_base;
	symbol_t *symbol_type_argument;

	symbol = symbol_rpush(parent, NODE_KIND_COMPOSITE, node);
	if(!symbol)
	{
		return 0;
	}

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	int32_t result;
	if(node_composite->base)
	{
		symbol_base = symbol_rpush(symbol, SYMBOL_FLAG_BASE, node);
		if(!symbol_base)
		{
			return 0;
		}

		result = graph_postfix(symbol_base, node_composite->base);
		if (!result)
		{
			return 0;
		}
	}

	if (node_composite->type_arguments)
	{
		symbol_type_argument = symbol_rpush(symbol, SYMBOL_FLAG_TYPE_ARGUMENT, node);
		if(!symbol_type_argument)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_composite->type_arguments->begin; a != node_composite->type_arguments->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_expression(symbol_type_argument, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
graph_call(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_callable;
	symbol_t *symbol_argument;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_CALL, node);
	if(!symbol)
	{
		return 0;
	}

	node_call_t *node_call;
	node_call = (node_call_t *)node->value;

	symbol_callable = symbol_rpush(symbol, SYMBOL_FLAG_CALLABLE, node);
	if(!symbol_callable)
	{
		return 0;
	}

	int32_t result;
	result = graph_postfix(symbol_callable, node_call->callable);
	if (!result)
	{
		return 0;
	}

	if (node_call->arguments)
	{
		symbol_argument = symbol_rpush(symbol, SYMBOL_FLAG_TYPE_ARGUMENT, node);
		if(!symbol_argument)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_call->arguments->begin; a != node_call->arguments->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_expression(symbol_argument, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
graph_get_slice(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_name;
	symbol_t *symbol_start;
	symbol_t *symbol_step;
	symbol_t *symbol_stop;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_GET_SLICE, node);
	if(!symbol)
	{
		return 0;
	}

	node_get_slice_t *node_get_slice;
	node_get_slice = (node_get_slice_t *)node->value;

	symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_get_slice->name);
	if(!symbol_name)
	{
		return 0;
	}

	int32_t result;
	result = graph_postfix(symbol_name, node_get_slice->name);
	if (!result)
	{
		return 0;
	}

	if (node_get_slice->start)
	{
		symbol_start = symbol_rpush(symbol, SYMBOL_FLAG_GET_START, node_get_slice->start);
		if(!symbol_start)
		{
			return 0;
		}

		result = graph_expression(symbol_start, node_get_slice->start);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_slice->step)
	{
		symbol_step = symbol_rpush(symbol, SYMBOL_FLAG_GET_STEP, node_get_slice->step);
		if(!symbol_step)
		{
			return 0;
		}

		result = graph_expression(symbol_step, node_get_slice->step);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_slice->stop)
	{
		symbol_stop = symbol_rpush(symbol, SYMBOL_FLAG_GET_STOP, node_get_slice->stop);
		if(!symbol_stop)
		{
			return 0;
		}

		result = graph_expression(symbol_stop, node_get_slice->stop);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_get_item(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_name;
	symbol_t *symbol_index;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_GET_ITEM, node);
	if(!symbol)
	{
		return 0;
	}

	node_get_item_t *node_get_item;
	node_get_item = (node_get_item_t *)node->value;

	int32_t result;
	if(node_get_item->name)
	{
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_get_item->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_postfix(symbol_name, node_get_item->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_item->index)
	{
		symbol_index = symbol_rpush(symbol, SYMBOL_FLAG_INDEX, node_get_item->index);
		if(!symbol_index)
		{
			return 0;
		}

		result = graph_expression(symbol_index, node_get_item->index);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
graph_get_attr(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	symbol = symbol_rpush(parent, SYMBOL_FLAG_ATTR, node);
	if(!symbol)
	{
		return 0;
	}

	node_get_attr_t *node_get_attr;
	node_get_attr = (node_get_attr_t *)node->value;
	
	int32_t result;
	if(node_get_attr->left)
	{
		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_get_attr->left);
		if(!symbol_left)
		{
			return 0;
		}
		
		result = graph_postfix(symbol_left, node_get_attr->left);
		if (!result)
		{
			return 0;
		}
	}

	if(node_get_attr->right)
	{
		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_get_attr->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_id(symbol_right, node_get_attr->right);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_postfix(symbol_t *parent, node_t *node)
{
	int32_t result;
	switch (node->kind)
	{
	case NODE_KIND_COMPOSITE:
		result = graph_composite(parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_CALL:
		result = graph_call(parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_SLICE:
		result = graph_get_slice(parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_ITEM:
		result = graph_get_item(parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_ATTR:
		result = graph_get_attr(parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_primary(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}
	return 1;
}

static int32_t
graph_prefix(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_TILDE)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_TILDE, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_NOT)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_NOT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_NEG)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_NEG, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_POS)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_POS, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_GET_VALUE)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_GET_VALUE, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_GET_ADDRESS)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_GET_ADDRESS, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_AWAIT)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_AWAIT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_SIZEOF)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_SIZEOF, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_TYPEOF)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_TYPEOF, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_ELLIPSIS)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_ELLIPSIS, node);
		if(!symbol)
		{
			return 0;
		}
	}

	int32_t result;
	node_unary_t *node_unary;
	switch (node->kind)
	{
	case NODE_KIND_TILDE:
	case NODE_KIND_NOT:
	case NODE_KIND_NEG:
	case NODE_KIND_POS:
	case NODE_KIND_GET_VALUE:
	case NODE_KIND_GET_ADDRESS:
	case NODE_KIND_AWAIT:
	case NODE_KIND_SIZEOF:
	case NODE_KIND_TYPEOF:
	case NODE_KIND_ELLIPSIS:
		node_unary = (node_unary_t *)node->value;

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_unary->right);
		if(!symbol_right)
		{
			return 0;
		}

		if (node_unary->right->kind == NODE_KIND_TILDE)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_NOT)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_POS)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_NEG)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_GET_VALUE)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_GET_ADDRESS)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_AWAIT)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_SIZEOF)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_TYPEOF)
		{
			result = graph_prefix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_postfix(symbol_right, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		break;

	default:
		result = graph_postfix(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_multiplicative(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_MUL)
	{
		symbol = symbol_rpush(parent, NODE_KIND_MUL, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_DIV)
	{
		symbol = symbol_rpush(parent, NODE_KIND_DIV, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_MOD)
	{
		symbol = symbol_rpush(parent, NODE_KIND_MOD, node);
		if(!symbol)
		{
			return 0;
		}
	}

	int32_t result;
	node_binary_t *node_binary;

	switch (node->kind)
	{
	case NODE_KIND_MUL:
	case NODE_KIND_DIV:
	case NODE_KIND_MOD:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_MUL)
		{
			result = graph_multiplicative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_DIV)
		{
			result = graph_multiplicative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_MOD)
		{
			result = graph_multiplicative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_prefix(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_prefix(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_prefix(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_addative(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_PLUS)
	{
		symbol = symbol_rpush(parent, NODE_KIND_SHL, node);
		if(!symbol)
		{
			return 0;
		}
	}
	if(node->kind == NODE_KIND_MINUS)
	{
		symbol = symbol_rpush(parent, NODE_KIND_SHL, node);
		if(!symbol)
		{
			return 0;
		}
	}

	int32_t result;
	node_binary_t *node_binary;

	switch (node->kind)
	{
	case NODE_KIND_PLUS:
	case NODE_KIND_MINUS:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_PLUS)
		{
			result = graph_addative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_MINUS)
		{
			result = graph_addative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_multiplicative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_multiplicative(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_multiplicative(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_shifting(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_SHL)
	{
		symbol = symbol_rpush(parent, NODE_KIND_SHL, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_SHR)
	{
		symbol = symbol_rpush(parent, NODE_KIND_SHR, node);
		if(!symbol)
		{
			return 0;
		}
	}

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_SHL:
	case NODE_KIND_SHR:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_SHL)
		{
			result = graph_shifting(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_SHR)
		{
			result = graph_shifting(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_addative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_addative(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_addative(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_relational(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_LT)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_LT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_GT)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_GT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_LE)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_LE, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_GE)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_GE, node);
		if(!symbol)
		{
			return 0;
		}
	}

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_LT:
	case NODE_KIND_GT:
	case NODE_KIND_LE:
	case NODE_KIND_GE:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_LT)
		{
			result = graph_relational(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_LE)
		{
			result = graph_relational(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_GT)
		{
			result = graph_relational(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_GE)
		{
			result = graph_relational(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_shifting(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_shifting(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_shifting(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_equality(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;

	if(node->kind == NODE_KIND_EQ)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_EQ, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_IN)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_IN, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_EXTENDS)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_EXTENDS, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_NEQ)
	{
		symbol = symbol_rpush(parent, SYMBOL_FLAG_NEQ, node);
		if(!symbol)
		{
			return 0;
		}
	}

	switch (node->kind)
	{
	case NODE_KIND_EQ:
	case NODE_KIND_IN:
	case NODE_KIND_EXTENDS:
	case NODE_KIND_NEQ:
		node_binary = (node_binary_t *)node->value;
		
		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_IN)
		{
			result = graph_equality(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_EQ)
		{
			result = graph_equality(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_NEQ)
		{
			result = graph_equality(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_relational(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_relational(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_relational(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_bitwise_and(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_AND:
		symbol = symbol_rpush(parent, SYMBOL_FLAG_AND, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_AND)
		{
			result = graph_bitwise_and(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_equality(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_equality(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_equality(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_bitwise_xor(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_XOR:
		symbol = symbol_rpush(parent, SYMBOL_FLAG_XOR, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_XOR)
		{
			result = graph_bitwise_xor(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_bitwise_and(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_bitwise_and(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_bitwise_and(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_bitwise_or(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_OR:
		symbol = symbol_rpush(parent, SYMBOL_FLAG_OR, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_OR)
		{
			result = graph_bitwise_or(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_bitwise_xor(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_bitwise_xor(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_bitwise_xor(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_logical_and(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_LAND:
		symbol = symbol_rpush(parent, SYMBOL_FLAG_LAND, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_LAND)
		{
			result = graph_logical_and(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_bitwise_or(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_bitwise_or(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_bitwise_or(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_logical_or(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_LOR:
		symbol = symbol_rpush(parent, SYMBOL_FLAG_LOR, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_LOR)
		{
			result = graph_logical_or(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_logical_and(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_logical_and(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_logical_and(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_conditional(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_condition;
	symbol_t *symbol_true;
	symbol_t *symbol_false;

	int32_t result;
	node_conditional_t *node_conditional;
	switch (node->kind)
	{
	case NODE_KIND_CONDITIONAL:
		node_conditional = (node_conditional_t *)node->value;

		symbol = symbol_rpush(parent, SYMBOL_FLAG_CONDITIONAL, node);
		if(!symbol)
		{
			return 0;
		}

		symbol_condition = symbol_rpush(symbol, SYMBOL_FLAG_CONDITION, node);
		if(!symbol_condition)
		{
			return 0;
		}

		result = graph_logical_or(symbol_condition, node_conditional->condition);
		if (!result)
		{
			return 0;
		}

		symbol_true = symbol_rpush(symbol, SYMBOL_FLAG_TRUE, node);
		if(!symbol_true)
		{
			return 0;
		}
		result = graph_conditional(symbol_true, node_conditional->true_expression);
		if (!result)
		{
			return 0;
		}

		symbol_false = symbol_rpush(symbol, SYMBOL_FLAG_FALSE, node);
		if(!symbol_false)
		{
			return 0;
		}
		result = graph_conditional(symbol_false, node_conditional->false_expression);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_logical_or(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_expression(symbol_t *parent, node_t *node)
{
	return graph_conditional(parent, node);
}

static int32_t
graph_assign(symbol_t *parent, node_t *node)
{
	node_binary_t *node_binary;

	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	switch (node->kind)
	{
	case NODE_KIND_ASSIGN:
	case NODE_KIND_DEFINE:
	case NODE_KIND_ADD_ASSIGN:
	case NODE_KIND_SUB_ASSIGN:
	case NODE_KIND_DIV_ASSIGN:
	case NODE_KIND_MUL_ASSIGN:
	case NODE_KIND_MOD_ASSIGN:
	case NODE_KIND_AND_ASSIGN:
	case NODE_KIND_OR_ASSIGN:
	case NODE_KIND_SHL_ASSIGN:
	case NODE_KIND_SHR_ASSIGN:
		node_binary = (node_binary_t *)node->value;
		
		symbol = symbol_rpush(parent, SYMBOL_FLAG_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}

		symbol_left = symbol_rpush(symbol, SYMBOL_FLAG_ASSIGN_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		int32_t result;
		result = graph_expression(symbol_left, node_binary->left);
		if (!result)
		{
			return 0;
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_FLAG_ASSIGN_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_expression(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_expression(parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
graph_if(symbol_t *parent, node_t *node)
{
	node_if_t *node_if;
	node_if = (node_if_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_IF, node);
	if (!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_if->condition)
	{
		symbol_t *symbol_condition;
		symbol_condition = symbol_rpush(symbol, SYMBOL_FLAG_IF_CONDITION, node_if->condition);
		if(!symbol_condition)
		{
			return 0;
		}
		
		result = graph_expression(symbol_condition, node_if->condition);
		if (!result)
		{
			return 0;
		}
	}

	if (node_if->then_body)
	{
		result = graph_block(symbol, node_if->then_body);
		if (!result)
		{
			return 0;
		}
	}

	if (node_if->else_body)
	{
		symbol_t *symbol_else;
		symbol_else = symbol_rpush(symbol, SYMBOL_FLAG_ELSE, node_if->else_body);
		if(!symbol_else)
		{
			return 0;
		}

		if(node_if->else_body->kind == NODE_KIND_IF)
		{
			result = graph_if(symbol_else, node_if->else_body);
		} 
		else 
		{
			result = graph_block(symbol_else, node_if->else_body);
		}

		if (!result) {
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_catch(symbol_t *parent, node_t *node)
{
	node_catch_t *node_catch;
	node_catch = (node_catch_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_TRY, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_catch->parameter)
	{
		result = graph_parameter(symbol, node_catch->parameter);
		if (!result)
		{
			return 0;
		}
	}

	if (node_catch->body)
	{
		result = graph_block(symbol, node_catch->body);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_try(symbol_t *parent, node_t *node)
{
	node_try_t *node_try;
	node_try = (node_try_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_TRY, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	result = graph_block(symbol, node_try->body);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_try->catchs->begin; a != node_try->catchs->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		result = graph_catch(symbol, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_for_init(symbol_t *parent, node_t *node)
{
	node_enumerable_t *node_enumerable;
	node_enumerable = (node_enumerable_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FOR_INIT, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	ilist_t *a;
	for (a = node_enumerable->list->begin; a != node_enumerable->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		switch (temp->kind)
		{
		case NODE_KIND_VAR:
			result = graph_var(symbol, temp);
			break;

		case NODE_KIND_CONST:
			result = graph_const(symbol, temp);
			break;

		default:
			result = graph_assign(symbol, temp);
			break;
		}

		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_for_step(symbol_t *parent, node_t *node)
{
	node_enumerable_t *node_enumerable;
	node_enumerable = (node_enumerable_t *)node->value;
	
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FOR_STEP, node);
	if(!symbol)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_enumerable->list->begin; a != node_enumerable->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result = graph_assign(symbol, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_for(symbol_t *parent, node_t *node)
{
	node_for_t *node_for;
	node_for = (node_for_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FOR, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_for->initializer)
	{
		symbol_t *symbol_initializer;
		symbol_initializer = symbol_rpush(symbol, SYMBOL_FLAG_FOR_INITIALIZER, node_for->initializer);
		if(!symbol_initializer)
		{
			return 0;
		}

		switch (node_for->initializer->kind)
		{
		case NODE_KIND_FOR_INIT_LIST:
			result = graph_for_init(symbol_initializer, node_for->initializer);
			break;

		case NODE_KIND_VAR:
			result = graph_var(symbol_initializer, node_for->initializer);
			break;

		case NODE_KIND_CONST:
			result = graph_const(symbol_initializer, node_for->initializer);
			break;

		default:
			result = graph_assign(symbol_initializer, node_for->initializer);
			break;
		}
	}

	if (node_for->condition)
	{
		symbol_t *symbol_condition;
		symbol_condition = symbol_rpush(symbol, SYMBOL_FLAG_FOR_CONDITION, node);
		if(!symbol_condition)
		{
			return 0;
		}

		result = graph_expression(symbol_condition, node_for->condition);
		if (!result)
		{
			return 0;
		}
	}

	if (node_for->incrementor)
	{
		symbol_t *symbol_incrementor;
		symbol_incrementor = symbol_rpush(symbol, SYMBOL_FLAG_FOR_INCREMENTOR, node);
		if(!symbol_incrementor)
		{
			return 0;
		}

		if (node_for->incrementor->kind == NODE_KIND_FOR_STEP_LIST)
		{
			result = graph_for_step(symbol_incrementor, node_for->incrementor);
		}
		else
		{
			result = graph_assign(symbol_incrementor, node_for->incrementor);
		}
		if (!result)
		{
			return 0;
		}
	}
	
	if (node_for->body)
	{
		result = graph_block(symbol, node_for->body);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_forin(symbol_t *parent, node_t *node)
{
	node_forin_t *node_forin;
	node_forin = (node_forin_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FORIN, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = 0;
	if (node_forin->initializer)
	{
		symbol_t *symbol_initializer;
		symbol_initializer = symbol_rpush(symbol, SYMBOL_FLAG_FORIN_INITIALIZER, node);
		if(!symbol_initializer)
		{
			return 0;
		}
		if(node_forin->initializer->kind == NODE_KIND_VAR)
		{
			result = graph_var(symbol_initializer, node_forin->initializer);
		}
		else if(node_forin->initializer->kind == NODE_KIND_CONST)
		{
			result = graph_const(symbol_initializer, node_forin->initializer);
		}
		if (!result)
		{
			return 0;
		}
	}

	if (node_forin->expression)
	{
		symbol_t *symbol_expression;
		symbol_expression = symbol_rpush(symbol, SYMBOL_FLAG_FORIN_EXPRESSION, node);
		if(!symbol_expression)
		{
			return 0;
		}

		result = graph_expression(symbol_expression, node_forin->expression);
		if (!result)
		{
			return 0;
		}
	}

	if (node_forin->body)
	{
		result = graph_block(symbol, node_forin->body);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_break(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_BREAK, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_continue(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_CONTINUE, node);
	if(!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_return(symbol_t *parent, node_t *node)
{
	node_return_t *node_return;
	node_return = (node_return_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_RETURN, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = graph_expression(symbol, node_return->expression);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_throw(symbol_t *parent, node_t *node)
{
	node_throw_t *node_throw;
	node_throw = (node_throw_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_THROW, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = graph_expression(symbol, node_throw->expression);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_statement(symbol_t *parent, node_t *node)
{
	int32_t result = 1;
	switch (node->kind)
	{
	case NODE_KIND_BLOCK:
		result = graph_block(parent, node);
		break;

	case NODE_KIND_IF:
		result = graph_if(parent, node);
		break;

	case NODE_KIND_TRY:
		result = graph_try(parent, node);
		break;

	case NODE_KIND_FOR:
		result = graph_for(parent, node);
		break;

	case NODE_KIND_FORIN:
		result = graph_forin(parent, node);
		break;

	case NODE_KIND_VAR:
		result = graph_var(parent, node);
		break;

	case NODE_KIND_CONST:
		result = graph_const(parent, node);
		break;

	case NODE_KIND_TYPE:
		result = graph_type(parent, node);
		break;

	case NODE_KIND_BREAK:
		result = graph_break(parent, node);
		break;

	case NODE_KIND_CONTINUE:
		result = graph_continue(parent, node);
		break;

	case NODE_KIND_RETURN:
		result = graph_return(parent, node);
		break;

	case NODE_KIND_THROW:
		result = graph_throw(parent, node);
		break;

	case NODE_KIND_FUNC:
		result = graph_func(parent, node);
		break;

	default:
		result = graph_assign(parent, node);
		break;
	}

	return result;
}

static int32_t
graph_block(symbol_t *parent, node_t *node)
{
	node_block_t *node_block;
	node_block = (node_block_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_BLOCK, node);
	if(!symbol)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_block->list->begin; a != node_block->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result = graph_statement(symbol, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_type_parameter(symbol_t *parent, node_t *node)
{
	node_type_parameter_t *node_type_parameter;
	node_type_parameter = (node_type_parameter_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_TYPE_PARAMETER, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;

	if (node_type_parameter->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_type_parameter->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_type_parameter->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_type_parameter->extends)
	{
		symbol_t *symbol_extends;
		symbol_extends = symbol_rpush(symbol, SYMBOL_FLAG_TYPE_PARAMETER_TYPE, node_type_parameter->extends);
		if(!symbol_extends)
		{
			return 0;
		}

		result = graph_expression(symbol_extends, node_type_parameter->extends);
		if (!result)
		{
			return 0;
		}
	}

	if (node_type_parameter->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_TYPE_PARAMETER_VALUE, node_type_parameter->value);
		if(!symbol_value)
		{
			return 0;
		}

		result = graph_expression(symbol_value, node_type_parameter->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_enum_member(symbol_t *parent, node_t *node)
{
	node_enum_member_t *node_enum_member;
	node_enum_member = (node_enum_member_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_ENUM_MEMBER, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *symbol_name;
	symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_enum_member->name);
	if(!symbol_name)
	{
		return 0;
	}

	int32_t result;
	result = graph_id(symbol_name, node_enum_member->name);
	if (!result)
	{
		return 0;
	}

	if (node_enum_member->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_VALUE, node_enum_member->value);
		if(!symbol_value)
		{
			return 0;
		}

		int32_t result = graph_expression(symbol_value, node_enum_member->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_enum(symbol_t *parent, node_t *node)
{
	node_enum_t *node_enum;
	node_enum = (node_enum_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_ENUM, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_enum->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_enum->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_enum->name);
		if (!result)
		{
			return 0;
		}
	}

	ilist_t *a;
	for (a = node_enum->body->begin; a != node_enum->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result;
		result = graph_enum_member(symbol, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_property(symbol_t *parent, node_t *node)
{
	node_property_t *node_property;
	node_property = (node_property_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_PROPERTY, node);
	if (!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_property->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_property->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_property->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_property->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_FLAG_PROPERTY_TYPE, node_property->type);
		if(!symbol_type)
		{
			return 0;
		}

		result = graph_expression(symbol_type, node_property->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_property->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_PROPERTY_VALUE, node_property->value);
		if(!symbol_value)
		{
			return 0;
		}

		result = graph_expression(symbol_value, node_property->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_parameter(symbol_t *parent, node_t *node)
{
	node_parameter_t *node_parameter;
	node_parameter = (node_parameter_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_PARAMETER, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_parameter->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_parameter->name);
		if(!symbol_name)
		{
			return 0;
		}
		
		result = graph_id(symbol_name, node_parameter->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_parameter->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_FLAG_PARAMETER_TYPE, node_parameter->type);
		if(!symbol_type)
		{
			return 0;
		}

		result = graph_expression(symbol_type, node_parameter->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_parameter->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_PARAMETER_VALUE, node_parameter->value);
		if(!symbol_value)
		{
			return 0;
		}

		result = graph_expression(symbol_value, node_parameter->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_field(symbol_t *parent, node_t *node)
{
	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FIELD, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_field->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_field->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_field->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_field->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_FLAG_TYPE_EXPRESSION, node_field->type);
		if(!symbol_type)
		{
			return 0;
		}

		result = graph_expression(symbol_type, node_field->type);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_method(symbol_t *parent, node_t *node)
{
	node_method_t *node_method;
	node_method = (node_method_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_METHOD, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_method->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_method->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_method->name);
		if (!result)
		{
			return 0;
		}
	}

	ilist_t *a;
	if (node_method->parameters)
	{
		for (a = node_method->parameters->begin; a != node_method->parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_parameter(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_method->type_parameters)
	{
		for (a = node_method->type_parameters->begin; a != node_method->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_type_parameter(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_method->body)
	{
		result = graph_block(symbol, node_method->body);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_heritage(symbol_t *parent, node_t *node)
{
	node_heritage_t *node_heritage;
	node_heritage = (node_heritage_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_HERITAGE, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_heritage->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_heritage->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_heritage->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_heritage->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_FLAG_HERITAGE_TYPE, node);
		if(!symbol_type)
		{
			return 0;
		}

		int32_t result = graph_expression(symbol_type, node_heritage->type);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_class(symbol_t *parent, node_t *node)
{
	node_class_t *node_class;
	node_class = (node_class_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_CLASS, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_class->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_class->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_class->name);
		if (!result)
		{
			return 0;
		}
	}

	ilist_t *a;
	if (node_class->heritages)
	{
		for (a = node_class->heritages->begin; a != node_class->heritages->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_heritage(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_class->type_parameters)
	{
		for (a = node_class->type_parameters->begin; a != node_class->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_type_parameter(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	for (a = node_class->body->begin; a != node_class->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		
		switch (temp->kind)
		{
		case NODE_KIND_EXPORT:
			result = graph_export(symbol, temp);
			break;

		case NODE_KIND_CLASS:
			result = graph_class(symbol, temp);
			break;

		case NODE_KIND_ENUM:
			result = graph_enum(symbol, temp);
			break;

		case NODE_KIND_PROPERTY:
			result = graph_property(symbol, temp);
			break;

		case NODE_KIND_METHOD:
			result = graph_method(symbol, temp);
			break;

		default:
			return 0;
		}

		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_func(symbol_t *parent, node_t *node)
{
	node_func_t *node_func;
	node_func = (node_func_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_FUNCTION, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_func->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_func->name);
		if(!symbol_name)
		{
			return 0;
		}

		result = graph_id(symbol_name, node_func->name);
		if (!result)
		{
			return 0;
		}
	}

	ilist_t *a;
	if (node_func->parameters)
	{
		for (a = node_func->parameters->begin; a != node_func->parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_parameter(parent, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->type_parameters)
	{
		for (a = node_func->type_parameters->begin; a != node_func->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_type_parameter(parent, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->fields)
	{
		ilist_t *a;
		for (a = node_func->fields->begin; a != node_func->fields->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;
			result = graph_field(parent, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->body)
	{
		if(node_func->body->kind == NODE_KIND_BLOCK)
		{
			result = graph_block(symbol, node_func->body);
		}
		else
		{
			symbol_t *symbol_value;
			symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_VALUE, node_func->body);
			if(!symbol_value)
			{
				return 0;
			}

			result = graph_expression(symbol_value, node_func->body);
		}
		
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_var(symbol_t *parent, node_t *node)
{
	node_var_t *node_var;
	node_var = (node_var_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_VAR, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *symbol_name;
	symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_var->name);
	if(!symbol_name)
	{
		return 0;
	}

	int32_t result;
	if (node_var->name->kind == NODE_KIND_OBJECT)
	{
		result = graph_object(symbol_name, node_var->name);
		if (!result)
		{
			return 0;
		}
	}
	else if (node_var->name->kind == NODE_KIND_ARRAY)
	{
		result = graph_array(symbol_name, node_var->name);
		if (!result)
		{
			return 0;
		}
	}
	else
	{
		result = graph_id(symbol_name, node_var->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_var->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_FLAG_TYPE, node_var->type);
		if(!symbol_type)
		{
			return 0;
		}

		result = graph_expression(symbol_type, node_var->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_var->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_VALUE, node_var->value);
		if(!symbol_value)
		{
			return 0;
		}

		result = graph_expression(symbol_value, node_var->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_const(symbol_t *parent, node_t *node)
{
	node_const_t *node_const;
	node_const = (node_const_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_CONST, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *symbol_name;
	symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_const->name);
	if(!symbol_name)
	{
		return 0;
	}

	int32_t result;
	if (node_const->name->kind == NODE_KIND_OBJECT)
	{
		result = graph_object(symbol_name, node_const->name);
		if (!result)
		{
			return 0;
		}
	}
	else if (node_const->name->kind == NODE_KIND_ARRAY)
	{
		result = graph_array(symbol_name, node_const->name);
		if (!result)
		{
			return 0;
		}
	}
	else
	{
		result = graph_id(symbol_name, node_const->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_const->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_FLAG_TYPE, node_const->type);
		if(!symbol_type)
		{
			return 0;
		}

		result = graph_expression(symbol_type, node_const->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_const->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_VALUE, node_const->value);
		if(!symbol_value)
		{
			return 0;
		}

		result = graph_expression(symbol_value, node_const->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_type(symbol_t *parent, node_t *node)
{
	node_type_t *node_type;
	node_type = (node_type_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_TYPE, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *symbol_name;
	symbol_name = symbol_rpush(symbol, SYMBOL_FLAG_NAME, node_type->name);
	if(!symbol_name)
	{
		return 0;
	}

	int32_t result;
	result = graph_id(symbol_name, node_type->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	if (node_type->type_parameters)
	{
		for (a = node_type->type_parameters->begin; a != node_type->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_type_parameter(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_type->heritages)
	{
		for (a = node_type->heritages->begin; a != node_type->heritages->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_heritage(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	symbol_t *symbol_value;
	symbol_value = symbol_rpush(symbol, SYMBOL_FLAG_VALUE, node_type->body);
	if(!symbol_value)
	{
		return 0;
	}

	result = graph_expression(symbol_value, node_type->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_export(symbol_t *parent, node_t *node)
{
	node_modifier_t *node_export;
	node_export = (node_modifier_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_EXPORT, node);

	int32_t result = 1;
	switch (node_export->x->kind)
	{
	case NODE_KIND_CLASS:
		result = graph_class(symbol, node_export->x);
		break;

	case NODE_KIND_ENUM:
		result = graph_enum(symbol, node_export->x);
		break;

	case NODE_KIND_FUNC:
		result = graph_func(symbol, node_export->x);
		break;

	case NODE_KIND_VAR:
		result = graph_var(symbol, node_export->x);
		break;

	case NODE_KIND_CONST:
		result = graph_const(symbol, node_export->x);
		break;

	case NODE_KIND_TYPE:
		result = graph_type(symbol, node_export->x);
		break;

	case NODE_KIND_OBJECT:
		result = graph_object(symbol, node_export->x);
		break;

	default:
		return 0;
	}

	return result;
}

static int32_t
graph_import(symbol_t *parent, node_t *node)
{
	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_IMPORT, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *symbol_path;
	symbol_path = symbol_rpush(symbol, SYMBOL_FLAG_PATH, node_import->path);
	if(!symbol_path)
	{
		return 0;
	}

	if(node_import->fields)
	{
		ilist_t *a;
		for (a = node_import->fields->begin; a != node_import->fields->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			int32_t result = graph_field(symbol, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
graph_module(symbol_t *parent, node_t *node)
{	
	node_module_t *node_module;
	node_module = (node_module_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FLAG_MODULE, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = 1;
	ilist_t *a;
	for (a = node_module->members->begin; a != node_module->members->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		switch (temp->kind)
		{
		case NODE_KIND_EXPORT:
			result = graph_export(symbol, temp);
			break;

		case NODE_KIND_IMPORT:
			result = graph_import(symbol, temp);
			break;

		case NODE_KIND_CLASS:
			result = graph_class(symbol, temp);
			break;

		case NODE_KIND_ENUM:
			result = graph_enum(symbol, temp);
			break;

		case NODE_KIND_FUNC:
			result = graph_func(symbol, temp);
			break;

		case NODE_KIND_VAR:
			result = graph_var(symbol, temp);
			break;

		case NODE_KIND_CONST:
			result = graph_const(symbol, temp);
			break;

		case NODE_KIND_TYPE:
			result = graph_type(symbol, temp);
			break;

		default:
			return 0;
		}
	}

	return result;
}

int32_t
graph_run(graph_t *graph, syntax_t *syntax)
{
	ilist_t *a;
	for(a = syntax->modules->begin; a != syntax->modules->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result = graph_module(graph->symbol, temp);
		if(!result)
		{
			return 0;
		}
	}

	return 1;
}

graph_t *
graph_create(program_t *program, list_t *errors)
{
	graph_t *graph;
	graph = (graph_t *)malloc(sizeof(graph_t));
	if (!graph)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(graph_t));
		return NULL;
	}
	memset(graph, 0, sizeof(graph_t));

	graph->program = program;
	graph->errors = errors;

	symbol_t *symbol;
	symbol = symbol_create(SYMBOL_FLAG_NONE, NULL);
	if(!symbol)
	{
		return 0;
	}

	graph->symbol = symbol;

	return graph;
}

static error_t *
graph_error(graph_t *graph, symbol_t *current, const char *format, ...)
{
	char *message;
	message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	va_list arg;
	if (format)
	{
		va_start(arg, format);
		vsprintf(message, format, arg);
		va_end(arg);
	}

	node_t *node;
	node = current->declaration;

	error_t *error;
	error = error_create(node->position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(graph->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}



static int32_t
graph_analysis_symbol_in_object_struct(graph_t *graph, symbol_t *current, symbol_t *target);

static int32_t
graph_analysis_symbol_in_array_struct(graph_t *graph, symbol_t *current, symbol_t *target);


static int32_t
graph_analysis_compare_symbol_id(symbol_t *current, symbol_t *target)
{
	node_t *node_current = current->declaration;
	node_basic_t *node_basic_current;
	node_basic_current = (node_basic_t *)node_current->value;

	node_t *node_target = target->declaration;
	node_basic_t *node_basic_target;
	node_basic_target = (node_basic_t *)node_target->value;

	return (strncmp(node_basic_target->value, node_basic_current->value, 
		max(strlen(node_basic_current->value), strlen(node_basic_target->value))) == 0);
}


static int32_t
graph_analysis_same_symbol_in_name_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return graph_analysis_symbol_in_object_struct(graph, a, target);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return graph_analysis_symbol_in_array_struct(graph, a, target);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(a->id == target->id)
			{
				break;
			}
			if(graph_analysis_compare_symbol_id(a, target))
			{
				graph_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
graph_analysis_symbol_in_object_property_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return graph_analysis_same_symbol_in_name_struct(graph, a, target);
		}
	}
	return 1;
}

static int32_t
graph_analysis_symbol_in_object_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return graph_analysis_symbol_in_object_property_struct(graph, a, target);
		}
	}
	return 1;
}

static int32_t
graph_analysis_symbol_in_array_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return graph_analysis_symbol_in_object_struct(graph, a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return graph_analysis_symbol_in_array_struct(graph, a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(graph_analysis_compare_symbol_id(a, target))
			{
				graph_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
graph_analysis_same_symbol_contain_name_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return graph_analysis_same_symbol_in_name_struct(graph, a, target);
		}
	}

	return 1;
}

static int32_t
graph_analysis_symbol_is_duplicated(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = root->begin; a && (a != sub) && (a != root->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					return graph_analysis_same_symbol_contain_name_struct(graph, b, target);
				}
			}
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = graph_analysis_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			symbol_t *b;
			for(b = a->begin; b && (b != a->end); b = b->next)
			{
				result = graph_analysis_symbol_is_duplicated(graph, root, a, target);
				if(!result)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}


static int32_t
graph_analysis_object(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);

static int32_t
graph_analysis_array(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);


static int32_t
graph_analysis_name(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return graph_analysis_object(graph, root, sub, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return graph_analysis_array(graph, root, sub, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return graph_analysis_symbol_is_duplicated(graph, root, sub, a);
		}
	}
	return 0;
}

static int32_t
graph_analysis_object_property(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return graph_analysis_name(graph, root, sub, a);
		}
	}
	return 0;
}

static int32_t
graph_analysis_object(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return graph_analysis_object_property(graph, root, sub, a);
		}
	}
	return 0;
}

static int32_t
graph_analysis_array(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return graph_analysis_object(graph, root, sub, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return graph_analysis_array(graph, root, sub, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return graph_analysis_symbol_is_duplicated(graph, root, sub, a);
		}
	}
	return 0;
}


static int32_t
graph_analysis_contain_name(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, sub, a);
		}
	}
	return result;
}


static int32_t
graph_analysis_contain_symbol_by_flag(symbol_t *refrence, uint64_t flag)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			return 1;
		}
	}
	return 0;
}

static int32_t
graph_analysis_parameter_is_ellipsis(symbol_t *refrence)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_ELLIPSIS))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}


static int32_t
graph_analysis_is_same_type_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	uint64_t refrence_counter_by_value = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (graph_analysis_contain_symbol_by_flag(a, SYMBOL_FLAG_TYPE_PARAMETER_VALUE))
			{
				refrence_counter_by_value += 1;
			}
			refrence_counter += 1;
		}
	}

	uint64_t target_counter = 0;
	uint64_t target_counter_by_value = 0;

	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (graph_analysis_contain_symbol_by_flag(a, SYMBOL_FLAG_TYPE_PARAMETER_VALUE))
			{
				target_counter_by_value += 1;
			}
			target_counter += 1;
		}
	}

	if ((refrence_counter - refrence_counter_by_value) == (target_counter - target_counter_by_value))
	{
		return 1;
	}

	return 0;
}

static int32_t
graph_analysis_subset_of_symbol_type(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		refrence_counter += 1;
		int32_t founded = 0;
		uint64_t target_counter = 0;
		symbol_t *b;
		for (b = target->begin; b != target->end; b = b->next)
		{
			target_counter += 1;
			if(target_counter < refrence_counter)
			{
				continue;
			}

			founded = 1;

			if (!symbol_equal_flag(a, b))
			{
				return 0;
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_ID))
			{
				if (!graph_analysis_compare_symbol_id(a, b))
				{
					return 0;
				}
			}
			if (!graph_analysis_subset_of_symbol_type(a, b))
			{
				return 0;
			}
			break;
		}
		if (!founded)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
graph_analysis_subset_of_parameter_type(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER_TYPE))
		{
			refrence_counter += 1;
			int32_t founded = 0;
			uint64_t target_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER_TYPE))
				{
					target_counter += 1;
					if(target_counter < refrence_counter)
					{
						continue;
					}
					founded = 1;
					if(!graph_analysis_subset_of_symbol_type(a, b))
					{
						return 0;
					}
					break;
				}
			}
			if(!founded)
			{
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
graph_analysis_subset_of_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_parameter_counter = 0;  
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			if (graph_analysis_parameter_is_ellipsis(a))
			{
				goto subset;
			}
			if (graph_analysis_contain_symbol_by_flag(a, SYMBOL_FLAG_PARAMETER_VALUE))
			{
				continue;
			}

			refrence_parameter_counter += 1;
			int32_t target_founded = 0;
			uint64_t target_parameter_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER))
				{
					target_parameter_counter += 1;
					if (target_parameter_counter < refrence_parameter_counter)
					{
						continue;
					}
					target_founded = 1;

					if (graph_analysis_parameter_is_ellipsis(b))
					{
						goto subset;
					}

					if (!graph_analysis_subset_of_parameter_type(a, b))
					{
						goto not_subset;
					}
					break;
				}
			}
			
			if (!target_founded)
			{
				if (graph_analysis_contain_symbol_by_flag(a, SYMBOL_FLAG_PARAMETER_VALUE))
				{
					continue;
				}
				goto not_subset;
			}
		}
	}

	uint64_t target_parameter_counter = 0;
	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			target_parameter_counter += 1;
			if (target_parameter_counter <= refrence_parameter_counter)
			{
				continue;
			}

			if (graph_analysis_parameter_is_ellipsis(a))
			{
				goto subset;
			}
			
			if (!graph_analysis_contain_symbol_by_flag(a, SYMBOL_FLAG_PARAMETER_VALUE))
			{
				goto not_subset;
			}
		}
	}

	subset:
	return 1;

	not_subset:
	return 0;
}

static int32_t
graph_analysis_method_on_name_struct(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(a->id == target->id)
			{
				break;
			}
			if(graph_analysis_compare_symbol_id(a, target))
			{
				result = graph_analysis_is_same_type_parameter(refrence, target_refrence);
				if(result)
				{
					result = graph_analysis_subset_of_parameter(refrence, target_refrence);
					if(result)
					{
						goto error;
					}
					result = graph_analysis_subset_of_parameter(target_refrence, refrence);
					if(result)
					{
						goto error;
					}
				}
				return 1;

				error:
				graph_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
graph_analysis_method_contain_name_struct(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return graph_analysis_method_on_name_struct(graph, a, refrence, target_refrence, target);
		}
	}
	return 1;
}

static int32_t
graph_analysis_method_is_duplicated(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = root->begin; a && (a != sub) && (a != root->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = graph_analysis_method_contain_name_struct(graph, a, refrence, a, target);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			symbol_t *b;
			for (b = a->begin; b && (b != a->end); b = b->next)
			{
				result = graph_analysis_method_is_duplicated(graph, root, a, refrence, target);
				if (!result)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

static int32_t
graph_analysis_method_name(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *refrence, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return graph_analysis_method_is_duplicated(graph, root, sub, refrence, a);
		}
	}
	return 0;
}





static int32_t
graph_analysis_import(graph_t *graph, symbol_t *root , symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result &= graph_analysis_contain_name(graph, current, a, a);
			if(!result)
			{
				return 0;
			}
			result &= graph_analysis_contain_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_type_parameter(graph_t *graph, symbol_t *root , symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_heritage(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
			symbol_t *b;
			for(b = root->begin; b != root->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= graph_analysis_contain_name(graph, root, b, b);
					if(!result)
					{
						return result;
					}
				}
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_method(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_method_name(graph, root, current, current, a);
			if (!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_class(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result &= graph_analysis_heritage(graph, current, a);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			result &= graph_analysis_type_parameter(graph, current, a);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			result &= graph_analysis_class(graph, current, a);
			if(!result)
			{
				return 0;
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				result &= graph_analysis_name(graph, current, a, a);
				if(!result)
				{
					return 0;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			symbol_t *b;
			for (b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			result &= graph_analysis_method(graph, current, a);
			if (!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				result &= graph_analysis_name(graph, current, a, a);
				if(!result)
				{
					return 0;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= graph_analysis_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				result &= graph_analysis_name(graph, current, a, a);
				if(!result)
				{
					return 0;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			symbol_t *b;
			for (b = a->begin; b != a->end; b = b->next)
			{
				result &= graph_analysis_class(graph, root, b);
				if (!result)
				{
					return result;
				}
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_enum(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_var(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_const(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_func(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= graph_analysis_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
graph_analysis_export(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			result &= graph_analysis_import(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result &= graph_analysis_class(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result &= graph_analysis_enum(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result &= graph_analysis_var(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result &= graph_analysis_const(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result &= graph_analysis_func(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			result &= graph_analysis_export(graph, root, a);
		}
	}

	return result;
}



static int32_t
graph_analysis_module(graph_t *graph, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			result &= graph_analysis_import(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result &= graph_analysis_class(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result &= graph_analysis_enum(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result &= graph_analysis_var(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result &= graph_analysis_const(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result &= graph_analysis_func(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			result &= graph_analysis_export(graph, current, a);
		}
	}

	return result;
}

int32_t
graph_analysis(graph_t *graph)
{
	symbol_t *symbol;
	symbol = (symbol_t *)graph->symbol;

	int32_t result = 1;

	symbol_t *a;
	for(a = symbol->begin; a != symbol->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_MODULE))
		{
			result &= graph_analysis_module(graph, a);
		}
	}

	return result;
}





