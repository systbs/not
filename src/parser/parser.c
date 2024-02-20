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
parser_expression(program_t *program, parser_t *parser);

static node_t *
parser_statement(program_t *program, parser_t *parser);

static node_t *
parser_block_stmt(program_t *program, parser_t *parser);

static node_t *
parser_class(program_t *program, parser_t *parser, uint64_t flag);

static node_t *
parser_func_stmt(program_t *program, parser_t *parser, uint64_t flag);

static list_t *
parser_parameters(program_t *program, parser_t *parser);

static list_t *
parser_generics(program_t *program, parser_t *parser);

static node_t *
parser_parameter(program_t *program, parser_t *parser);

static node_t *
parser_parenthesis(program_t *program, parser_t *parser);

static list_t *
parser_heritages(program_t *program, parser_t *parser);

static node_t *
parser_prefix(program_t *program, parser_t *parser);

static node_t *
parser_object(program_t *program, parser_t *parser, uint64_t flag);

static node_t *
parser_export(program_t *program, parser_t *parser);

int32_t
parser_save_state(program_t *program, parser_t *parser)
{
	scanner_t *scanner;
	scanner = parser->scanner;

	parser_state_t *state;
	if (!(state = (parser_state_t *)malloc(sizeof(parser_state_t))))
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(parser_state_t));
		return 0;
	}

	state->loop_depth = parser->loop_depth;
	state->offset = scanner->offset;
	state->reading_offset = scanner->reading_offset;
	state->line = scanner->line;
	state->column = scanner->column;
	state->ch = scanner->ch;
	state->token = scanner->token;

	if (!list_rpush(parser->states, (uint64_t)state))
	{
		return 0;
	}

	return 1;
}

int32_t
parser_restore_state(program_t *program, parser_t *parser)
{
	scanner_t *scanner;
	scanner = parser->scanner;

	parser_state_t *state;
	state = (parser_state_t *)list_content(list_rpop(parser->states));
	if (!state)
	{
		return 0;
	}

	parser->loop_depth = state->loop_depth;

	scanner->offset = state->offset;
	scanner->reading_offset = state->reading_offset;
	scanner->line = state->line;
	scanner->column = state->column;
	scanner->ch = state->ch;

	scanner->token = state->token;
	free(state);
	return 1;
}

int32_t
parser_release_state(program_t *program, parser_t *parser)
{
	parser_state_t *state;
	state = (parser_state_t *)list_content(list_rpop(parser->states));
	if (!state)
	{
		return 0;
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

	if (list_rpush(program->errors, (uint64_t)error))
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

	if (list_rpush(program->errors, (uint64_t)error))
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
		if (!scanner_advance(parser->scanner))
		{
			return 0;
		}
	}
	else
	{
		parser_expected(program, parser, type);
		return 0;
	}
	return 1;
}

static int32_t
parser_next(program_t *program, parser_t *parser)
{
	if (!scanner_advance(parser->scanner))
	{
		return 0;
	}
	return 1;
}

static node_t *
parser_id(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_id(parser->token->position, parser->token->value);
	if (!node)
	{
		return NULL;
	}
	if (!parser_match(program, parser, TOKEN_ID))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_number(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_number(parser->token->position, parser->token->value);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_NUMBER))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_char(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_char(parser->token->position, parser->token->value);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_CHAR))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_string(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_string(parser->token->position, parser->token->value);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_STRING))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_null(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_null(parser->token->position);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_NULL_KEYWORD))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_true(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_true(parser->token->position);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_TRUE_KEYWORD))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_false(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_false(parser->token->position);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_FALSE_KEYWORD))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_infinity(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_infinity(parser->token->position);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_INFINITY_KEYWORD))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_this(program_t *program, parser_t *parser)
{
	node_t *node;
	node = node_make_this(parser->token->position);
	if (!node)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_THIS_KEYWORD))
	{
		return NULL;
	}
	return node;
}

static node_t *
parser_operator(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	char *operator = NULL;
	if (parser->token->type == TOKEN_PLUS)
	{
		if (!parser_match(program, parser, TOKEN_PLUS))
		{
			return NULL;
		}
		operator = "+";
	}
	else if (parser->token->type == TOKEN_MINUS)
	{
		if (!parser_match(program, parser, TOKEN_MINUS))
		{
			return NULL;
		}
		operator = "-";
	}
	else if (parser->token->type == TOKEN_STAR)
	{
		if (!parser_match(program, parser, TOKEN_STAR))
		{
			return NULL;
		}
		operator = "*";
	}
	else if (parser->token->type == TOKEN_SLASH)
	{
		if (!parser_match(program, parser, TOKEN_SLASH))
		{
			return NULL;
		}
		operator = "/";
	}
	else if (parser->token->type == TOKEN_POWER)
	{
		if (!parser_match(program, parser, TOKEN_POWER))
		{
			return NULL;
		}
		operator = "**";
	}
	else if (parser->token->type == TOKEN_PERCENT)
	{
		if (!parser_match(program, parser, TOKEN_PERCENT))
		{
			return NULL;
		}
		operator = "%";
	}
	else if (parser->token->type == TOKEN_AND)
	{
		if (!parser_match(program, parser, TOKEN_AND))
		{
			return NULL;
		}
		operator = "&";
	}
	else if (parser->token->type == TOKEN_OR)
	{
		if (!parser_match(program, parser, TOKEN_OR))
		{
			return NULL;
		}
		operator = "|";
	}
	else if (parser->token->type == TOKEN_CARET)
	{
		if (!parser_match(program, parser, TOKEN_CARET))
		{
			return NULL;
		}
		operator = "^";
	}
	else if (parser->token->type == TOKEN_LT_LT)
	{
		if (!parser_match(program, parser, TOKEN_LT_LT))
		{
			return NULL;
		}
		operator = "<<";
	}
	else if (parser->token->type == TOKEN_GT_GT)
	{
		if (!parser_match(program, parser, TOKEN_GT_GT))
		{
			return NULL;
		}
		operator = ">>";
	}
	else if (parser->token->type == TOKEN_LT)
	{
		if (!parser_match(program, parser, TOKEN_LT))
		{
			return NULL;
		}
		operator = "<";
	}
	else if (parser->token->type == TOKEN_GT)
	{
		if (!parser_match(program, parser, TOKEN_GT))
		{
			return NULL;
		}
		operator = ">";
	}
	else if (parser->token->type == TOKEN_LBRACKET)
	{
		if (!parser_match(program, parser, TOKEN_LBRACKET))
		{
			return NULL;
		}
		if (!parser_match(program, parser, TOKEN_RBRACKET))
		{
			return NULL;
		}
		operator = "[]";
	}
	else 
	{
		parser_error(program, position, "operator expected\n");
		return NULL;
	}

	node_t *node = node_make_id(position, operator);
	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_array(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_LBRACKET))
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
			node_t *expr;
			expr = parser_expression(program, parser);
			if (!expr)
			{
				return NULL;
			}

			if (!list_rpush(expr_list, (uint64_t)expr))
			{
				return NULL;
			}

			if (parser->token->type != TOKEN_COMMA)
			{
				break;
			}
			if (!parser_match(program, parser, TOKEN_COMMA))
			{
				return NULL;
			}
		}
	}

	if (!parser_match(program, parser, TOKEN_RBRACKET))
	{
		return NULL;
	}

	return node_make_array(position, expr_list);
}

static node_t *
parser_object_property(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *name;
	name = parser_expression(program, parser);
	if (!name)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_object_property(position, name, value);
}

static node_t *
parser_object(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_LBRACE))
	{
		return NULL;
	}

	list_t *property_list;
	property_list = list_create();
	if (!property_list)
	{
		return NULL;
	}

	if (parser->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *property;
			property = parser_object_property(program, parser);
			if (!property)
			{
				return NULL;
			}

			if (!list_rpush(property_list, (uint64_t)property))
			{
				return NULL;
			}

			if (parser->token->type != TOKEN_COMMA)
			{
				break;
			}
			if (!parser_next(program, parser))
			{
				return NULL;
			}
		}
	}

	if (!parser_match(program, parser, TOKEN_RBRACE))
	{
		return NULL;
	}

	return node_make_object(position, flag, property_list);
}

static node_t *
parser_parenthesis(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_LPAREN))
	{
		return NULL;
	}

	if(!parser_save_state(program, parser))
	{
		return NULL;
	}

	node_t *value = parser_expression(program, parser);
	if (!value)
	{
		return NULL;
	}

	if (parser->token->type == TOKEN_COLON)
	{
		if(!parser_restore_state(program, parser))
		{
			return NULL;
		}
		goto region_lambda;
	}

	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	return node_make_parenthesis(position, value);

	list_t *parameters;
	region_lambda:
	parameters = parser_parameters(program, parser);
	if (!parameters)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	node_t *body = NULL;
	if (parser->token->type == TOKEN_MINUS_GT)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		body = parser_expression(program, parser);
		if (!body)
		{
			return NULL;
		}
	}
	else
	{
		body = parser_block_stmt(program, parser);
		if (!body)
		{
			return NULL;
		}
	}
	
	return node_make_lambda(position, parameters, body);
}

static node_t *
parser_primary(program_t *program, parser_t *parser)
{
	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_ID:
		node = parser_id(program, parser);
		break;

	case TOKEN_NUMBER:
		node = parser_number(program, parser);
		break;

	case TOKEN_STRING:
		node = parser_string(program, parser);
		break;

	case TOKEN_CHAR:
		node = parser_char(program, parser);
		break;

	case TOKEN_NULL_KEYWORD:
		node = parser_null(program, parser);
		break;

	case TOKEN_TRUE_KEYWORD:
		node = parser_true(program, parser);
		break;

	case TOKEN_FALSE_KEYWORD:
		node = parser_false(program, parser);
		break;

	case TOKEN_INFINITY_KEYWORD:
		node = parser_infinity(program, parser);
		break;

	case TOKEN_THIS_KEYWORD:
		node = parser_this(program, parser);
		break;

	case TOKEN_LBRACKET:
		node = parser_array(program, parser);
		break;

	case TOKEN_LBRACE:
		node = parser_object(program, parser, PARSER_MODIFIER_NONE);
		break;

	case TOKEN_LPAREN:
		node = parser_parenthesis(program, parser);
		break;

	default:
		parser_error(program, parser->token->position, "expression expected %d\n", parser->token->type);
		return NULL;
	}

	return node;
}

static node_t *
parser_argument(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *key = NULL;
	key = parser_expression(program, parser);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_argument(position, key, value);
}

static list_t *
parser_arguments(program_t *program, parser_t *parser)
{
	list_t *arguments;
	arguments = list_create();
	if (!arguments)
	{
		return NULL;
	}

	node_t *node;
	while (true)
	{
		node = parser_argument(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(arguments, (uint64_t)node))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	return arguments;
}

static node_t *
parser_postfix(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_primary(program, parser);
	if (!node)
	{
		return NULL;
	}

	list_t *arguments;
	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;

		switch (parser->token->type)
		{
		case TOKEN_LBRACE:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			if (parser->token->type == TOKEN_RBRACE)
			{
				parser_error(program, parser->token->position, "empty type arguments");
			}

			arguments = parser_arguments(program, parser);
			if (!arguments)
			{
				return NULL;
			}

			if (!parser_match(program, parser, TOKEN_RBRACE))
			{
				return NULL;
			}

			node = node_make_composite(position, node, arguments);
			break;

		case TOKEN_LPAREN:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			arguments = NULL;
			if (parser->token->type != TOKEN_RPAREN)
			{
				arguments = parser_arguments(program, parser);
				if (!(arguments))
				{
					return NULL;
				}
			}

			if (!parser_match(program, parser, TOKEN_RPAREN))
			{
				return NULL;
			}
			node = node_make_call(position, node, arguments);
			break;

		case TOKEN_LBRACKET:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			arguments = NULL;
			if (parser->token->type != TOKEN_RBRACKET)
			{
				arguments = parser_arguments(program, parser);
				if (!(arguments))
				{
					return NULL;
				}
			}

			if (!parser_match(program, parser, TOKEN_RBRACKET))
			{
				return NULL;
			}

			node = node_make_item(position, node, arguments);
			break;

		case TOKEN_DOT:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_id(program, parser);
			if (!right)
			{
				return NULL;
			}
      
			node = node_make_attribute(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_prefix(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *node = NULL;
	node_t *right;
	switch (parser->token->type)
	{
	case TOKEN_TILDE:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_prefix(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_tilde(position, right);
		break;

	case TOKEN_NOT:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_prefix(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_not(position, right);
		break;

	case TOKEN_MINUS:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_prefix(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_neg(position, right);
		break;

	case TOKEN_PLUS:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_prefix(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_pos(position, right);
		break;

	case TOKEN_SIZEOF_KEYWORD:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_prefix(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_sizeof(position, right);
		break;

	case TOKEN_TYPEOF_KEYWORD:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_prefix(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_typeof(position, right);
		break;

	default:
		node = parser_postfix(program, parser);
		break;
	}

	return node;
}

static node_t *
parser_power(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_prefix(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_POWER:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_prefix(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_pow(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_multiplicative(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_power(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_STAR:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_power(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_mul(position, node, right);
			break;

		case TOKEN_SLASH:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_power(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_div(position, node, right);
			break;

		case TOKEN_PERCENT:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_power(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_mod(position, node, right);
			break;

		case TOKEN_BACKSLASH:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_power(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_epi(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_addative(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_multiplicative(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_PLUS:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_multiplicative(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_plus(position, node, right);
			break;

		case TOKEN_MINUS:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_multiplicative(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_minus(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_shifting(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_addative(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_LT_LT:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_addative(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_shl(position, node, right);
			break;

		case TOKEN_GT_GT:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_addative(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_shr(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_relational(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_shifting(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_LT:
			if (!parser_next(program, parser))
			{
				return NULL;
			}
			right = parser_shifting(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_lt(position, node, right);
			break;

		case TOKEN_LT_EQ:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_shifting(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_le(position, node, right);
			break;

		case TOKEN_GT:
			if (!parser_next(program, parser))
			{
				return NULL;
			}
			right = parser_shifting(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_gt(position, node, right);
			break;

		case TOKEN_GT_EQ:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_shifting(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_ge(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_equality(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_relational(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;

		switch (parser->token->type)
		{
		case TOKEN_EQ_EQ:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_relational(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_eq(position, node, right);
			break;

		case TOKEN_NOT_EQ:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_relational(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_neq(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_bitwise_and(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_equality(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_AND:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_equality(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_and(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_bitwise_xor(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_bitwise_and(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_CARET:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_bitwise_and(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_xor(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_bitwise_or(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_bitwise_xor(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_OR:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_bitwise_xor(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_or(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_logical_and(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_bitwise_or(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_AND_AND:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_bitwise_or(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_land(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_logical_or(program_t *program, parser_t *parser)
{
	node_t *node;
	node = parser_logical_and(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_OR_OR:
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			right = parser_logical_and(program, parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_lor(position, node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_conditional(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *node;
	node = parser_logical_or(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *true_expression;
	node_t *false_expression;
	switch (parser->token->type)
	{
	case TOKEN_QUESTION:
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		true_expression = NULL;
		if (parser->token->type != TOKEN_COLON)
		{
			true_expression = parser_conditional(program, parser);
			if (!true_expression)
			{
				break;
			}
		}
		if (!parser_match(program, parser, TOKEN_COLON))
		{
			return NULL;
		}

		false_expression = parser_conditional(program, parser);
		if (!false_expression)
		{
			break;
		}

		node = node_make_conditional(position, node, true_expression, false_expression);
		break;

	default:
		return node;
	}

	return node;
}

static node_t *
parser_expression(program_t *program, parser_t *parser)
{
	return parser_conditional(program, parser);
}

static node_t *
parser_assign_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *node;
	node = parser_expression(program, parser);
	if (!node)
	{
		return NULL;
	}

	node_t *right;
	switch (parser->token->type)
	{
	case TOKEN_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_assign(position, node, right);
		break;

	case TOKEN_COLON_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_define(position, node, right);
		break;

	case TOKEN_PLUS_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_add_assign(position, node, right);
		break;

	case TOKEN_MINUS_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_sub_assign(position, node, right);
		break;

	case TOKEN_STAR_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_mul_assign(position, node, right);
		break;

	case TOKEN_SLASH_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_div_assign(position, node, right);
		break;

	case TOKEN_PERCENT_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_mod_assign(position, node, right);
		break;

	case TOKEN_AND_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_and_assign(position, node, right);
		break;

	case TOKEN_OR_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_or_assign(position, node, right);
		break;

	case TOKEN_LT_LT_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_shl_assign(position, node, right);
		break;

	case TOKEN_GT_GT_EQ:
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		right = parser_expression(program, parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_shr_assign(position, node, right);
		break;

	default:
		return node;
	}

	return node;
}

static node_t *
parser_if_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_IF_KEYWORD))
	{
		return NULL;
	}

	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser);
		if (!key)
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_LPAREN))
	{
		return NULL;
	}

	node_t *condition;
	condition = parser_expression(program, parser);
	if (!condition)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	node_t *then_body;
	then_body = parser_block_stmt(program, parser);
	if (!then_body)
	{
		return NULL;
	}

	node_t *else_body;
	else_body = NULL;
	if (parser->token->type == TOKEN_ELSE_KEYWORD)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_IF_KEYWORD)
		{
			else_body = parser_if_stmt(program, parser);
		}
		else
		{
			else_body = parser_block_stmt(program, parser);
		}

		if (!else_body)
		{
			return NULL;
		}
	}

	return node_make_if(position, key, condition, then_body, else_body);
}

static node_t *
parser_var_stmt(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_VAR_KEYWORD))
	{
		return NULL;
	}

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		type = parser_expression(program, parser);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_var(position, flag, key, type, value);
}

static node_t *
parser_for_stmt(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_FOR_KEYWORD))
	{
		return NULL;
	}

	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser);
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

		node_t *body;
		body = parser_block_stmt(program, parser);
		if (!body)
		{
			return NULL;
		}

		parser->loop_depth -= 1;

		return node_make_for(position, flag, key, initializer, condition, incrementor, body);
	}

	if (!parser_match(program, parser, TOKEN_LPAREN))
	{
		return NULL;
	}

	if (parser->token->type == TOKEN_SEMICOLON)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		goto region_condition;
	}

	int32_t use_readonly = 0;
	if (parser->token->type == TOKEN_READONLY_KEYWORD)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		use_readonly = 1;
	}

	while (true)
	{
		node_t *node;
		if (use_readonly || (parser->token->type == TOKEN_VAR_KEYWORD))
		{
			node = parser_var_stmt(program, parser, use_readonly ? PARSER_MODIFIER_READONLY:PARSER_MODIFIER_NONE);
		}
		else
		{
			node = parser_assign_stmt(program, parser);
		}
		
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(initializer, (uint64_t)node))
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

		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	region_condition:
	if (parser->token->type == TOKEN_SEMICOLON)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		goto region_step;
	}

	condition = parser_expression(program, parser);
	if (!condition)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_SEMICOLON))
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
		node_t *node = parser_assign_stmt(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(incrementor, (uint64_t)node))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	region_finish:
	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	parser->loop_depth += 1;

	node_t *body = NULL;
	body = parser_block_stmt(program, parser);
	if (!body)
	{
		return NULL;
	}

	parser->loop_depth -= 1;

	return node_make_for(position, flag, key, initializer, condition, incrementor, body);

	region_forin:
	if (!parser_match(program, parser, TOKEN_IN_KEYWORD))
	{
		return NULL;
	}

	node_t *iterator;
	iterator = parser_expression(program, parser);
	if (!iterator)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	parser->loop_depth += 1;

	body = parser_block_stmt(program, parser);
	if (!body)
	{
		return NULL;
	}

	parser->loop_depth -= 1;

	return node_make_forin(position, flag, key, initializer, iterator, body);
}

static node_t *
parser_break_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_BREAK_KEYWORD))
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type != TOKEN_SEMICOLON)
	{
		value = parser_id(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_break(position, value);
}

static node_t *
parser_continue_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_CONTINUE_KEYWORD))
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type != TOKEN_SEMICOLON)
	{
		value = parser_id(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_continue(position, value);
}

static node_t *
parser_catch_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_CATCH_KEYWORD))
	{
		return NULL;
	}

	list_t *parameters = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		if (parser->token->type != TOKEN_RPAREN)
		{
			parameters = parser_parameters(program, parser);
			if (!parameters)
			{
				return NULL;
			}
		}
		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	node_t *body;
	body = parser_block_stmt(program, parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_catch(position, parameters, body);
}

static list_t *
parser_catchs(program_t *program, parser_t *parser)
{
	list_t *list;
	list = list_create();
	if (!list)
	{
		return NULL;
	}

	node_t *node;
	while (parser->token->type == TOKEN_CATCH_KEYWORD)
	{
		node = parser_catch_stmt(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(list, (uint64_t)node))
		{
			return NULL;
		}
	}

	return list;
}

static node_t *
parser_try_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_TRY_KEYWORD))
	{
		return NULL;
	}

	node_t *body;
	body = parser_block_stmt(program, parser);
	if (!body)
	{
		return NULL;
	}

	list_t *catchs = NULL;
	if (parser->token->type == TOKEN_CATCH_KEYWORD)
	{
		catchs = parser_catchs(program, parser);
		if (!catchs)
		{
			return NULL;
		}
	}

	return node_make_try(position, body, catchs);
}

static node_t *
parser_return_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_RETURN_KEYWORD))
	{
		return NULL;
	}

	node_t *value = NULL;
	value = parser_expression(program, parser);
	if (!value)
	{
		return NULL;
	}

	return node_make_return(position, value);
}

static node_t *
parser_throw_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_THROW_KEYWORD))
	{
		return NULL;
	}
	
	if (!parser_match(program, parser, TOKEN_LPAREN))
	{
		return NULL;
	}
	list_t *arguments;
	arguments = parser_arguments(program, parser);
	if (!arguments)
	{
		return NULL;
	}
	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	return node_make_throw(position, arguments);
}

static node_t *
parser_block_stmt(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_LBRACE))
	{
		return NULL;
	}

	list_t *stmt_list;
	stmt_list = list_create();
	if (!stmt_list)
	{
		return NULL;
	}

	while (parser->token->type != TOKEN_RBRACE)
	{
		/* empty stmt */
		if (parser->token->type == TOKEN_SEMICOLON)
		{
			if (!parser_next(program, parser))
			{
				return NULL;
			}

			continue;
		}

		node_t *stmt;
		stmt = parser_statement(program, parser);
		if (!stmt)
		{
			return NULL;
		}

		if (!list_rpush(stmt_list, (uint64_t)stmt))
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_RBRACE))
	{
		return NULL;
	}

	return node_make_block(position, stmt_list);
}

static node_t *
parser_parameter(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	uint64_t flag = PARSER_MODIFIER_NONE;
	if (parser->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		flag |= PARSER_MODIFIER_REFERENCE;
		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}
	else if (parser->token->type == TOKEN_READONLY_KEYWORD)
	{
		flag |= PARSER_MODIFIER_READONLY;
		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	
	if (!parser_match(program, parser, TOKEN_COLON))
	{
		return NULL;
	}

	node_t *type = NULL;
	type = parser_expression(program, parser);
	if (!type)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_parameter(position, flag, key, type, value);
}

static list_t *
parser_parameters(program_t *program, parser_t *parser)
{
	list_t *parameters;
	parameters = list_create();
	if (!parameters)
	{
		return NULL;
	}

	node_t *node;
	while (true)
	{
		node = parser_parameter(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(parameters, (uint64_t)node))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	return parameters;
}

static node_t *
parser_generic(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		type = parser_expression(program, parser);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_generic(position, key, type, value);
}

static list_t *
parser_generics(program_t *program, parser_t *parser)
{
	list_t *generics;
	generics = list_create();
	if (!generics)
	{
		return NULL;
	}

	node_t *node;
	while (true)
	{
		node = parser_generic(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(generics, (uint64_t)node))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	return generics;
}

static node_t *
parser_func_stmt(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_FUNC_KEYWORD))
	{
		return NULL;
	}
  
	list_t *generics = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RPAREN)
		{
			parser_error(program, parser->token->position, "empty generics");
			return NULL;
		}

		generics = parser_generics(program, parser);
		if (!generics)
		{
			return NULL;
		}

		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser);
		if (!key)
		{
			return NULL;
		}
	}
	else
	{
		key = parser_operator(program, parser);
		if (!key)
		{
			return NULL;
		}
	}

	list_t *parameters = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		if (parser->token->type != TOKEN_RPAREN)
		{
			parameters = parser_parameters(program, parser);
			if (!parameters)
			{
				return NULL;
			}
		}
		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	node_t *body = NULL;
	body = parser_block_stmt(program, parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_func(position, flag, key, generics, parameters, body);
}

static node_t *
parser_readonly_stmt(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_READONLY_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_READONLY;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_VAR_KEYWORD:
		node = parser_var_stmt(program, parser, flag);
		break;

	default:
		parser_error(program, position, "incorrect use of modifier 'readonly'");
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
parser_async_stmt(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_ASYNC_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_ASYNC;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_FOR_KEYWORD:
		node = parser_for_stmt(program, parser, flag);
		break;

	default:
		parser_error(program, position, "incorrect use of modifier 'async'");
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
parser_statement(program_t *program, parser_t *parser)
{
	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_LBRACE:
		node = parser_block_stmt(program, parser);
		break;

	case TOKEN_IF_KEYWORD:
		node = parser_if_stmt(program, parser);
		break;

	case TOKEN_TRY_KEYWORD:
		node = parser_try_stmt(program, parser);
		break;

	case TOKEN_FOR_KEYWORD:
		node = parser_for_stmt(program, parser, PARSER_MODIFIER_NONE);
		break;

	case TOKEN_VAR_KEYWORD:
		node = parser_var_stmt(program, parser, PARSER_MODIFIER_NONE);
		if (!parser_match(program, parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		break;

	case TOKEN_READONLY_KEYWORD:
		node = parser_readonly_stmt(program, parser, PARSER_MODIFIER_NONE);
		break;

	case TOKEN_ASYNC_KEYWORD:
		node = parser_async_stmt(program, parser, PARSER_MODIFIER_NONE);
		break;

	case TOKEN_BREAK_KEYWORD:
		node = parser_break_stmt(program, parser);
		if (!parser_match(program, parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		break;

	case TOKEN_CONTINUE_KEYWORD:
		node = parser_continue_stmt(program, parser);
		if (!parser_match(program, parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		break;

	case TOKEN_RETURN_KEYWORD:
		node = parser_return_stmt(program, parser);
		if (!parser_match(program, parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		break;

	case TOKEN_THROW_KEYWORD:
		node = parser_throw_stmt(program, parser);
		if (!parser_match(program, parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func_stmt(program, parser, PARSER_MODIFIER_NONE);
		break;

	default:
		node = parser_assign_stmt(program, parser);
		if (!node)
		{
			break;
		}
		if (!parser_match(program, parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		break;
	}


	return node;
}

static node_t *
parser_member(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_member(position, key, value);
}

static node_t *
parser_enum(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_ENUM_KEYWORD))
	{
		return NULL;
	}

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_LBRACE))
	{
		return NULL;
	}

	list_t *members;
	members = list_create();
	if (!members)
	{
		return NULL;
	}

	if (parser->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *member;
			member = parser_member(program, parser);
			if (!member)
			{
				return NULL;
			}

			if (!list_rpush(members, (uint64_t)member))
			{
				return NULL;
			}

			if (parser->token->type != TOKEN_COMMA)
			{
				break;
			}

			if (!parser_next(program, parser))
			{
				return NULL;
			}
		}
	}

	if (!parser_match(program, parser, TOKEN_RBRACE))
	{
		return NULL;
	}

	return node_make_enum(position, flag, key, members);
}

static node_t *
parser_class_func(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_FUNC_KEYWORD))
	{
		return NULL;
	}

	list_t *generics = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RPAREN)
		{
			parser_error(program, parser->token->position, "empty generics");
			return NULL;
		}

		generics = parser_generics(program, parser);
		if (!generics)
		{
			return NULL;
		}

		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}


	node_t *key = NULL;
	if (parser->token->type == TOKEN_ID)
	{
		key = parser_id(program, parser);
		if (!key)
		{
			return NULL;
		}
	}
	else
	{
		key = parser_operator(program, parser);
		if (!key)
		{
			return NULL;
		}
	}

	list_t *parameters = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		if (parser->token->type != TOKEN_RPAREN)
		{
			parameters = parser_parameters(program, parser);
			if (!parameters)
			{
				return NULL;
			}
		}
		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	node_t *body;
	body = parser_block_stmt(program, parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_func(position, flag, key, generics, parameters, body);
}

static node_t *
parser_class_property(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	node_t *key = NULL;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}
		type = parser_expression(program, parser);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if ((flag & PARSER_MODIFIER_STATIC) == PARSER_MODIFIER_STATIC)
	{
		if (!parser_match(program, parser, TOKEN_EQ))
		{
			return NULL;
		}
		value = parser_expression(program, parser);
		if (!value)
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_property(position, flag, key, type, value);
}

static node_t *
parser_class_readonly(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_READONLY_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_READONLY;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_ID:
		node = parser_class_property(program, parser, flag);
		break;

	default:
		parser_error(program, position, "incorrect use of modifier 'readonly'");
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
parser_class_static(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_STATIC_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_STATIC;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(program, parser, flag);
		break;

	case TOKEN_ID:
		node = parser_class_property(program, parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(program, parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, flag);
		break;

	default:
		parser_error(program, position, "incorrect use of modifier 'static'");
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
parser_class_protected(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_PROTECTED_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_PROTECTED;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(program, parser, flag);
		break;

	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(program, parser, flag);
		break;

	case TOKEN_ID:
		node = parser_class_property(program, parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(program, parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, flag);
		break;

	default:
		parser_error(program, position, "incorrect use of modifier 'protected'");
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
parser_class_export(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_EXPORT_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_EXPORT;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(program, parser, flag);
		break;

	case TOKEN_PROTECTED_KEYWORD:
		node = parser_class_protected(program, parser, flag);
		break;
	
	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(program, parser, flag);
		break;

	case TOKEN_ID:
		node = parser_class_property(program, parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(program, parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, flag);
		break;

	default:
		parser_error(program, position, "incorrect use of modifier 'public'");
		break;
	}

	flag &= ~PARSER_MODIFIER_EXPORT;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static list_t *
parser_class_block(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	list_t *list;
	list = list_create();
	if (!list)
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_LBRACE))
	{
		return NULL;
	}

	while (parser->token->type != TOKEN_RBRACE)
	{
		node_t *decl = NULL;
		switch (parser->token->type)
		{
		case TOKEN_READONLY_KEYWORD:
			decl = parser_class_readonly(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_STATIC_KEYWORD:
			decl = parser_class_static(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_EXPORT_KEYWORD:
			decl = parser_class_export(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_ID:
			decl = parser_class_property(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_FUNC_KEYWORD:
			decl = parser_class_func(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_CLASS_KEYWORD:
			decl = parser_class(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_ENUM_KEYWORD:
			decl = parser_enum(program, parser, PARSER_MODIFIER_NONE);
			break;

		default:
			parser_error(program, position, "illegal declaration type");
			break;
		}

		if (!decl)
		{
			return NULL;
		}

		if (!(list_rpush(list, (uint64_t)decl)))
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_RBRACE))
	{
		return NULL;
	}

	return list;
}

static node_t *
parser_heritage(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}
	
	if (!parser_match(program, parser, TOKEN_COLON))
	{
		return NULL;
	}
	
	node_t *type;
	type = parser_expression(program, parser);
	if (!type)
	{
		return NULL;
	}

	return node_make_heritage(position, key, type);
}

static list_t *
parser_heritages(program_t *program, parser_t *parser)
{
	list_t *heritage_list;
	heritage_list = list_create();
	if (!heritage_list)
	{
		return NULL;
	}

	node_t *node;
	while (true)
	{
		node = parser_heritage(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(heritage_list, (uint64_t)node))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	return heritage_list;
}

static node_t *
parser_class(program_t *program, parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_CLASS_KEYWORD))
	{
		return NULL;
	}

	list_t *generics = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RPAREN)
		{
			parser_error(program, parser->token->position, "type parameters");
			return NULL;
		}

		generics = parser_generics(program, parser);
		if (!generics)
		{
			return NULL;
		}

		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	node_t *key;
	key = parser_id(program, parser);
	if (!key)
	{
		return NULL;
	}

	list_t *heritages = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		if (!parser_match(program, parser, TOKEN_LPAREN))
		{
			return NULL;
		}

		if (parser->token->type == TOKEN_RPAREN)
		{
			parser_error(program, parser->token->position, "empty heritages");
			return NULL;
		}

		heritages = parser_heritages(program, parser);
		if (!heritages)
		{
			return NULL;
		}

		if (!parser_match(program, parser, TOKEN_RPAREN))
		{
			return NULL;
		}

	}

	list_t *body;
	body = parser_class_block(program, parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_class(position, flag, key, generics, heritages, body);
}

static node_t *
parser_field(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	node_t *key;
	key = parser_expression(program, parser);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if (!parser_next(program, parser))
		{
			return NULL;
		}

		type = parser_expression(program, parser);
		if (!type)
		{
			return NULL;
		}
	}

	return node_make_field(position, key, type);
}

static list_t *
parser_fields(program_t *program, parser_t *parser)
{
	list_t *fields;
	fields = list_create();
	if (!fields)
	{
		return NULL;
	}

	node_t *node;
	while (true)
	{
		node = parser_field(program, parser);
		if (!node)
		{
			return NULL;
		}

		if (!list_rpush(fields, (uint64_t)node))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (!parser_next(program, parser))
		{
			return NULL;
		}
	}

	return fields;
}

static node_t *
parser_import(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_IMPORT_KEYWORD))
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_LPAREN))
	{
		return NULL;
	}

	node_t *path;
	path = parser_string(program, parser);
	if (!path)
	{
		return NULL;
	}

	list_t *fields = NULL;
	if (parser->token->type == TOKEN_COMMA)
	{
		if (!parser_match(program, parser, TOKEN_COMMA))
		{
			return NULL;
		}

		fields = parser_fields(program, parser);
		if (!fields)
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	if (!parser_match(program, parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_import(position, path, fields);
}

static node_t *
parser_export(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	if (!parser_match(program, parser, TOKEN_EXPORT_KEYWORD))
	{
		return NULL;
	}

	uint64_t flag = PARSER_MODIFIER_EXPORT;

	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_CLASS_KEYWORD:
		node = parser_class(program, parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(program, parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func_stmt(program, parser, flag);
		break;

	case TOKEN_VAR_KEYWORD:
		node = parser_var_stmt(program, parser, flag);
		break;

	case TOKEN_READONLY_KEYWORD:
		node = parser_readonly_stmt(program, parser, flag);
		break;

	case TOKEN_LBRACE:
		node = parser_object(program, parser, flag);
		break;

	default:
		parser_error(program, position, "illegal declaration");
		break;
	}

	flag &= ~PARSER_MODIFIER_EXPORT;

	return node;
}

node_t *
parser_module(program_t *program, parser_t *parser)
{
	position_t position = parser->token->position;

	list_t *members;
	members = list_create();
	if (!members)
	{
		return NULL;
	}

	node_t *decl;
	while (parser->token->type != TOKEN_EOF)
	{

		if (parser->token->type == TOKEN_SEMICOLON)
		{
			if (!parser_next(program, parser))
			{
				return NULL;
			}
			continue;
		}

		switch (parser->token->type)
		{
		case TOKEN_EXPORT_KEYWORD:
			decl = parser_export(program, parser);
			break;

		case TOKEN_IMPORT_KEYWORD:
			decl = parser_import(program, parser);
			break;

		case TOKEN_CLASS_KEYWORD:
			decl = parser_class(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_ENUM_KEYWORD:
			decl = parser_enum(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_FUNC_KEYWORD:
			decl = parser_func_stmt(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_VAR_KEYWORD:
			decl = parser_var_stmt(program, parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_READONLY_KEYWORD:
			decl = parser_readonly_stmt(program, parser, PARSER_MODIFIER_NONE);
			break;

		default:
			parser_error(program, parser->token->position, "non-declaration statement outside function body");
			return NULL;
		}

		if (!decl)
		{
			return NULL;
		}

		if (!list_rpush(members, (uint64_t)decl))
		{
			return NULL;
		}
	}

	if (!parser_match(program, parser, TOKEN_EOF))
	{
		return NULL;
	}

	return node_make_module(position, parser->file_source->path, members);
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
