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
#include "symbol.h"
#include "graph.h"
#include "syntax.h"

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
graph_generic(symbol_t *parent, node_t *node);

static int32_t
graph_postfix(symbol_t *parent, node_t *node);

static int32_t
graph_block(symbol_t *parent, node_t *node);

static int32_t
graph_var(symbol_t *parent, node_t *node);

static int32_t
graph_type(symbol_t *parent, node_t *node);

static int32_t
graph_prefix(symbol_t *parent, node_t *node);


static int32_t
graph_id(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_TYPE_ID, node);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_NUMBER, node);
	if (!symbol)
	{
		return 0;
	}
	return 1;
}

static int32_t
graph_string(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_TYPE_STRING, node);
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

	symbol = symbol_rpush(parent, SYMBOL_TYPE_CHAR, node);
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
	
	symbol = symbol_rpush(parent, SYMBOL_TYPE_NULL, node);
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
	
	symbol = symbol_rpush(parent, SYMBOL_TYPE_TRUE, node);
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
	
	symbol = symbol_rpush(parent, SYMBOL_TYPE_FALSE, node);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_ARRAY, node);
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

	symbol = symbol_rpush(parent, SYMBOL_TYPE_PARENTHESIS, node);
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
graph_lambda(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_LAMBDA, node);
	if(!symbol)
	{
		return 0;
	}

	node_lambda_t *node_lambda = (node_lambda_t *)node->value;

	int32_t result;
	if (node_lambda->parameters)
	{
		symbol_t *parameters;
		parameters = symbol_rpush(symbol, SYMBOL_TYPE_PARAMETERS, NULL);

		ilist_t *a;
		for (a = node_lambda->parameters->begin; a != node_lambda->parameters->end; a = a->next)
		{
			node_t *parameter = (node_t *)a->value;

			result = graph_parameter(parameters, parameter);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_lambda->body)
	{
		if(node_lambda->body->kind == NODE_KIND_BLOCK)
		{
			result = graph_block(symbol, node_lambda->body);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			symbol_t *value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, NULL);
			if(!value)
			{
				return 0;
			}

			result = graph_expression(value, node_lambda->body);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
graph_object_property(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_OBJECT_PROPERTY, node);
	if(!symbol)
	{
		return 0;
	}

	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	if(node_object_property->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_object_property->name);
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
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, node_object_property->value);
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
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_OBJECT, node);
	if(!symbol)
	{
		return 0;
	}

	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

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

	case NODE_KIND_LAMBDA:
		result = graph_lambda(parent, node);
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
graph_argument(symbol_t *parent, node_t *node)
{
	node_argument_t *node_argument = (node_argument_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_ARGUMENT, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_argument->name)
	{
		symbol_t *name;
		name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}
		
		result = graph_expression(name, node_argument->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_argument->value)
	{
		symbol_t *value;
		value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, NULL);
		if(!value)
		{
			return 0;
		}

		result = graph_expression(value, node_argument->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_composite(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_COMPOSITE, node);
	if (!symbol)
	{
		return 0;
	}

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	int32_t result;
	if (node_composite->base)
	{
		symbol_t *base = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node);
		if(!base)
		{
			return 0;
		}

		result = graph_expression(base, node_composite->base);
		if (!result)
		{
			return 0;
		}
	}

	if (node_composite->arguments)
	{
		symbol_t *arguments = symbol_rpush(symbol, SYMBOL_TYPE_ARGUMENTS, NULL);
		if(!arguments)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_composite->arguments->begin; a != node_composite->arguments->end; a = a->next)
		{
			node_t *argument = (node_t *)a->value;

			result = graph_argument(arguments, argument);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_CALL, node);
	if(!symbol)
	{
		return 0;
	}

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	symbol_t *name;
	name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
	if(!name)
	{
		return 0;
	}

	int32_t result;
	result = graph_postfix(name, node_composite->base);
	if (!result)
	{
		return 0;
	}

	if (node_composite->arguments)
	{
		symbol_t *arguments = symbol_rpush(symbol, SYMBOL_TYPE_ARGUMENTS, NULL);
		if(!arguments)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_composite->arguments->begin; a != node_composite->arguments->end; a = a->next)
		{
			node_t *argument = (node_t *)a->value;

			result = graph_argument(arguments, argument);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
graph_item(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_ITEM, node);
	if(!symbol)
	{
		return 0;
	}

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;
	
	symbol_t *name;
	name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
	if(!name)
	{
		return 0;
	}

	int32_t result;
	result = graph_postfix(name, node_composite->base);
	if (!result)
	{
		return 0;
	}

	if (node_composite->arguments)
	{
		symbol_t *arguments = symbol_rpush(symbol, SYMBOL_TYPE_ARGUMENTS, NULL);
		if(!arguments)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_composite->arguments->begin; a != node_composite->arguments->end; a = a->next)
		{
			node_t *argument = (node_t *)a->value;

			result = graph_argument(arguments, argument);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
graph_attribute(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	symbol = symbol_rpush(parent, SYMBOL_TYPE_ATTR, node);
	if(!symbol)
	{
		return 0;
	}

	node_binary_t *node_binary;
	node_binary = (node_binary_t *)node->value;
	
	int32_t result;
	if(node_binary->left)
	{
		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, NULL);
		if(!symbol_left)
		{
			return 0;
		}
		
		result = graph_postfix(symbol_left, node_binary->left);
		if (!result)
		{
			return 0;
		}
	}

	if(node_binary->right)
	{
		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, NULL);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_id(symbol_right, node_binary->right);
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

	case NODE_KIND_GET_ITEM:
		result = graph_item(parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_ATTRIBUTE:
		result = graph_attribute(parent, node);
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
	symbol_t *symbol = NULL;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_TILDE)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_TILDE, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_NOT)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_NOT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_NEG)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_NEG, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_POS)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_POS, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_SIZEOF)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_SIZEOF, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_TYPEOF)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_TYPEOF, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_ELLIPSIS)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_ELLIPSIS, node);
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
	case NODE_KIND_SIZEOF:
	case NODE_KIND_TYPEOF:
	case NODE_KIND_ELLIPSIS:
		node_unary = (node_unary_t *)node->value;

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_unary->right);
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
graph_power(symbol_t *parent, node_t *node)
{
	symbol_t *symbol = NULL;
	if(node->kind == NODE_KIND_TILDE)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_POW, node);
		if(!symbol)
		{
			return 0;
		}
	}
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_POW:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		if (node_binary->left->kind == NODE_KIND_POW)
		{
			result = graph_power(symbol_left, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
graph_multiplicative(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_MUL)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_MUL, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_DIV)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_DIV, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_MOD)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_MOD, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_EPI)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_EPI, node);
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
	case NODE_KIND_EPI:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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
		else if (node_binary->left->kind == NODE_KIND_EPI)
		{
			result = graph_multiplicative(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = graph_power(symbol_left, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
		if(!symbol_right)
		{
			return 0;
		}

		result = graph_power(symbol_right, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = graph_power(parent, node);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_PLUS, node);
		if(!symbol)
		{
			return 0;
		}
	}
	
	if(node->kind == NODE_KIND_MINUS)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_MINUS, node);
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

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_SHL, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_SHR)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_SHR, node);
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

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_LT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_GT)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_GT, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_LE)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_LE, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_GE)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_GE, node);
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

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_EQ, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_IN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_IN, node);
		if(!symbol)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_NEQ)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_NEQ, node);
		if(!symbol)
		{
			return 0;
		}
	}

	switch (node->kind)
	{
	case NODE_KIND_EQ:
	case NODE_KIND_IN:
	case NODE_KIND_NEQ:
		node_binary = (node_binary_t *)node->value;
		
		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_AND, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_XOR, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_OR, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_LAND, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
		symbol = symbol_rpush(parent, SYMBOL_TYPE_LOR, node);
		if(!symbol)
		{
			return 0;
		}

		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
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

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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
	symbol_t *condition;
	symbol_t *symbol_true;
	symbol_t *symbol_false;

	int32_t result;
	node_triple_t *node_triple;
	switch (node->kind)
	{
	case NODE_KIND_CONDITIONAL:
		node_triple = (node_triple_t *)node->value;

		symbol = symbol_rpush(parent, SYMBOL_TYPE_CONDITIONAL, node);
		if(!symbol)
		{
			return 0;
		}

		condition = symbol_rpush(symbol, SYMBOL_TYPE_CONDITION, NULL);
		if(!condition)
		{
			return 0;
		}

		result = graph_logical_or(condition, node_triple->base);
		if (!result)
		{
			return 0;
		}

		symbol_true = symbol_rpush(symbol, SYMBOL_TYPE_TRUE, NULL);
		if(!symbol_true)
		{
			return 0;
		}
		result = graph_conditional(symbol_true, node_triple->left);
		if (!result)
		{
			return 0;
		}

		symbol_false = symbol_rpush(symbol, SYMBOL_TYPE_FALSE, NULL);
		if(!symbol_false)
		{
			return 0;
		}
		result = graph_conditional(symbol_false, node_triple->right);
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

	symbol_t *symbol = NULL;
	symbol_t *symbol_left;
	symbol_t *symbol_right;

	if(node->kind == NODE_KIND_DEFINE)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_DEFINE, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_ADD_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_ADD_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_SUB_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_SUB_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_DIV_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_DIV_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_MUL_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_MUL_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_MOD_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_MOD_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_AND_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_AND_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_OR_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_OR_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_SHL_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_SHL_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	else if (node->kind == NODE_KIND_SHR_ASSIGN)
	{
		symbol = symbol_rpush(parent, SYMBOL_TYPE_SHR_ASSIGN, node);
		if(!symbol)
		{
			return 0;
		}
	}

	int32_t result;
	switch (node->kind)
	{
	case NODE_KIND_ASSIGN:
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

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_LEFT, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		result = graph_expression(symbol_left, node_binary->left);
		if (!result)
		{
			return 0;
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_RIGHT, node_binary->right);
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

	case NODE_KIND_DEFINE:
		node_binary = (node_binary_t *)node->value;

		symbol_left = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_binary->left);
		if(!symbol_left)
		{
			return 0;
		}

		result = graph_expression(symbol_left, node_binary->left);
		if (!result)
		{
			return 0;
		}

		symbol_right = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, node_binary->right);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_IF, node);
	if (!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_if->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_if->name);
		if(!symbol_name)
		{
			return 0;
		}
		result = graph_id(symbol_name, node_if->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_if->condition)
	{
		symbol_t *symbol_condition;
		symbol_condition = symbol_rpush(symbol, SYMBOL_TYPE_CONDITION, node_if->condition);
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
		symbol_else = symbol_rpush(symbol, SYMBOL_TYPE_ELSE, node_if->else_body);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_CATCH, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_catch->parameters)
	{
		symbol_t *parameters;
		parameters = symbol_rpush(symbol, SYMBOL_TYPE_PARAMETERS, NULL);

		ilist_t *a;
		for (a = node_catch->parameters->begin; a != node_catch->parameters->end; a = a->next)
		{
			node_t *parameter = (node_t *)a->value;

			result = graph_parameter(parameters, parameter);
			if (!result)
			{
				return 0;
			}
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_TRY, node);
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

	if (node_try->catchs)
	{
		symbol_t *catchs;
		catchs = symbol_rpush(symbol, SYMBOL_TYPE_CATCHS, NULL);
		if(!catchs)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_try->catchs->begin; a != node_try->catchs->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;
			result = graph_catch(catchs, temp);
			if (!result)
			{
				return 0;
			}
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_FOR, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_for->name)
	{
		symbol_t *name;
		name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}
		result = graph_id(name, node_for->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_for->initializer)
	{
		symbol_t *initializer;
		initializer = symbol_rpush(symbol, SYMBOL_TYPE_INITIALIZER, NULL);
		if(!initializer)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_for->initializer->begin; a != node_for->initializer->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			if (temp->kind == NODE_KIND_VAR)
			{
				result = graph_var(initializer, temp);
				if (!result)
				{
					return 0;
				}
			}
			else
			{
				result = graph_assign(initializer, temp);
				if (!result)
				{
					return 0;
				}
			}
		}
	}

	if (node_for->condition)
	{
		symbol_t *symbol_condition;
		symbol_condition = symbol_rpush(symbol, SYMBOL_TYPE_CONDITION, NULL);
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
		symbol_t *incrementor;
		incrementor = symbol_rpush(symbol, SYMBOL_TYPE_INCREMENTOR, NULL);
		if(!incrementor)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_for->incrementor->begin; a != node_for->incrementor->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			result = graph_assign(incrementor, temp);
			if (!result)
			{
				return 0;
			}
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_FORIN, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_forin->name)
	{
		symbol_t *name;
		name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}
		result = graph_id(name, node_forin->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_forin->initializer)
	{
		symbol_t *initializer;
		initializer = symbol_rpush(symbol, SYMBOL_TYPE_INITIALIZER, NULL);
		if(!initializer)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_forin->initializer->begin; a != node_forin->initializer->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			if (temp->kind == NODE_KIND_VAR)
			{
				result = graph_var(initializer, temp);
				if (!result)
				{
					return 0;
				}
			}
			else
			{
				result = graph_assign(initializer, temp);
				if (!result)
				{
					return 0;
				}
			}
		}
	}

	if (node_forin->expression)
	{
		symbol_t *expression;
		expression = symbol_rpush(symbol, SYMBOL_TYPE_EXPRESSION, NULL);
		if(!expression)
		{
			return 0;
		}

		result = graph_expression(expression, node_forin->expression);
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
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_BREAK, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = graph_expression(symbol, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_continue(symbol_t *parent, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_CONTINUE, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = graph_expression(symbol, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_return(symbol_t *parent, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_RETURN, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = graph_expression(symbol, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
graph_throw(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_THROW, node);
	if(!symbol)
	{
		return 0;
	}

	node_throw_t *node_throw;
	node_throw = (node_throw_t *)node->value;

	if (node_throw->arguments)
	{
		symbol_t *arguments = symbol_rpush(symbol, SYMBOL_TYPE_ARGUMENTS, NULL);
		if(!arguments)
		{
			return 0;
		}

		ilist_t *a;
		for (a = node_throw->arguments->begin; a != node_throw->arguments->end; a = a->next)
		{
			node_t *argument = (node_t *)a->value;

			int32_t result;
			result = graph_argument(arguments, argument);
			if (!result)
			{
				return 0;
			}
		}
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_BLOCK, node);
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
graph_generic(symbol_t *parent, node_t *node)
{
	node_generic_t *node_generic;
	node_generic = (node_generic_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_GENERIC, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;

	if (node_generic->name)
	{
		symbol_t *name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}

		result = graph_id(name, node_generic->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_generic->type)
	{
		symbol_t *type = symbol_rpush(symbol, SYMBOL_TYPE_TYPE, NULL);
		if(!type)
		{
			return 0;
		}

		result = graph_expression(type, node_generic->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_generic->value)
	{
		symbol_t *value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, NULL);
		if(!value)
		{
			return 0;
		}

		result = graph_expression(value, node_generic->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_member(symbol_t *parent, node_t *node)
{
	node_member_t *node_member = (node_member_t *)node->value;

	symbol_t *symbol = symbol_rpush(parent, SYMBOL_TYPE_MEMBER, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
	if(!name)
	{
		return 0;
	}

	int32_t result;
	result = graph_id(name, node_member->name);
	if (!result)
	{
		return 0;
	}

	if (node_member->value)
	{
		symbol_t *value;
		value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, NULL);
		if(!value)
		{
			return 0;
		}

		result = graph_expression(value, node_member->value);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_ENUM, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_enum->name)
	{
		symbol_t *name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}

		result = graph_id(name, node_enum->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_enum->body)
	{
		symbol_t *members = symbol_rpush(symbol, SYMBOL_TYPE_MEMBERS, NULL);
		if(!members)
		{
			return 0;
		}
		ilist_t *a;
		for (a = node_enum->body->begin; a != node_enum->body->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			result = graph_member(members, temp);
			if (!result)
			{
				return 0;
			}
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_PROPERTY, node);
	if (!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_property->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_property->name);
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
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE_TYPE, node_property->type);
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
		symbol_value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, node_property->value);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_PARAMETER, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_parameter->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_parameter->name);
		if(!symbol_name)
		{
			return 0;
		}
		
		result = graph_expression(symbol_name, node_parameter->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_parameter->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE_TYPE, node_parameter->type);
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
		symbol_value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, node_parameter->value);
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
graph_method(symbol_t *parent, node_t *node)
{
	node_method_t *node_method;
	node_method = (node_method_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_METHOD, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_method->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_method->name);
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
		symbol_t *symbol_parameters;
		symbol_parameters = symbol_rpush(symbol, SYMBOL_TYPE_PARAMETERS, NULL);

		for (a = node_method->parameters->begin; a != node_method->parameters->end; a = a->next)
		{
			node_t *parameter;
			parameter = (node_t *)a->value;

			result = graph_parameter(symbol_parameters, parameter);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_method->generics)
	{
		symbol_t *symbol_generics;
		symbol_generics = symbol_rpush(symbol, SYMBOL_TYPE_GENERICS, NULL);

		for (a = node_method->generics->begin; a != node_method->generics->end; a = a->next)
		{
			node_t *generic;
			generic = (node_t *)a->value;

			result = graph_generic(symbol_generics, generic);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_HERITAGE, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_heritage->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_heritage->name);
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
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE_TYPE, node);
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
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_CLASS, node);
	if(!symbol)
	{
		return 0;
	}

	node_class_t *node_class;
	node_class = (node_class_t *)node->value;

	int32_t result;
	if (node_class->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_class->name);
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
		symbol_t *symbol_heritages;
		symbol_heritages = symbol_rpush(symbol, SYMBOL_TYPE_HERITAGES, NULL);

		for (a = node_class->heritages->begin; a != node_class->heritages->end; a = a->next)
		{
			node_t *heritage;
			heritage = (node_t *)a->value;

			result = graph_heritage(symbol_heritages, heritage);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_class->generics)
	{
		symbol_t *symbol_generics;
		symbol_generics = symbol_rpush(symbol, SYMBOL_TYPE_GENERICS, NULL);

		for (a = node_class->generics->begin; a != node_class->generics->end; a = a->next)
		{
			node_t *generic;
			generic = (node_t *)a->value;

			result = graph_generic(symbol_generics, generic);
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
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_FUNCTION, node);
	if(!symbol)
	{
		return 0;
	}

	node_func_t *node_func;
	node_func = (node_func_t *)node->value;

	int32_t result;
	if (node_func->name)
	{
		symbol_t *symbol_name;
		symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
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
	if (node_func->generics)
	{
		symbol_t *generics;
		generics = symbol_rpush(symbol, SYMBOL_TYPE_GENERICS, NULL);

		for (a = node_func->generics->begin; a != node_func->generics->end; a = a->next)
		{
			node_t *generic = (node_t *)a->value;
			result = graph_generic(generics, generic);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->heritages)
	{
		symbol_t *heritages;
		heritages = symbol_rpush(symbol, SYMBOL_TYPE_HERITAGES, NULL);

		ilist_t *a;
		for (a = node_func->heritages->begin; a != node_func->heritages->end; a = a->next)
		{
			node_t *heritage = (node_t *)a->value;

			result = graph_heritage(heritages, heritage);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->parameters)
	{
		symbol_t *parameters;
		parameters = symbol_rpush(symbol, SYMBOL_TYPE_PARAMETERS, NULL);

		for (a = node_func->parameters->begin; a != node_func->parameters->end; a = a->next)
		{
			node_t *parameter = (node_t *)a->value;

			result = graph_parameter(parameters, parameter);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->body)
	{
		result = graph_block(symbol, node_func->body);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_VAR, node);
	if(!symbol)
	{
		return 0;
	}

	symbol_t *symbol_name;
	symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_var->name);
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
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE_TYPE, node_var->type);
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
		symbol_value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, node_var->value);
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
graph_type(symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_TYPE, node);
	if(!symbol)
	{
		return 0;
	}

	node_type_t *node_type;
	node_type = (node_type_t *)node->value;

	symbol_t *symbol_name;
	symbol_name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, node_type->name);
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
	if (node_type->generics)
	{
		symbol_t *symbol_generics;
		symbol_generics = symbol_rpush(symbol, SYMBOL_TYPE_GENERICS, NULL);

		for (a = node_type->generics->begin; a != node_type->generics->end; a = a->next)
		{
			node_t *generic;
			generic = (node_t *)a->value;

			result = graph_generic(symbol_generics, generic);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_type->heritages)
	{
		symbol_t *symbol_heritages;
		symbol_heritages = symbol_rpush(symbol, SYMBOL_TYPE_HERITAGES, NULL);

		for (a = node_type->heritages->begin; a != node_type->heritages->end; a = a->next)
		{
			node_t *heritage;
			heritage = (node_t *)a->value;

			result = graph_heritage(symbol_heritages, heritage);
			if (!result)
			{
				return 0;
			}
		}
	}

	symbol_t *symbol_value;
	symbol_value = symbol_rpush(symbol, SYMBOL_TYPE_VALUE, node_type->body);
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
graph_field(symbol_t *parent, node_t *node)
{
	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_FIELD, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_field->name)
	{
		symbol_t *name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}

		result = graph_expression(name, node_field->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_field->type)
	{
		symbol_t *type = symbol_rpush(symbol, SYMBOL_TYPE_TYPE, NULL);
		if(!type)
		{
			return 0;
		}

		result = graph_expression(type, node_field->type);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
graph_import(symbol_t *parent, node_t *node)
{
	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TYPE_IMPORT, node);
	if (!symbol)
	{
		return 0;
	}

	int32_t result;
	if (node_import->name)
	{
		symbol_t *name;
		name = symbol_rpush(symbol, SYMBOL_TYPE_NAME, NULL);
		if(!name)
		{
			return 0;
		}

		result = graph_id(name, node_import->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_import->generics)
	{
		symbol_t *generics;
		generics = symbol_rpush(symbol, SYMBOL_TYPE_GENERICS, NULL);

		ilist_t *a;
		for (a = node_import->generics->begin; a != node_import->generics->end; a = a->next)
		{
			node_t *generic = (node_t *)a->value;
			result = graph_generic(generics, generic);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_import->path)
	{
		symbol_t *path = symbol_rpush(symbol, SYMBOL_TYPE_PATH, NULL);
		if(!path)
		{
			return 0;
		}

		result = graph_string(path, node_import->path);
		if (!result)
		{
			return 0;
		}
	}

	if (node_import->fields)
	{
		symbol_t *fields = symbol_rpush(symbol, SYMBOL_TYPE_FIELDS, NULL);
		if(!fields)
		{
			return 0;
		}
		ilist_t *a;
		for (a = node_import->fields->begin; a != node_import->fields->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			result = graph_field(fields, temp);
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
	symbol = symbol_rpush(parent, SYMBOL_TYPE_MODULE, node);
	if(!symbol)
	{
		return 0;
	}

	int32_t result = 1;
	ilist_t *a;
	for (a = node_module->members->begin; a != node_module->members->end; a = a->next)
	{
		node_t *temp = (node_t *)a->value;

		switch (temp->kind)
		{
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
graph_run(graph_t *graph, node_t *root)
{
	int32_t result = graph_module(graph->symbol, root);
	if(!result)
	{
		return 0;
	}
	return 1;
}

graph_t *
graph_create(program_t *program, list_t *errors)
{
	graph_t *graph = (graph_t *)malloc(sizeof(graph_t));
	if (!graph)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(graph_t));
		return NULL;
	}
	memset(graph, 0, sizeof(graph_t));

	graph->program = program;
	graph->errors = errors;

	symbol_t *symbol;
	symbol = symbol_create(SYMBOL_TYPE_NONE, NULL);
	if(!symbol)
	{
		return 0;
	}

	graph->symbol = symbol;

	return graph;
}
