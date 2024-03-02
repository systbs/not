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

static error_t *
graph_error(program_t *program, node_t *node, const char *format, ...)
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

	error_t *error;
	error = error_create(node->position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(program->errors, error))
	{
		return NULL;
	}

	return error;
}


static int32_t
graph_expression(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_expression(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_func(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_parameter(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_generic(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_postfix(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_block(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_var(program_t *program, symbol_t *parent, node_t *node);

static int32_t
graph_prefix(program_t *program, symbol_t *parent, node_t *node);


static int32_t
graph_id(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_ID, node);
	if(symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_number(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_NUMBER, node);
	if (symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_string(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_STRING, node);
	if (symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_char(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;

	symbol = symbol_rpush(parent, SYMBOL_CHAR, node);
	if (symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_null(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_NULL, node);
	if (symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_true(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_TRUE, node);
	if (symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_false(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	
	symbol = symbol_rpush(parent, SYMBOL_FALSE, node);
	if (symbol)
	{
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_array(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_ARRAY, node);
	if (symbol)
	{
		node_array_t *node_array;
		node_array = (node_array_t *)node->value;

		int32_t result;
		ilist_t *a;
		for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = graph_expression(program, symbol, temp);
			if (result == -1)
			{
				return -1;
			}
		}
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_parenthesis(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_PARENTHESIS, node);
	if(symbol)
	{
		node_unary_t *node_unary;
		node_unary = (node_unary_t *)node->value;

		int32_t result;
		result = graph_expression(program, symbol, node_unary->right);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_lambda(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_LAMBDA, node);
	if(symbol)
	{
		node_lambda_t *lambda = (node_lambda_t *)node->value;
		int32_t result;
		if (lambda->parameters)
		{
			symbol_t *parameters;
			parameters = symbol_rpush(symbol, SYMBOL_PARAMETERS, NULL);
			if (parameters)
			{
				ilist_t *a;
				for (a = lambda->parameters->begin; a != lambda->parameters->end; a = a->next)
				{
					node_t *parameter = (node_t *)a->value;

					result = graph_parameter(program, parameters, parameter);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}

		if (lambda->body)
		{
			if(lambda->body->kind == NODE_KIND_BLOCK)
			{
				result = graph_block(program, symbol, lambda->body);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				symbol_t *value = symbol_rpush(symbol, SYMBOL_VALUE, NULL);
				if(value)
				{
					result = graph_expression(program, value, lambda->body);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					fprintf(stderr, "unable to allocate memory");
					return -1;
				}
			}
		}

		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_member(program_t *program, symbol_t *parent, node_t *node)
{
	node_member_t *node_member = (node_member_t *)node->value;

	symbol_t *symbol = symbol_rpush(parent, SYMBOL_MEMBER, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_member->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}
		
		result = graph_id(program, key, node_member->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_member->value)
	{
		symbol_t *value;
		value = symbol_rpush(symbol, SYMBOL_VALUE, NULL);
		if(!value)
		{
			return -1;
		}

		result = graph_expression(program, value, node_member->value);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_object(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_OBJECT, node);

	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	if(symbol == NULL)
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *member = (node_t *)a->value;
		result = graph_member(program, symbol, member);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_primary(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_ID)
	{
		int32_t result;
		result = graph_id(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_NUMBER)
	{
		int32_t result;
		result = graph_number(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_STRING)
	{
		int32_t result;
		result = graph_string(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_CHAR)
	{
		int32_t result;
		result = graph_char(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_NULL)
	{
		int32_t result;
		result = graph_null(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_TRUE)
	{
		int32_t result;
		result = graph_true(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_FALSE)
	{
		int32_t result;
		result = graph_false(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_ARRAY)
	{
		int32_t result;
		result = graph_array(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_OBJECT)
	{
		int32_t result;
		result = graph_object(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_LAMBDA)
	{
		int32_t result;
		result = graph_lambda(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_PARENTHESIS)
	{
		int32_t result;
		result = graph_parenthesis(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	{
		graph_error(program, node, "primary node");
		return -1;
	}
}

static int32_t
graph_argument(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_ARGUMENT, node);
	if(symbol)
	{
		node_argument_t *node_argument = (node_argument_t *)node->value;

		int32_t result;
		if (node_argument->key)
		{
			symbol_t *key;
			key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
			if(key)
			{
				result = graph_expression(program, key, node_argument->key);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}	
		}

		if (node_argument->value)
		{
			symbol_t *value;
			value = symbol_rpush(symbol, SYMBOL_VALUE, NULL);
			if(value)
			{
				result = graph_expression(program, value, node_argument->value);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}

		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_composite(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_COMPOSITE, node);
	if (symbol)
	{
		node_composite_t *node_composite;
		node_composite = (node_composite_t *)node->value;

		int32_t result;
		if (node_composite->base)
		{
			symbol_t *base = symbol_rpush(symbol, SYMBOL_KEY, NULL);
			if(base)
			{
				result = graph_expression(program, base, node_composite->base);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}

		if (node_composite->arguments)
		{
			symbol_t *arguments = symbol_rpush(symbol, SYMBOL_ARGUMENTS, NULL);
			if(arguments)
			{
				ilist_t *a;
				for (a = node_composite->arguments->begin; a != node_composite->arguments->end; a = a->next)
				{
					node_t *argument = (node_t *)a->value;

					result = graph_argument(program, arguments, argument);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}

		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_call(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_CALL, node);
	if(symbol)
	{
		node_composite_t *node_composite;
		node_composite = (node_composite_t *)node->value;

		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(key)
		{
			int32_t result;
			result = graph_postfix(program, key, node_composite->base);
			if (result == -1)
			{
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}

		if (node_composite->arguments)
		{
			symbol_t *arguments = symbol_rpush(symbol, SYMBOL_ARGUMENTS, NULL);
			if(arguments)
			{
				ilist_t *a;
				for (a = node_composite->arguments->begin; a != node_composite->arguments->end; a = a->next)
				{
					node_t *argument = (node_t *)a->value;
					int32_t result;
					result = graph_argument(program, arguments, argument);
					if (result == -1)
					{
						return -1;
					}
				}
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}

		return 1;
	}
	else 
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_item(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_ITEM, node);
	if(symbol)
	{
		node_composite_t *node_composite;
		node_composite = (node_composite_t *)node->value;
		
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(key)
		{
			int32_t result;
			result = graph_postfix(program, key, node_composite->base);
			if (result == -1)
			{
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}

		if (node_composite->arguments)
		{
			symbol_t *arguments = symbol_rpush(symbol, SYMBOL_ARGUMENTS, NULL);
			if(arguments)
			{
				ilist_t *a;
				for (a = node_composite->arguments->begin; a != node_composite->arguments->end; a = a->next)
				{
					node_t *argument = (node_t *)a->value;
					int32_t result;
					result = graph_argument(program, arguments, argument);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}

		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_attribute(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_ATTR, node);
	if(symbol)
	{
		node_binary_t *node_binary;
		node_binary = (node_binary_t *)node->value;
		
		int32_t result;
		if(node_binary->left)
		{
			symbol_t *left;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				result = graph_postfix(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}

		if(node_binary->right)
		{
			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				result = graph_id(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;	
			}
		}
		return 1;
	}
	else
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}
}

static int32_t
graph_postfix(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_COMPOSITE)
	{
		int32_t result;
		result = graph_composite(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_CALL)
	{
		int32_t result;
		result = graph_call(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_GET_ITEM)
	{
		int32_t result;
		result = graph_item(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	if (node->kind == NODE_KIND_ATTRIBUTE)
	{
		int32_t result;
		result = graph_attribute(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
	else
	{
		int32_t result;
		result = graph_primary(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_prefix(program_t *program, symbol_t *parent, node_t *node)
{
	if(node->kind == NODE_KIND_TILDE)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_TILDE, node);

		if (symbol)
		{
			node_unary_t *node_unary;
			node_unary = (node_unary_t *)node->value;

			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_unary->right);
				if (result == -1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_NOT)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_NOT, node);
		if (symbol)
		{
			node_unary_t *node_unary;
			node_unary = (node_unary_t *)node->value;

			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_unary->right);
				if (result == -1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_NEG)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_NEG, node);

		if (symbol)
		{
			node_unary_t *node_unary;
			node_unary = (node_unary_t *)node->value;

			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_unary->right);
				if (result == -1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_POS)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_POS, node);

		if (symbol)
		{
			node_unary_t *node_unary;
			node_unary = (node_unary_t *)node->value;

			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_unary->right);
				if (result == -1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_SIZEOF)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_SIZEOF, node);

		if (symbol)
		{
			node_unary_t *node_unary;
			node_unary = (node_unary_t *)node->value;

			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_unary->right);
				if (result == -1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_TYPEOF)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_TYPEOF, node);

		if (symbol)
		{
			node_unary_t *node_unary;
			node_unary = (node_unary_t *)node->value;

			symbol_t *right;
			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_unary->right);
				if (result == -1)
				{
					return -1;
				}

				return 1;
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_postfix(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_power(program_t *program, symbol_t *parent, node_t *node)
{
	
	if(node->kind == NODE_KIND_POW)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_POW, node);
		if (symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_power(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_prefix(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_prefix(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_multiplicative(program_t *program, symbol_t *parent, node_t *node)
{
	if(node->kind == NODE_KIND_MUL)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_MUL, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_multiplicative(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_power(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_DIV)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_DIV, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_multiplicative(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_power(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_MOD)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_MOD, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_multiplicative(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_power(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_EPI)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_EPI, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_multiplicative(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_power(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_power(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_addative(program_t *program, symbol_t *parent, node_t *node)
{
	if(node->kind == NODE_KIND_PLUS)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_PLUS, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_addative(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_multiplicative(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	if(node->kind == NODE_KIND_MINUS)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_MINUS, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_addative(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_multiplicative(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_multiplicative(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_shifting(program_t *program, symbol_t *parent, node_t *node)
{
	if(node->kind == NODE_KIND_SHL)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_SHL, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_shifting(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_addative(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_SHR)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_SHR, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_shifting(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_addative(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_addative(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_relational(program_t *program, symbol_t *parent, node_t *node)
{
	if(node->kind == NODE_KIND_LT)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_LT, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_relational(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_shifting(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_GT)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_GT, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_relational(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_shifting(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_LE)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_LE, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_relational(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_shifting(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_GE)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_GE, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_relational(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_shifting(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_shifting(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_equality(program_t *program, symbol_t *parent, node_t *node)
{
	if(node->kind == NODE_KIND_EQ)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_EQ, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_equality(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_relational(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else 
	if(node->kind == NODE_KIND_NEQ)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_NEQ, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_equality(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_relational(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_relational(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_bitwise_and(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_AND)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_AND, node);
		if (symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				if (node_binary->left->kind == NODE_KIND_AND)
				{
					int32_t result;
					result = graph_bitwise_and(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = graph_equality(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_equality(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_equality(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_bitwise_xor(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_XOR)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_XOR, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left,  *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				if (node_binary->left->kind == NODE_KIND_XOR)
				{
					int32_t result;
					result = graph_bitwise_xor(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = graph_bitwise_and(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_bitwise_and(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_bitwise_and(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_bitwise_or(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_OR)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_OR, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left,  *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				if (node_binary->left->kind == NODE_KIND_OR)
				{
					int32_t result;
					result = graph_bitwise_or(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = graph_bitwise_xor(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_bitwise_xor(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_bitwise_xor(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_logical_and(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_LAND)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_LAND, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left,  *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				if (node_binary->left->kind == NODE_KIND_LAND)
				{
					int32_t result;
					result = graph_logical_and(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = graph_bitwise_or(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_bitwise_or(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_bitwise_or(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_logical_or(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_LOR)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_LOR, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left,  *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				if (node_binary->left->kind == NODE_KIND_LOR)
				{
					int32_t result;
					result = graph_logical_or(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = graph_logical_and(program, left, node_binary->left);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_logical_and(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	else
	{
		int32_t result;
		result = graph_logical_and(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_conditional(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_CONDITIONAL)
	{
		node_triple_t *node_triple;
		node_triple = (node_triple_t *)node->value;

		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_CONDITIONAL, node);
		if(symbol)
		{
			symbol_t *condition;
			condition = symbol_rpush(symbol, SYMBOL_CONDITION, NULL);
			if(condition)
			{
				int32_t result;
				result = graph_logical_or(program, condition, node_triple->base);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_conditional(program, left, node_triple->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_conditional(program, right, node_triple->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}

		return 1;
	}
	else
	{
		int32_t result;
		result = graph_logical_or(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_expression(program_t *program, symbol_t *parent, node_t *node)
{
	return graph_conditional(program, parent, node);
}

static int32_t
graph_assign(program_t *program, symbol_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_ADD_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_ADD_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_SUB_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_SUB_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_DIV_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_DIV_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_MUL_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_MUL_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_MOD_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_MOD_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_AND_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_AND_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_OR_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_OR_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_SHL_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_SHL_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	else 
	if (node->kind == NODE_KIND_SHR_ASSIGN)
	{
		symbol_t *symbol;
		symbol = symbol_rpush(parent, SYMBOL_SHR_ASSIGN, node);
		if(symbol)
		{
			node_binary_t *node_binary;
			node_binary = (node_binary_t *)node->value;

			symbol_t *left, *right;
			left = symbol_rpush(symbol, SYMBOL_LEFT, NULL);
			if(left)
			{
				int32_t result;
				result = graph_expression(program, left, node_binary->left);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}

			right = symbol_rpush(symbol, SYMBOL_RIGHT, NULL);
			if(right)
			{
				int32_t result;
				result = graph_expression(program, right, node_binary->right);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				fprintf(stderr, "unable to allocate memory");
				return -1;
			}
			return 1;
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}
	
	else
	{
		int32_t result;
		result = graph_expression(program, parent, node);
		if (result == -1)
		{
			return -1;
		}
		return 1;
	}
}

static int32_t
graph_if(program_t *program, symbol_t *parent, node_t *node)
{
	node_if_t *node_if;
	node_if = (node_if_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_IF, node);
	if (!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_if->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}
		result = graph_id(program, key, node_if->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_if->condition)
	{
		symbol_t *symbol_condition;
		symbol_condition = symbol_rpush(symbol, SYMBOL_CONDITION, node_if->condition);
		if(!symbol_condition)
		{
			return -1;
		}
		
		result = graph_expression(program, symbol_condition, node_if->condition);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_if->then_body)
	{
		result = graph_block(program, symbol, node_if->then_body);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_if->else_body)
	{
		symbol_t *symbol_else;
		symbol_else = symbol_rpush(symbol, SYMBOL_ELSE, node_if->else_body);
		if(!symbol_else)
		{
			return -1;
		}

		if(node_if->else_body->kind == NODE_KIND_IF)
		{
			result = graph_if(program, symbol_else, node_if->else_body);
		} 
		else 
		{
			result = graph_block(program, symbol_else, node_if->else_body);
		}

		if (result == -1) {
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_catch(program_t *program, symbol_t *parent, node_t *node)
{
	node_catch_t *node_catch;
	node_catch = (node_catch_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_CATCH, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_catch->parameters)
	{
		symbol_t *parameters;
		parameters = symbol_rpush(symbol, SYMBOL_PARAMETERS, NULL);

		ilist_t *a;
		for (a = node_catch->parameters->begin; a != node_catch->parameters->end; a = a->next)
		{
			node_t *parameter = (node_t *)a->value;

			result = graph_parameter(program, parameters, parameter);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	if (node_catch->body)
	{
		result = graph_block(program, symbol, node_catch->body);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_try(program_t *program, symbol_t *parent, node_t *node)
{
	node_try_t *node_try;
	node_try = (node_try_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_TRY, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	result = graph_block(program, symbol, node_try->body);
	if (result == -1)
	{
		return -1;
	}

	if (node_try->catchs)
	{
		symbol_t *catchs;
		catchs = symbol_rpush(symbol, SYMBOL_CATCHS, NULL);
		if(!catchs)
		{
			return -1;
		}

		ilist_t *a;
		for (a = node_try->catchs->begin; a != node_try->catchs->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;
			result = graph_catch(program, catchs, temp);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
graph_for(program_t *program, symbol_t *parent, node_t *node)
{
	node_for_t *node_for;
	node_for = (node_for_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FOR, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_for->key)
	{
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}
		result = graph_id(program, key, node_for->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_for->initializer)
	{
		symbol_t *initializer;
		initializer = symbol_rpush(symbol, SYMBOL_INITIALIZER, NULL);
		if(!initializer)
		{
			return -1;
		}

		ilist_t *a;
		for (a = node_for->initializer->begin; a != node_for->initializer->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			if (temp->kind == NODE_KIND_VAR)
			{
				result = graph_var(program, initializer, temp);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				result = graph_assign(program, initializer, temp);
				if (result == -1)
				{
					return -1;
				}
			}
		}
	}

	if (node_for->condition)
	{
		symbol_t *symbol_condition;
		symbol_condition = symbol_rpush(symbol, SYMBOL_CONDITION, NULL);
		if(!symbol_condition)
		{
			return -1;
		}

		result = graph_expression(program, symbol_condition, node_for->condition);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_for->incrementor)
	{
		symbol_t *incrementor;
		incrementor = symbol_rpush(symbol, SYMBOL_INCREMENTOR, NULL);
		if(!incrementor)
		{
			return -1;
		}

		ilist_t *a;
		for (a = node_for->incrementor->begin; a != node_for->incrementor->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			result = graph_assign(program, incrementor, temp);
			if (result == -1)
			{
				return -1;
			}
		}
	}
	
	if (node_for->body)
	{
		result = graph_block(program, symbol, node_for->body);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_forin(program_t *program, symbol_t *parent, node_t *node)
{
	node_forin_t *node_forin;
	node_forin = (node_forin_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FORIN, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_forin->key)
	{
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}
		result = graph_id(program, key, node_forin->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_forin->initializer)
	{
		symbol_t *initializer;
		initializer = symbol_rpush(symbol, SYMBOL_INITIALIZER, NULL);
		if(!initializer)
		{
			return -1;
		}

		ilist_t *a;
		for (a = node_forin->initializer->begin; a != node_forin->initializer->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			if (temp->kind == NODE_KIND_VAR)
			{
				result = graph_var(program, initializer, temp);
				if (result == -1)
				{
					return -1;
				}
			}
			else
			{
				result = graph_assign(program, initializer, temp);
				if (result == -1)
				{
					return -1;
				}
			}
		}
	}

	if (node_forin->expression)
	{
		symbol_t *expression;
		expression = symbol_rpush(symbol, SYMBOL_EXPRESSION, NULL);
		if(!expression)
		{
			return -1;
		}

		result = graph_expression(program, expression, node_forin->expression);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_forin->body)
	{
		result = graph_block(program, symbol, node_forin->body);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_break(program_t *program, symbol_t *parent, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_BREAK, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result = graph_expression(program, symbol, node_unary->right);
	if (result == -1)
	{
		return -1;
	}

	return 1;
}

static int32_t
graph_continue(program_t *program, symbol_t *parent, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_CONTINUE, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result = graph_expression(program, symbol, node_unary->right);
	if (result == -1)
	{
		return -1;
	}

	return 1;
}

static int32_t
graph_return(program_t *program, symbol_t *parent, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_RETURN, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result = graph_expression(program, symbol, node_unary->right);
	if (result == -1)
	{
		return -1;
	}

	return 1;
}

static int32_t
graph_throw(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_THROW, node);
	if(!symbol)
	{
		return -1;
	}

	node_throw_t *node_throw;
	node_throw = (node_throw_t *)node->value;

	if (node_throw->arguments)
	{
		symbol_t *arguments = symbol_rpush(symbol, SYMBOL_ARGUMENTS, NULL);
		if(!arguments)
		{
			return -1;
		}

		ilist_t *a;
		for (a = node_throw->arguments->begin; a != node_throw->arguments->end; a = a->next)
		{
			node_t *argument = (node_t *)a->value;

			int32_t result;
			result = graph_argument(program, arguments, argument);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
graph_statement(program_t *program, symbol_t *parent, node_t *node)
{
	int32_t result = 1;
	switch (node->kind)
	{
	case NODE_KIND_BLOCK:
		result = graph_block(program, parent, node);
		break;

	case NODE_KIND_IF:
		result = graph_if(program, parent, node);
		break;

	case NODE_KIND_TRY:
		result = graph_try(program, parent, node);
		break;

	case NODE_KIND_FOR:
		result = graph_for(program, parent, node);
		break;

	case NODE_KIND_FORIN:
		result = graph_forin(program, parent, node);
		break;

	case NODE_KIND_VAR:
		result = graph_var(program, parent, node);
		break;

	case NODE_KIND_BREAK:
		result = graph_break(program, parent, node);
		break;

	case NODE_KIND_CONTINUE:
		result = graph_continue(program, parent, node);
		break;

	case NODE_KIND_RETURN:
		result = graph_return(program, parent, node);
		break;

	case NODE_KIND_THROW:
		result = graph_throw(program, parent, node);
		break;

	case NODE_KIND_FUNC:
		result = graph_func(program, parent, node);
		break;

	default:
		result = graph_assign(program, parent, node);
		break;
	}

	return result;
}

static int32_t
graph_block(program_t *program, symbol_t *parent, node_t *node)
{
	node_block_t *node_block;
	node_block = (node_block_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_BLOCK, node);
	if(!symbol)
	{
		return -1;
	}

	ilist_t *a;
	for (a = node_block->list->begin; a != node_block->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result = graph_statement(program, symbol, temp);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_generic(program_t *program, symbol_t *parent, node_t *node)
{
	node_generic_t *node_generic;
	node_generic = (node_generic_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_GENERIC, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;

	if (node_generic->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_id(program, key, node_generic->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_generic->type)
	{
		symbol_t *type = symbol_rpush(symbol, SYMBOL_TYPE, NULL);
		if(!type)
		{
			return -1;
		}

		result = graph_expression(program, type, node_generic->type);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_generic->value)
	{
		symbol_t *value = symbol_rpush(symbol, SYMBOL_VALUE, NULL);
		if(!value)
		{
			return -1;
		}

		result = graph_expression(program, value, node_generic->value);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_enum(program_t *program, symbol_t *parent, node_t *node)
{
	node_enum_t *node_enum;
	node_enum = (node_enum_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_ENUM, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_enum->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_id(program, key, node_enum->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_enum->body)
	{
		symbol_t *members = symbol_rpush(symbol, SYMBOL_MEMBERS, NULL);
		if(!members)
		{
			return -1;
		}
		ilist_t *a;
		for (a = node_enum->body->begin; a != node_enum->body->end; a = a->next)
		{
			node_t *member = (node_t *)a->value;

			result = graph_member(program, members, member);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
graph_property(program_t *program, symbol_t *parent, node_t *node)
{
	node_property_t *node_property;
	node_property = (node_property_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_PROPERTY, node);
	if (!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_property->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_id(program, key, node_property->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_property->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE, node_property->type);
		if(!symbol_type)
		{
			return -1;
		}

		result = graph_expression(program, symbol_type, node_property->type);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_property->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_VALUE, node_property->value);
		if(!symbol_value)
		{
			return -1;
		}

		result = graph_expression(program, symbol_value, node_property->value);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_parameter(program_t *program, symbol_t *parent, node_t *node)
{
	node_parameter_t *node_parameter;
	node_parameter = (node_parameter_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_PARAMETER, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_parameter->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}
		
		result = graph_id(program, key, node_parameter->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_parameter->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE, node_parameter->type);
		if(!symbol_type)
		{
			return -1;
		}

		result = graph_expression(program, symbol_type, node_parameter->type);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_parameter->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_VALUE, node_parameter->value);
		if(!symbol_value)
		{
			return -1;
		}

		result = graph_expression(program, symbol_value, node_parameter->value);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_heritage(program_t *program, symbol_t *parent, node_t *node)
{
	node_heritage_t *node_heritage;
	node_heritage = (node_heritage_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_HERITAGE, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_heritage->key)
	{
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_id(program, key, node_heritage->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_heritage->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE, node);
		if(!symbol_type)
		{
			return -1;
		}

		int32_t result = graph_expression(program, symbol_type, node_heritage->type);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_func(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FUNCTION, node);
	if(!symbol)
	{
		fprintf(stderr, "unable to allocate memory");
		return -1;
	}

	node_func_t *node_func;
	node_func = (node_func_t *)node->value;

	int32_t result;
	if (node_func->key)
	{
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(key)
		{
			result = graph_id(program, key, node_func->key);
			if (result == -1)
			{
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	ilist_t *a;
	if (node_func->generics)
	{
		symbol_t *generics;
		generics = symbol_rpush(symbol, SYMBOL_GENERICS, NULL);
		if (generics)
		{
			for (a = node_func->generics->begin; a != node_func->generics->end; a = a->next)
			{
				node_t *generic = (node_t *)a->value;
				result = graph_generic(program, generics, generic);
				if (result == -1)
				{
					return -1;
				}
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	if (node_func->parameters)
	{
		symbol_t *parameters;
		parameters = symbol_rpush(symbol, SYMBOL_PARAMETERS, NULL);
		if (parameters)
		{
			for (a = node_func->parameters->begin; a != node_func->parameters->end; a = a->next)
			{
				node_t *parameter = (node_t *)a->value;
				result = graph_parameter(program, parameters, parameter);
				if (result == -1)
				{
					return -1;
				}
			}
		}
		else
		{
			fprintf(stderr, "unable to allocate memory");
			return -1;
		}
	}

	if (node_func->body)
	{
		result = graph_block(program, symbol, node_func->body);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_class(program_t *program, symbol_t *parent, node_t *node)
{
	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_CLASS, node);
	if(!symbol)
	{
		return -1;
	}

	node_class_t *node_class;
	node_class = (node_class_t *)node->value;

	int32_t result;
	if (node_class->key)
	{
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_id(program, key, node_class->key);
		if (result == -1)
		{
			return -1;
		}
	}

	ilist_t *a;
	if (node_class->heritages)
	{
		symbol_t *symbol_heritages;
		symbol_heritages = symbol_rpush(symbol, SYMBOL_HERITAGES, NULL);

		for (a = node_class->heritages->begin; a != node_class->heritages->end; a = a->next)
		{
			node_t *heritage;
			heritage = (node_t *)a->value;

			result = graph_heritage(program, symbol_heritages, heritage);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	if (node_class->generics)
	{
		symbol_t *symbol_generics;
		symbol_generics = symbol_rpush(symbol, SYMBOL_GENERICS, NULL);

		for (a = node_class->generics->begin; a != node_class->generics->end; a = a->next)
		{
			node_t *generic;
			generic = (node_t *)a->value;

			result = graph_generic(program, symbol_generics, generic);
			if (result == -1)
			{
				return -1;
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
			result = graph_class(program, symbol, temp);
			break;

		case NODE_KIND_ENUM:
			result = graph_enum(program, symbol, temp);
			break;

		case NODE_KIND_PROPERTY:
			result = graph_property(program, symbol, temp);
			break;

		case NODE_KIND_FUNC:
			result = graph_func(program, symbol, temp);
			break;

		default:
			return -1;
		}

		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_var(program_t *program, symbol_t *parent, node_t *node)
{
	node_var_t *node_var;
	node_var = (node_var_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_VAR, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_var->key)
	{
		symbol_t *key;
		key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_id(program, key, node_var->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_var->type)
	{
		symbol_t *symbol_type;
		symbol_type = symbol_rpush(symbol, SYMBOL_TYPE, node_var->type);
		if(!symbol_type)
		{
			return -1;
		}

		result = graph_expression(program, symbol_type, node_var->type);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_var->value)
	{
		symbol_t *symbol_value;
		symbol_value = symbol_rpush(symbol, SYMBOL_VALUE, node_var->value);
		if(!symbol_value)
		{
			return -1;
		}

		result = graph_expression(program, symbol_value, node_var->value);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_field(program_t *program, symbol_t *parent, node_t *node)
{
	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_FIELD, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result;
	if (node_field->key)
	{
		symbol_t *key = symbol_rpush(symbol, SYMBOL_KEY, NULL);
		if(!key)
		{
			return -1;
		}

		result = graph_expression(program, key, node_field->key);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_field->type)
	{
		symbol_t *type = symbol_rpush(symbol, SYMBOL_TYPE, NULL);
		if(!type)
		{
			return -1;
		}

		result = graph_expression(program, type, node_field->type);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
graph_import(program_t *program, symbol_t *parent, node_t *node)
{
	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_IMPORT, node);
	if (!symbol)
	{
		return -1;
	}

	int32_t result;

	if (node_import->path)
	{
		symbol_t *path = symbol_rpush(symbol, SYMBOL_PATH, NULL);
		if(!path)
		{
			return -1;
		}

		result = graph_string(program, path, node_import->path);
		if (result == -1)
		{
			return -1;
		}
	}

	if (node_import->fields)
	{
		symbol_t *fields = symbol_rpush(symbol, SYMBOL_FIELDS, NULL);
		if(!fields)
		{
			return -1;
		}
		ilist_t *a;
		for (a = node_import->fields->begin; a != node_import->fields->end; a = a->next)
		{
			node_t *temp = (node_t *)a->value;

			result = graph_field(program, fields, temp);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
graph_module(program_t *program, symbol_t *parent, node_t *node)
{	
	node_module_t *node_module;
	node_module = (node_module_t *)node->value;

	symbol_t *symbol;
	symbol = symbol_rpush(parent, SYMBOL_MODULE, node);
	if(!symbol)
	{
		return -1;
	}

	int32_t result = 1;
	ilist_t *a;
	for (a = node_module->members->begin; a != node_module->members->end; a = a->next)
	{
		node_t *temp = (node_t *)a->value;

		switch (temp->kind)
		{
		case NODE_KIND_IMPORT:
			result = graph_import(program, symbol, temp);
			break;

		case NODE_KIND_CLASS:
			result = graph_class(program, symbol, temp);
			break;

		case NODE_KIND_ENUM:
			result = graph_enum(program, symbol, temp);
			break;

		case NODE_KIND_FUNC:
			result = graph_func(program, symbol, temp);
			break;

		case NODE_KIND_VAR:
			result = graph_var(program, symbol, temp);
			break;

		default:
			return -1;
		}
	}

	return result;
}

int32_t
graph_run(program_t *program, symbol_t *root, node_t *node)
{
	int32_t result = graph_module(program, root, node);
	if(result == -1)
	{
		return -1;
	}
	return 1;
}