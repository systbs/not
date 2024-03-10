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
#include "parser.h"
#include "error.h"

static node_t *
parser_expression(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_statement(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_body(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_class(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
parser_func(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
parser_parameters(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_generics(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_parameter(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_property(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
parser_parenthesis(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_heritages(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_prefix(program_t *program, parser_t *parser, node_t *scope, node_t *parent);

static node_t *
parser_object(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
parser_export(program_t *program, parser_t *parser, node_t *scope, node_t *parent);



int32_t
parser_save(program_t *program, parser_t *parser)
{
	scanner_t *scanner;
	scanner = parser->scanner;

	parser_state_t *state;
	if (!(state = (parser_state_t *)malloc(sizeof(parser_state_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(parser_state_t));
		return -1;
	}

	state->loop_depth = parser->loop_depth;
	state->offset = scanner->offset;
	state->reading_offset = scanner->reading_offset;
	state->line = scanner->line;
	state->column = scanner->column;
	state->ch = scanner->ch;
	state->token = scanner->token;
	state->count_error = list_count(program->errors);

	if (!list_rpush(parser->states, state))
	{
		return -1;
	}

	return 1;
}

int32_t
parser_restore(program_t *program, parser_t *parser)
{
	scanner_t *scanner;
	scanner = parser->scanner;

	ilist_t *r1 = list_rpop(parser->states);
	if (r1 == NULL)
	{
		return -1;
	}

	parser_state_t *state;
	state = (parser_state_t *)r1->value;
	if (!state)
	{
		return -1;
	}

	parser->loop_depth = state->loop_depth;

	scanner->offset = state->offset;
	scanner->reading_offset = state->reading_offset;
	scanner->line = state->line;
	scanner->column = state->column;
	scanner->ch = state->ch;

	for(uint64_t i = state->count_error;i > 0;i--)
	{
		ilist_t *it;
		it = list_rpop(program->errors);
		if (it)
		{
			free((void *)it->value);
			free(it);
		}
	}

	scanner->token = state->token;
	free(state);
	return 1;
}

int32_t
parser_release(program_t *program, parser_t *parser)
{
	ilist_t *r1 = list_rpop(parser->states);
	if (r1 == NULL)
	{
		return -1;
	}

	parser_state_t *state;
	state = (parser_state_t *)r1->value;
	if (!state)
	{
		return -1;
	}

	free(state);
	return 1;
}

static error_t *
parser_error(program_t *program, position_t position, const char *format, ...)
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
	error = error_create(position, message);
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

static error_t *
parser_expected(program_t *program, parser_t *parser, int32_t type)
{
	char *message;
	message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	switch (parser->token->type)
	{
	case TOKEN_ID:
	case TOKEN_NUMBER:
	case TOKEN_CHAR:
	case TOKEN_STRING:
		sprintf(
				message,
				"expected '%s', got '%s'\n",
				token_get_name(type),
				parser->token->value);
		break;

	default:
		sprintf(
				message,
				"expected '%s', got '%s'\n",
				token_get_name(type),
				token_get_name(parser->token->type));
		break;
	}

	error_t *error;
	error = error_create(parser->token->position, message);
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
parser_match(program_t *program, parser_t *parser, int32_t type)
{
	if (parser->token->type == type)
	{
		if (scanner_advance(parser->scanner) == -1)
		{
			return -1;
		}
	}
	else
	{
		parser_expected(program, parser, type);
		return -1;
	}
	return 1;
}

static int32_t
parser_next(program_t *program, parser_t *parser)
{
	if (scanner_advance(parser->scanner) == -1)
	{
		return -1;
	}
	return 1;
}

static int32_t
parser_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return strcmp(nb1->value, name);
}




static node_t *
parser_id(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_id(node, parser->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_ID) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_number(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_number(node, parser->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_NUMBER) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_char(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_char(node, parser->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_CHAR) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_string(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_string(node, parser->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_STRING) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_null(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_null(node);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_NULL_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_true(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_true(node);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_TRUE_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_false(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_false(node);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_FALSE_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_infinity(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_infinity(node);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_INFINITY_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_this(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_this(node);
	if (!node2)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_THIS_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_operator(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	char *operator = NULL;
	if (parser->token->type == TOKEN_PLUS)
	{
		if (parser_match(program, parser, TOKEN_PLUS) == -1)
		{
			return NULL;
		}
		operator = "+";
	}
	else 
	if (parser->token->type == TOKEN_MINUS)
	{
		if (parser_match(program, parser, TOKEN_MINUS) == -1)
		{
			return NULL;
		}
		operator = "-";
	}
	else 
	if (parser->token->type == TOKEN_STAR)
	{
		if (parser_match(program, parser, TOKEN_STAR) == -1)
		{
			return NULL;
		}
		operator = "*";
	}
	else 
	if (parser->token->type == TOKEN_SLASH)
	{
		if (parser_match(program, parser, TOKEN_SLASH) == -1)
		{
			return NULL;
		}
		operator = "/";
	}
	else 
	if (parser->token->type == TOKEN_POWER)
	{
		if (parser_match(program, parser, TOKEN_POWER) == -1)
		{
			return NULL;
		}
		operator = "**";
	}
	else 
	if (parser->token->type == TOKEN_PERCENT)
	{
		if (parser_match(program, parser, TOKEN_PERCENT) == -1)
		{
			return NULL;
		}
		operator = "%";
	}
	else 
	if (parser->token->type == TOKEN_AND)
	{
		if (parser_match(program, parser, TOKEN_AND) == -1)
		{
			return NULL;
		}
		operator = "&";
	}
	else 
	if (parser->token->type == TOKEN_OR)
	{
		if (parser_match(program, parser, TOKEN_OR) == -1)
		{
			return NULL;
		}
		operator = "|";
	}
	else 
	if (parser->token->type == TOKEN_CARET)
	{
		if (parser_match(program, parser, TOKEN_CARET) == -1)
		{
			return NULL;
		}
		operator = "^";
	}
	else 
	if (parser->token->type == TOKEN_LT_LT)
	{
		if (parser_match(program, parser, TOKEN_LT_LT) == -1)
		{
			return NULL;
		}
		operator = "<<";
	}
	else 
	if (parser->token->type == TOKEN_GT_GT)
	{
		if (parser_match(program, parser, TOKEN_GT_GT) == -1)
		{
			return NULL;
		}
		operator = ">>";
	}
	else 
	if (parser->token->type == TOKEN_LT)
	{
		if (parser_match(program, parser, TOKEN_LT) == -1)
		{
			return NULL;
		}
		operator = "<";
	}
	else 
	if (parser->token->type == TOKEN_GT)
	{
		if (parser_match(program, parser, TOKEN_GT) == -1)
		{
			return NULL;
		}
		operator = ">";
	}
	else 
	if (parser->token->type == TOKEN_LBRACKET)
	{
		if (parser_match(program, parser, TOKEN_LBRACKET) == -1)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_RBRACKET) == -1)
		{
			return NULL;
		}
		operator = "[]";
	}
	else 
	{
		parser_error(program, parser->token->position, "operator expected\n");
		return NULL;
	}

	node_t *node2 = node_make_id(node, operator);
	if (!node2)
	{
		return NULL;
	}

	return node2;
}

static node_t *
parser_array(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LBRACKET) == -1)
	{
		return NULL;
	}

	list_t *expr_list;
	expr_list = list_create();
	if (!expr_list)
	{
		return NULL;
	}

	if (parser->token->type != TOKEN_RBRACKET)
	{
		while (true)
		{
			node_t *expr = parser_expression(program, parser, scope, node);
			if (!expr)
			{
				return NULL;
			}

			if (!list_rpush(expr_list, expr))
			{
				return NULL;
			}

			if (parser->token->type != TOKEN_COMMA)
			{
				break;
			}
			if (parser_match(program, parser, TOKEN_COMMA) == -1)
			{
				return NULL;
			}
		}
	}

	if (parser_match(program, parser, TOKEN_RBRACKET) == -1)
	{
		return NULL;
	}

	return node_make_array(node, expr_list);
}

static node_t *
parser_object(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	list_t *properties = list_create();
	if (!properties)
	{
		return NULL;
	}

	if (parser->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *property = parser_property(program, parser, scope, node, flag);
			if (!property)
			{
				return NULL;
			}

			if (!list_rpush(properties, property))
			{
				return NULL;
			}

			if (parser->token->type != TOKEN_COMMA)
			{
				break;
			}
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}
		}
	}

	if (parser_match(program, parser, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_object(node, properties);
}

static node_t *
parser_parenthesis(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	if(!parser_save(program, parser))
	{
		return NULL;
	}

	node_t *value = parser_expression(program, parser, scope, node);
	if (!value)
	{
		return NULL;
	}

	if (parser->token->type == TOKEN_COLON)
	{
		if(!parser_restore(program, parser))
		{
			return NULL;
		}
		goto region_lambda;
	}

	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	return node_make_parenthesis(node, value);

	node_t *parameters;
	region_lambda:
	parameters = parser_parameters(program, parser, scope, node);
	if (!parameters)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	node_t *body = NULL;
	if (parser->token->type == TOKEN_MINUS_GT)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		body = parser_expression(program, parser, scope, node);
		if (!body)
		{
			return NULL;
		}
	}
	else
	{
		body = parser_body(program, parser, scope, node);
		if (!body)
		{
			return NULL;
		}
	}	return node_make_lambda(node, parameters, body);
}

static node_t *
parser_primary(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	if (parser->token->type == TOKEN_ID)
	{
		return parser_id(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_NUMBER)
	{
		return parser_number(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_STRING)
	{
		return parser_string(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_CHAR)
	{
		return parser_char(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_NULL_KEYWORD)
	{
		return parser_null(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_TRUE_KEYWORD)
	{
		return parser_true(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_FALSE_KEYWORD)
	{
		return parser_false(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_INFINITY_KEYWORD)
	{
		return parser_infinity(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_THIS_KEYWORD)
	{
		return parser_this(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_LBRACKET)
	{
		return parser_array(program, parser, scope, parent);
	}
	else 
	if (parser->token->type == TOKEN_LBRACE)
	{
		return parser_object(program, parser, scope, parent, PARSER_MODIFIER_NONE);
	}
	else 
	if (parser->token->type == TOKEN_LPAREN)
	{
		return parser_parenthesis(program, parser, scope, parent);
	}
	else 
	{
		parser_error(program, parser->token->position, "expression expected %d\n", parser->token->type);
		return NULL;
	}
	return NULL;
}

static node_t *
parser_argument(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = parser_expression(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_argument(node, key, value);
}

static node_t *
parser_arguments(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *arguments = list_create();
	if (arguments == NULL)
	{
		return NULL;
	}

	node_t *node2;
	while (true)
	{
		node2 = parser_argument(program, parser, scope, parent);
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(arguments, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_arguments(node, arguments);
}

static node_t *
parser_datatype(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = parser_expression(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_datatype(node, key, value);
}

static node_t *
parser_datatypes(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *datatypes = list_create();
	if (datatypes == NULL)
	{
		return NULL;
	}

	node_t *node2;
	while (true)
	{
		node2 = parser_datatype(program, parser, scope, parent);
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(datatypes, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_datatypes(node, datatypes);
}

static node_t *
parser_postfix(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_primary(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}

	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_LBRACE)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			if (parser->token->type == TOKEN_RBRACE)
			{
				parser_error(program, parser->token->position, "empty arguments");
				return NULL;
			}

			node_t *arguments = parser_arguments(program, parser, scope, parent);
			if (arguments == NULL)
			{
				return NULL;
			}

			if (parser_match(program, parser, TOKEN_RBRACE) == -1)
			{
				return NULL;
			}

			node2 = node_make_composite(node, node2, arguments);
			continue;
		}
		else
		if (parser->token->type == TOKEN_LBRACKET)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *datatypes = NULL;
			if (parser->token->type != TOKEN_RBRACKET)
			{
				datatypes = parser_datatypes(program, parser, scope, parent);
				if (!(datatypes))
				{
					return NULL;
				}
			}

			if (parser_match(program, parser, TOKEN_RBRACKET) == -1)
			{
				return NULL;
			}

			node2 = node_make_item(node, node2, datatypes);
			continue;
		}
		else
		if (parser->token->type == TOKEN_DOT)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}
	
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_id(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}
      
			node2 = node_make_attribute(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_LPAREN)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}	

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *datatypes = NULL;
			if (parser->token->type != TOKEN_RPAREN)
			{
				datatypes = parser_datatypes(program, parser, scope, node);
				if (datatypes == NULL)
				{
					return NULL;
				}
			}

			if (parser_match(program, parser, TOKEN_RPAREN) == -1)
			{
				return NULL;
			}

			node2 = node_make_call(node, node2, datatypes);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_prefix(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	if (parser->token->type == TOKEN_TILDE)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		if (node == NULL)
		{
			return NULL;
		}
		
		

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_prefix(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_tilde(node, right);
	}
	else 
	if (parser->token->type == TOKEN_NOT)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		
		node_t *right = parser_prefix(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_not(node, right);
	}
	else 
	if (parser->token->type == TOKEN_MINUS)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		
		node_t *right = parser_prefix(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_neg(node, right);
	}
	else 
	if (parser->token->type == TOKEN_PLUS)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		
		node_t *right = parser_prefix(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_pos(node, right);
	}
	else 
	if (parser->token->type == TOKEN_SIZEOF_KEYWORD)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		
		node_t *right = parser_prefix(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_sizeof(node, right);
	}
	else 
	if (parser->token->type == TOKEN_TYPEOF_KEYWORD)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		
		node_t *right = parser_prefix(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_typeof(node, right);
	}
	else
	{
		return parser_postfix(program, parser, scope, parent);
	}
}

static node_t *
parser_power(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_prefix(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_POWER)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_prefix(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_pow(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_multiplicative(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_power(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_STAR)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_power(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_mul(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_SLASH)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			
			
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_power(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_div(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_PERCENT)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			
			
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_power(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_mod(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_BACKSLASH)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			
			
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_power(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_epi(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_addative(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_multiplicative(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}	
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_PLUS)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_multiplicative(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_plus(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_MINUS)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_multiplicative(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_minus(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_shifting(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_addative(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_LT_LT)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_addative(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_shl(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_GT_GT)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_addative(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_shr(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_relational(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_shifting(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_LT)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_shifting(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_lt(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_LT_EQ)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_shifting(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_le(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_GT)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_shifting(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_gt(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_GT_EQ)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_shifting(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_ge(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_equality(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_relational(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_EQ_EQ)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_relational(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_eq(node, node2, right);
			continue;
		}
		else
		if (parser->token->type == TOKEN_NOT_EQ)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_relational(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_neq(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_bitwise_and(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_equality(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_AND)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_equality(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_and(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_bitwise_xor(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_bitwise_and(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_CARET)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_bitwise_and(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_xor(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_bitwise_or(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_bitwise_xor(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_OR)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_bitwise_xor(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_or(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_logical_and(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_bitwise_or(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_AND_AND)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			node_t *right = parser_bitwise_or(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_land(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_logical_or(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_logical_and(program, parser, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (parser->token->type == TOKEN_OR_OR)
		{
			node_t *node = node_create(scope, parent, parser->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}
			
			node_t *right = parser_logical_and(program, parser, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_lor(node, node2, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static node_t *
parser_conditional(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_logical_or(program, parser, scope, parent);
	if (!node2)
	{
		return NULL;
	}

	if (parser->token->type == TOKEN_QUESTION)
	{
		node_t *node = node_create(scope, parent, parser->token->position);

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *true_expression = parser_conditional(program, parser, scope, node);
		if (!true_expression)
		{
			return NULL;
		}

		if (parser_match(program, parser, TOKEN_COLON) == -1)
		{
			return NULL;
		}

		node_t *false_expression = parser_conditional(program, parser, scope, node);
		if (!false_expression)
		{
			return NULL;
		}

		node2 = node_make_conditional(node, node2, true_expression, false_expression);
	}

	return node2;
}

static node_t *
parser_expression(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	return parser_conditional(program, parser, scope, parent);
}

static node_t *
parser_assign(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node2 = parser_expression(program, parser, scope, parent);
	if (!node2)
	{
		return NULL;
	}

	if (parser->token->type == TOKEN_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_PLUS_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_add_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_MINUS_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_sub_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_STAR_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_mul_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_SLASH_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_div_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_PERCENT_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_mod_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_AND_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_and_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_OR_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_or_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_LT_LT_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_shl_assign(node, node2, right);
	}
	else
	if (parser->token->type == TOKEN_GT_GT_EQ)
	{
		node_t *node = node_create(scope, parent, parser->token->position);
		
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		node_t *right = parser_expression(program, parser, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_shr_assign(node, node2, right);
	}

	return node2;
}

static node_t *
parser_if(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_IF_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser, scope, node);
		if (!key)
		{
			return NULL;
		}
	}

	if (parser_match(program, parser, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	node_t *condition;
	condition = parser_expression(program, parser, scope, node);
	if (!condition)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	node_t *then_body;
	then_body = parser_body(program, parser, scope, node);
	if (!then_body)
	{
		return NULL;
	}

	node_t *else_body;
	else_body = NULL;
	if (parser->token->type == TOKEN_ELSE_KEYWORD)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_IF_KEYWORD)
		{
			else_body = parser_if(program, parser, scope, node);
		}
		else
		{
			else_body = parser_body(program, parser, scope, node);
		}

		if (!else_body)
		{
			return NULL;
		}
	}

	return node_make_if(node, key, condition, then_body, else_body);
}

static node_t *
parser_var(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_VAR_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t object_used = 0;
	node_t *key = NULL;
	if (parser->token->type != TOKEN_ID)
	{
		key = parser_object(program, parser, scope, node, flag);
		if (!key)
		{
			return NULL;
		}
		object_used = 1;
	}
	else
	{
		key = parser_id(program, parser, scope, node);
		if (!key)
		{
			return NULL;
		}
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		if (object_used == 1)
		{
			parser_error(program, parser->token->position, "multiple variables with type");
			return NULL;
		}
		type = parser_expression(program, parser, scope, node);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_var(node, flag, key, type, value);
}

static node_t *
parser_for(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_next(program, parser) == -1)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_FOR_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser, scope, node);
		if (!key)
		{
			return NULL;
		}
	}

	list_t *initializer = list_create();
	if (!initializer)
	{
		return NULL;
	}

	list_t *incrementor = list_create();
	if (!incrementor)
	{
		return NULL;
	}

	node_t *condition = NULL;
	if (parser->token->type == TOKEN_LBRACE)
	{
		parser->loop_depth += 1;

		node_t *body = parser_body(program, parser, scope, node);
		if (!body)
		{
			return NULL;
		}

		parser->loop_depth -= 1;

		node_t *initializer_block = node_make_block(node, initializer);
		if (initializer_block == NULL)
		{
			return NULL;
		}

		node_t *incrementor_block = node_make_block(node, incrementor);
		if (incrementor_block == NULL)
		{
			return NULL;
		}

		return node_make_for(node, flag, key, initializer_block, condition, incrementor_block, body);
	}

	if (parser_match(program, parser, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	if (parser->token->type == TOKEN_SEMICOLON)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		goto region_condition;
	}

	int32_t use_readonly = 0;
	if (parser->token->type == TOKEN_READONLY_KEYWORD)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		use_readonly = 1;
	}

	while (true)
	{
		node_t *node2 = NULL;
		if (use_readonly || (parser->token->type == TOKEN_VAR_KEYWORD))
		{
			node2 = parser_var(program, parser, scope, node, use_readonly ? PARSER_MODIFIER_READONLY:PARSER_MODIFIER_NONE);
		}
		else
		{
			node2 = parser_assign(program, parser, scope, node);
		}
		
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(initializer, node2))
		{
			return NULL;
		}

		if (use_readonly)
		{
			goto region_forin;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
	{
		return NULL;
	}

	region_condition:
	if (parser->token->type == TOKEN_SEMICOLON)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		goto region_step;
	}

	condition = parser_expression(program, parser, scope, node);
	if (!condition)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
	{
		return NULL;
	}

	region_step:
	if (parser->token->type == TOKEN_RPAREN)
	{
		goto region_finish;
	}

	while (true)
	{
		node_t *node2 = parser_assign(program, parser, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(incrementor, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	region_finish:
	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	parser->loop_depth += 1;

	node_t *body = parser_body(program, parser, scope, node);
	if (!body)
	{
		return NULL;
	}

	parser->loop_depth -= 1;

	node_t *initializer_block = node_make_block(node, initializer);
	if (initializer_block == NULL)
	{
		return NULL;
	}

	node_t *incrementor_block = node_make_block(node, incrementor);
	if (incrementor_block == NULL)
	{
		return NULL;
	}

	return node_make_for(node, flag, key, initializer_block, condition, incrementor_block, body);

	region_forin:
	if (parser_match(program, parser, TOKEN_IN_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *iterator = parser_expression(program, parser, scope, node);
	if (!iterator)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	parser->loop_depth += 1;

	body = parser_body(program, parser, scope, node);
	if (!body)
	{
		return NULL;
	}

	parser->loop_depth -= 1;	initializer_block = node_make_block(node, initializer);
	if (initializer_block == NULL)
	{
		return NULL;
	}

	return node_make_forin(node, flag, key, initializer_block, iterator, body);
}

static node_t *
parser_break(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_next(program, parser) == -1)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_BREAK_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type != TOKEN_SEMICOLON)
	{
		value = parser_id(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_break(node, value);
}

static node_t *
parser_continue(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_next(program, parser) == -1)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_CONTINUE_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type != TOKEN_SEMICOLON)
	{
		value = parser_id(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_continue(node, value);
}

static node_t *
parser_catch(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}
	
	if (parser_next(program, parser) == -1)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_CATCH_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *parameters = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		if (parser->token->type != TOKEN_RPAREN)
		{
			parameters = parser_parameters(program, parser, scope, node);
			if (!parameters)
			{
				return NULL;
			}
		}
		if (parser_match(program, parser, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	node_t *body;
	body = parser_body(program, parser, scope, node);
	if (!body)
	{
		return NULL;
	}

	return node_make_catch(node, parameters, body);
}

static node_t *
parser_catchs(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *list = list_create();
	if (!list)
	{
		return NULL;
	}
	while (parser->token->type == TOKEN_CATCH_KEYWORD)
	{
		node_t *node2 = parser_catch(program, parser, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(list, node2))
		{
			return NULL;
		}
	}

	return node_make_catchs(node, list);
}

static node_t *
parser_try(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_TRY_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *body = parser_body(program, parser, scope, node);
	if (!body)
	{
		return NULL;
	}

	node_t *catchs = NULL;
	if (parser->token->type == TOKEN_CATCH_KEYWORD)
	{
		catchs = parser_catchs(program, parser, scope, node);
		if (!catchs)
		{
			return NULL;
		}
	}

	return node_make_try(node, body, catchs);
}

static node_t *
parser_return(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_RETURN_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	value = parser_expression(program, parser, scope, node);
	if (!value)
	{
		return NULL;
	}

	return node_make_return(node, value);
}

static node_t *
parser_throw(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_THROW_KEYWORD) == -1)
	{
		return NULL;
	}	if (parser_match(program, parser, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}
	node_t *arguments;
	arguments = parser_arguments(program, parser, scope, node);
	if (!arguments)
	{
		return NULL;
	}
	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	return node_make_throw(node, arguments);
}

static node_t *
parser_body(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	list_t *stmt_list = list_create();
	if (!stmt_list)
	{
		return NULL;
	}

	while (parser->token->type != TOKEN_RBRACE)
	{
		if (parser->token->type == TOKEN_SEMICOLON)
		{
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}

			continue;
		}

		node_t *stmt = parser_statement(program, parser, scope, node);
		if (!stmt)
		{
			return NULL;
		}

		if (!list_rpush(stmt_list, stmt))
		{
			return NULL;
		}
	}

	if (parser_match(program, parser, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_body(node, stmt_list);
}

static node_t *
parser_parameter(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	uint64_t flag = PARSER_MODIFIER_NONE;
	if (parser->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		flag |= PARSER_MODIFIER_REFERENCE;
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}
	else 
	if (parser->token->type == TOKEN_READONLY_KEYWORD)
	{
		flag |= PARSER_MODIFIER_READONLY;
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	node_t *key = parser_id(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}	if (parser_match(program, parser, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *type = parser_expression(program, parser, scope, node);
	if (!type)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_parameter(node, flag, key, type, value);
}

static node_t *
parser_parameters(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *parameters = list_create();
	if (!parameters)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = parser_parameter(program, parser, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(parameters, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_parameters(node, parameters);
}

static node_t *
parser_generic(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key;
	key = parser_id(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		type = parser_expression(program, parser, scope, node);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_generic(node, key, type, value);
}

static node_t *
parser_generics(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *generics = list_create();
	if (!generics)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = parser_generic(program, parser, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(generics, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_generics(node, generics);
}

static node_t *
parser_func(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_FUNC_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t used_generic = 0;
	node_t *generics = NULL;
	if (parser->token->type == TOKEN_LBRACE)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RBRACE)
		{
			parser_error(program, parser->token->position, "empty generic types");
			return NULL;
		}

		generics = parser_generics(program, parser, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (parser_match(program, parser, TOKEN_RBRACE) == -1)
		{
			return NULL;
		}

		used_generic = 1;
	}

	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser, node, node);
		if (!key)
		{
			return NULL;
		}
		
		if (parser_idstrcmp(key, "constructor") == 0)
		{
			if (used_generic == 1)
			{
				parser_error(program, parser->token->position, "constructor with generic types");
				return NULL;
			}
		}
	}
	else
	{
		key = parser_operator(program, parser, node, node);
		if (!key)
		{
			return NULL;
		}

		if (used_generic == 1)
		{
			parser_error(program, parser->token->position, "operator with generic types");
			return NULL;
		}
	}

	node_t *parameters = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_RPAREN)
		{
			parameters = parser_parameters(program, parser, scope, node);
			if (!parameters)
			{
				return NULL;
			}
		}

		if (parser_match(program, parser, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	node_t *body = parser_body(program, parser, scope, node);
	if (!body)
	{
		return NULL;
	}

	return node_make_func(node, flag, key, generics, parameters, body);
}

static node_t *
parser_readonly(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	if (parser_match(program, parser, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_READONLY;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_VAR_KEYWORD:
		node = parser_var(program, parser, scope, parent, flag);
		break;

	default:
		parser_error(program, parser->token->position, "incorrect use of modifier 'readonly'");
		break;
	}

	flag &= ~PARSER_MODIFIER_READONLY;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_async(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	if (parser_match(program, parser, TOKEN_ASYNC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_ASYNC;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_FOR_KEYWORD:
		node = parser_for(program, parser, scope, parent, flag);
		break;

	default:
		parser_error(program, parser->token->position, "incorrect use of modifier 'async'");
		break;
	}

	flag &= ~PARSER_MODIFIER_ASYNC;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_statement(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = NULL;
	if (parser->token->type == TOKEN_IF_KEYWORD)
	{
		node = parser_if(program, parser, scope, parent);
	}
	else
	if (parser->token->type == TOKEN_TRY_KEYWORD)
	{
		node = parser_try(program, parser, scope, parent);
	}
	else
	if (parser->token->type == TOKEN_FOR_KEYWORD)
	{
		node = parser_for(program, parser, scope, parent, PARSER_MODIFIER_NONE);
	}
	else
	if (parser->token->type == TOKEN_VAR_KEYWORD)
	{
		node = parser_var(program, parser, scope, parent, PARSER_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (parser->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = parser_readonly(program, parser, scope, parent, PARSER_MODIFIER_NONE);
	}
	else
	if (parser->token->type == TOKEN_ASYNC_KEYWORD)
	{
		node = parser_async(program, parser, scope, parent, PARSER_MODIFIER_NONE);
	}
	else
	if (parser->token->type == TOKEN_BREAK_KEYWORD)
	{
		node = parser_break(program, parser, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (parser->token->type == TOKEN_CONTINUE_KEYWORD)
	{
		node = parser_continue(program, parser, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (parser->token->type == TOKEN_RETURN_KEYWORD)
	{
		node = parser_return(program, parser, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (parser->token->type == TOKEN_THROW_KEYWORD)
	{
		node = parser_throw(program, parser, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	{
		node = parser_assign(program, parser, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	return node;
}

static node_t *
parser_member(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = parser_id(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_member(node, key, value);
}

static node_t *
parser_members(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *properties = list_create();
	if (!properties)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = parser_member(program, parser, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(properties, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_members(node, properties);
}

static node_t *
parser_enum(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_ENUM_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key;
	key = parser_id(program, parser, node, node);
	if (!key)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	node_t *members = parser_members(program, parser, node, node);
	if (!members)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_enum(node, flag, key, members);
}

static node_t *
parser_property(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = parser_id(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
		type = parser_expression(program, parser, scope, node);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if ((flag & PARSER_MODIFIER_STATIC) == PARSER_MODIFIER_STATIC)
	{
		if (parser_match(program, parser, TOKEN_EQ) == -1)
		{
			return NULL;
		}
		value = parser_expression(program, parser, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_property(node, flag, key, type, value);
}

static node_t *
parser_class_readonly(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	if (parser_match(program, parser, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_READONLY;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_ID:
		node = parser_property(program, parser, scope, parent, flag);
		break;

	default:
		parser_error(program, parser->token->position, "incorrect use of modifier 'readonly'");
		break;
	}

	flag &= ~PARSER_MODIFIER_READONLY;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_static(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	if (parser_match(program, parser, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_STATIC;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(program, parser, scope, parent, flag);
		break;

	case TOKEN_ID:
		node = parser_property(program, parser, scope, parent, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func(program, parser, scope, parent, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, scope, parent, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, scope, parent, flag);
		break;

	default:
		parser_error(program, parser->token->position, "incorrect use of modifier 'static'");
		break;
	}

	flag &= ~PARSER_MODIFIER_STATIC;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_protected(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	if (parser_match(program, parser, TOKEN_PROTECTED_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_PROTECTED;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(program, parser, scope, parent, flag);
		break;

	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(program, parser, scope, parent, flag);
		break;

	case TOKEN_ID:
		node = parser_property(program, parser, scope, parent, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func(program, parser, scope, parent, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, scope, parent, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, scope, parent, flag);
		break;

	default:
		parser_error(program, parser->token->position, "incorrect use of modifier 'protected'");
		break;
	}

	flag &= ~PARSER_MODIFIER_PROTECTED;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_export(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	if (parser_match(program, parser, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_EXPORT;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(program, parser, scope, parent, flag);
		break;

	case TOKEN_PROTECTED_KEYWORD:
		node = parser_class_protected(program, parser, scope, parent, flag);
		break;	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(program, parser, scope, parent, flag);
		break;

	case TOKEN_ID:
		node = parser_property(program, parser, scope, parent, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func(program, parser, scope, parent, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, scope, parent, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, scope, parent, flag);
		break;

	default:
		parser_error(program, parser->token->position, "incorrect use of modifier 'public'");
		break;
	}

	flag &= ~PARSER_MODIFIER_EXPORT;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_block(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *list = list_create();
	if (!list)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	while (parser->token->type != TOKEN_RBRACE)
	{
		node_t *decl = NULL;
		if (parser->token->type == TOKEN_READONLY_KEYWORD)
		{
			decl = parser_class_readonly(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_STATIC_KEYWORD)
		{
			decl = parser_class_static(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_EXPORT_KEYWORD)
		{
			decl = parser_class_export(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_ID)
		{
			decl = parser_property(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
			if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_FUNC_KEYWORD)
		{
			decl = parser_func(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_CLASS_KEYWORD)
		{
			decl = parser_class(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_ENUM_KEYWORD)
		{
			decl = parser_enum(program, parser, scope, parent, PARSER_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		{
			parser_error(program, parser->token->position, "illegal declaration type");
			return NULL;
		}

		if (list_rpush(list, decl) == NULL)
		{
			return NULL;
		}
	}

	if (parser_match(program, parser, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_block(node, list);
}

static node_t *
parser_heritage(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = parser_id(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}	if (parser_match(program, parser, TOKEN_COLON) == -1)
	{
		return NULL;
	}	node_t *type = parser_expression(program, parser, scope, node);
	if (!type)
	{
		return NULL;
	}

	return node_make_heritage(node, key, type);
}

static node_t *
parser_heritages(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *heritage = list_create();
	if (!heritage)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = parser_heritage(program, parser, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(heritage, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_heritages(node, heritage);
}

static node_t *
parser_class(program_t *program, parser_t *parser, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_CLASS_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *generics = NULL;
	if (parser->token->type == TOKEN_LBRACE)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RBRACE)
		{
			parser_error(program, parser->token->position, "empty generic types");
			return NULL;
		}

		generics = parser_generics(program, parser, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (parser_match(program, parser, TOKEN_RBRACE) == -1)
		{
			return NULL;
		}
	}

	node_t *key = parser_id(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *heritages = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		if (parser_match(program, parser, TOKEN_LPAREN) == -1)
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RPAREN)
		{
			parser_error(program, parser->token->position, "empty heritages");
			return NULL;
		}

		heritages = parser_heritages(program, parser, node, node);
		if (!heritages)
		{
			return NULL;
		}

		if (parser_match(program, parser, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}

	}

	node_t *block = parser_class_block(program, parser, node, node);
	if (!block)
	{
		return NULL;
	}

	return node_make_class(node, flag, key, generics, heritages, block);
}

static node_t *
parser_field(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key;
	key = parser_expression(program, parser, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}

		type = parser_expression(program, parser, scope, node);
		if (!type)
		{
			return NULL;
		}
	}

	return node_make_field(node, key, type);
}

static node_t *
parser_fields(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *fields = list_create();
	if (!fields)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = parser_field(program, parser, scope, parent);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(fields, node2))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (parser_next(program, parser) == -1)
		{
			return NULL;
		}
	}

	return node_make_fields(node, fields);
}

static node_t *
parser_import(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_IMPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	node_t *path = parser_string(program, parser, scope, node);
	if (path == NULL)
	{
		return NULL;
	}

	node_t *fields = NULL;
	if (parser->token->type == TOKEN_COMMA)
	{
		if (parser_match(program, parser, TOKEN_COMMA) == -1)
		{
			return NULL;
		}

		fields = parser_fields(program, parser, scope, node);
		if (!fields)
		{
			return NULL;
		}
	}

	if (parser_match(program, parser, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_SEMICOLON) == -1)
	{
		return NULL;
	}

	return node_make_import(node, path, fields);
}

static node_t *
parser_export(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	if (parser_match(program, parser, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	uint64_t flag = PARSER_MODIFIER_EXPORT;

	node_t *object = NULL;

	if (parser->token->type == TOKEN_CLASS_KEYWORD)
	{
		object = parser_class(program, parser, scope, parent, flag);
	}
	else
	if (parser->token->type == TOKEN_ENUM_KEYWORD)
	{
		object = parser_enum(program, parser, scope, parent, flag);
	}
	else
	if (parser->token->type == TOKEN_FUNC_KEYWORD)
	{
		object = parser_func(program, parser, scope, parent, flag);
	}
	else
	if (parser->token->type == TOKEN_VAR_KEYWORD)
	{
		object = parser_var(program, parser, scope, parent, flag);
	}
	else
	if (parser->token->type == TOKEN_READONLY_KEYWORD)
	{
		object = parser_readonly(program, parser, scope, parent, flag);
	}
	else
	if (parser->token->type == TOKEN_LBRACE)
	{
		object = parser_object(program, parser, scope, parent, flag);
	}
	else
	{
		parser_error(program, parser->token->position, "illegal declaration");
		return NULL;
	}

	flag &= ~PARSER_MODIFIER_EXPORT;

	return object;
}

node_t *
parser_module_block(program_t *program, parser_t *parser, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *items = list_create();
	if (!items)
	{
		return NULL;
	}

	while (parser->token->type != TOKEN_EOF)
	{

		if (parser->token->type == TOKEN_SEMICOLON)
		{
			if (parser_next(program, parser) == -1)
			{
				return NULL;
			}
			continue;
		}

		node_t *item = NULL;

		if (parser->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = parser_export(program, parser, scope, node);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_IMPORT_KEYWORD)
		{
			item = parser_import(program, parser, scope, node);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_CLASS_KEYWORD)
		{
			item = parser_class(program, parser, scope, node, PARSER_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_ENUM_KEYWORD)
		{
			item = parser_enum(program, parser, scope, node, PARSER_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_FUNC_KEYWORD)
		{
			item = parser_func(program, parser, scope, node, PARSER_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_VAR_KEYWORD)
		{
			item = parser_var(program, parser, scope, node, PARSER_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (parser->token->type == TOKEN_READONLY_KEYWORD)
		{
			item = parser_readonly(program, parser, scope, node, PARSER_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		{
			parser_error(program, parser->token->position, "unknown type in module");
			return NULL;
		}

		if (!list_rpush(items, item))
		{
			return NULL;
		}
	}

	if (parser_match(program, parser, TOKEN_EOF) == -1)
	{
		return NULL;
	}

	return node_make_block(node, items);
}

node_t *
parser_module(program_t *program, parser_t *parser)
{
	node_t *node = node_create(NULL, NULL, parser->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *block = parser_module_block(program, parser, node, node);
	if (block == NULL)
	{
		return NULL;
	}

	if (parser_match(program, parser, TOKEN_EOF) == -1)
	{
		return NULL;
	}

	return node_make_module(node, parser->file_source->path, block);
}

parser_t *
parser_create(program_t *program, char *path)
{
	parser_t *parser;
	parser = (parser_t *)malloc(sizeof(parser_t));
	if (!parser)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(parser_t));
		return NULL;
	}
	memset(parser, 0, sizeof(parser_t));

	file_source_t *file_source;
	file_source = file_create_source(path);
	if (!file_source)
	{
		return NULL;
	}

	scanner_t *scanner;
	scanner = scanner_create(file_source, program->errors);
	if (!scanner)
	{
		return NULL;
	}

	parser->scanner = scanner;
	parser->file_source = file_source;
	parser->loop_depth = 0;
	parser->token = &scanner->token;

	parser->states = list_create();
	if (!parser->states)
	{
		return NULL;
	}

	return parser;
}
