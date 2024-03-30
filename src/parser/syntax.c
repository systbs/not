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
#include "syntax.h"
#include "error.h"

static node_t *
syntax_expression(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_statement(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_body(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_class(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
syntax_func(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
syntax_parameters(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_generics(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_parameter(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_property(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag);

static node_t *
syntax_parenthesis(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_heritages(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_prefix(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_export(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);

static node_t *
syntax_postfix(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent);




int32_t
syntax_save(program_t *program, syntax_t *syntax)
{
	scanner_t *scanner = syntax->scanner;

	syntax_state_t *state = (syntax_state_t *)malloc(sizeof(syntax_state_t));
	if (state == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(syntax_state_t));
		return -1;
	}

	state->loop_depth = syntax->loop_depth;
	state->offset = scanner->offset;
	state->reading_offset = scanner->reading_offset;
	state->line = scanner->line;
	state->column = scanner->column;
	state->ch = scanner->ch;
	state->token = scanner->token;
	state->count_error = list_count(program->errors);

	if (list_rpush(syntax->states, state) == NULL)
	{
		return -1;
	}

	return 1;
}

int32_t
syntax_restore(program_t *program, syntax_t *syntax)
{
	scanner_t *scanner = syntax->scanner;

	ilist_t *r1 = list_rpop(syntax->states);
	if (r1 == NULL)
	{
		return -1;
	}

	syntax_state_t *state = (syntax_state_t *)r1->value;
	if (state == NULL)
	{
		return -1;
	}

	syntax->loop_depth = state->loop_depth;

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
syntax_release(program_t *program, syntax_t *syntax)
{
	ilist_t *r1 = list_rpop(syntax->states);
	if (r1 == NULL)
	{
		return -1;
	}

	syntax_state_t *state = (syntax_state_t *)r1->value;
	if (state == NULL)
	{
		return -1;
	}

	free(state);
	return 1;
}

static error_t *
syntax_error(program_t *program, position_t position, const char *format, ...)
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
syntax_expected(program_t *program, syntax_t *syntax, int32_t type)
{
	char *message;
	message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	switch (syntax->token->type)
	{
	case TOKEN_ID:
	case TOKEN_NUMBER:
	case TOKEN_CHAR:
	case TOKEN_STRING:
		sprintf(
				message,
				"expected '%s', got '%s'\n",
				token_get_name(type),
				syntax->token->value);
		break;

	default:
		sprintf(
				message,
				"expected '%s', got '%s'\n",
				token_get_name(type),
				token_get_name(syntax->token->type));
		break;
	}

	error_t *error;
	error = error_create(syntax->token->position, message);
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
syntax_match(program_t *program, syntax_t *syntax, int32_t type)
{
	if (syntax->token->type == type)
	{
		if (scanner_advance(syntax->scanner) == -1)
		{
			return -1;
		}
	}
	else
	{
		syntax_expected(program, syntax, type);
		return -1;
	}
	return 1;
}

static int32_t
syntax_next(program_t *program, syntax_t *syntax)
{
	if (scanner_advance(syntax->scanner) == -1)
	{
		return -1;
	}
	return 1;
}

static int32_t
syntax_gt(program_t *program, syntax_t *syntax)
{
	return scanner_gt(syntax->scanner);
}

static int32_t
syntax_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}




static node_t *
syntax_id(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_id(node, syntax->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_ID) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_number(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_number(node, syntax->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_NUMBER) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_char(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_char(node, syntax->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_CHAR) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_string(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_string(node, syntax->token->value);
	if (!node2)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_STRING) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_null(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_null(node);
	if (!node2)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_NULL_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_infinity(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_infinity(node);
	if (!node2)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INFINITY_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_operator(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	char *operator = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_match(program, syntax, TOKEN_EQ) == -1)
		{
			return NULL;
		}
		operator = "=";
	}
	else 
	if (syntax->token->type == TOKEN_PLUS)
	{
		if (syntax_match(program, syntax, TOKEN_PLUS) == -1)
		{
			return NULL;
		}
		operator = "+";
	}
	else 
	if (syntax->token->type == TOKEN_MINUS)
	{
		if (syntax_match(program, syntax, TOKEN_MINUS) == -1)
		{
			return NULL;
		}
		operator = "-";
	}
	else 
	if (syntax->token->type == TOKEN_TILDE)
	{
		if (syntax_match(program, syntax, TOKEN_TILDE) == -1)
		{
			return NULL;
		}
		operator = "~";
	}
	else 
	if (syntax->token->type == TOKEN_NOT)
	{
		if (syntax_match(program, syntax, TOKEN_NOT) == -1)
		{
			return NULL;
		}
		operator = "!";
	}
	else 
	if (syntax->token->type == TOKEN_STAR)
	{
		if (syntax_match(program, syntax, TOKEN_STAR) == -1)
		{
			return NULL;
		}
		operator = "*";
	}
	else 
	if (syntax->token->type == TOKEN_SLASH)
	{
		if (syntax_match(program, syntax, TOKEN_SLASH) == -1)
		{
			return NULL;
		}
		operator = "/";
	}
	else 
	if (syntax->token->type == TOKEN_BACKSLASH)
	{
		if (syntax_match(program, syntax, TOKEN_BACKSLASH) == -1)
		{
			return NULL;
		}
		operator = "\\";
	}
	else 
	if (syntax->token->type == TOKEN_POWER)
	{
		if (syntax_match(program, syntax, TOKEN_POWER) == -1)
		{
			return NULL;
		}
		operator = "**";
	}
	else 
	if (syntax->token->type == TOKEN_PERCENT)
	{
		if (syntax_match(program, syntax, TOKEN_PERCENT) == -1)
		{
			return NULL;
		}
		operator = "%";
	}
	else 
	if (syntax->token->type == TOKEN_AND)
	{
		if (syntax_match(program, syntax, TOKEN_AND) == -1)
		{
			return NULL;
		}
		operator = "&";
	}
	else 
	if (syntax->token->type == TOKEN_OR)
	{
		if (syntax_match(program, syntax, TOKEN_OR) == -1)
		{
			return NULL;
		}
		operator = "|";
	}
	else 
	if (syntax->token->type == TOKEN_AND_AND)
	{
		if (syntax_match(program, syntax, TOKEN_AND_AND) == -1)
		{
			return NULL;
		}
		operator = "&&";
	}
	else 
	if (syntax->token->type == TOKEN_OR_OR)
	{
		if (syntax_match(program, syntax, TOKEN_OR_OR) == -1)
		{
			return NULL;
		}
		operator = "||";
	}
	else 
	if (syntax->token->type == TOKEN_CARET)
	{
		if (syntax_match(program, syntax, TOKEN_CARET) == -1)
		{
			return NULL;
		}
		operator = "^";
	}
	else 
	if (syntax->token->type == TOKEN_LT_LT)
	{
		if (syntax_match(program, syntax, TOKEN_LT_LT) == -1)
		{
			return NULL;
		}
		operator = "<<";
	}
	else 
	if (syntax->token->type == TOKEN_GT_GT)
	{
		if (syntax_match(program, syntax, TOKEN_GT_GT) == -1)
		{
			return NULL;
		}
		operator = ">>";
	}
	else 
	if (syntax->token->type == TOKEN_LT)
	{
		if (syntax_match(program, syntax, TOKEN_LT) == -1)
		{
			return NULL;
		}
		operator = "<";
	}
	else 
	if (syntax->token->type == TOKEN_GT)
	{
		if (syntax_match(program, syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
		operator = ">";
	}
	else 
	if (syntax->token->type == TOKEN_LBRACKET)
	{
		if (syntax_match(program, syntax, TOKEN_LBRACKET) == -1)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_RBRACKET) == -1)
		{
			return NULL;
		}
		operator = "[]";
	}
	else 
	{
		syntax_error(program, syntax->token->position, "operator expected\n");
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
syntax_array(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACKET) == -1)
	{
		return NULL;
	}

	list_t *items = list_create();
	if (!items)
	{
		return NULL;
	}

	if (syntax->token->type != TOKEN_RBRACKET)
	{
		while (true)
		{
			node_t *expr = syntax_expression(program, syntax, scope, node);
			if (!expr)
			{
				return NULL;
			}

			if (!list_rpush(items, expr))
			{
				return NULL;
			}

			if (syntax->token->type != TOKEN_COMMA)
			{
				break;
			}
			if (syntax_match(program, syntax, TOKEN_COMMA) == -1)
			{
				return NULL;
			}
		}
	}

	if (syntax_match(program, syntax, TOKEN_RBRACKET) == -1)
	{
		return NULL;
	}

	return node_make_array(node, items);
}

static node_t *
syntax_pair(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *value = syntax_expression(program, syntax, scope, node);
	if (!value)
	{
		return NULL;
	}

	return node_make_pair(node, key, value);
}

static node_t *
syntax_object(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	list_t *object = list_create();
	if (!object)
	{
		return NULL;
	}

	if (syntax->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *pair = syntax_pair(program, syntax, scope, node);
			if (pair == NULL)
			{
				return NULL;
			}

			if (list_rpush(object, pair) == NULL)
			{
				return NULL;
			}

			if (syntax->token->type != TOKEN_COMMA)
			{
				break;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}
		}
	}

	if (syntax_match(program, syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_object(node, object);
}

static node_t *
syntax_parenthesis(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	node_t *value = syntax_expression(program, syntax, scope, node);
	if (!value)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	return node_make_parenthesis(node, value);
}

static node_t *
syntax_lambda(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FUN_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = syntax_id(program, syntax, node, node);
		if (!key)
		{
			return NULL;
		}
		
		if (syntax_idstrcmp(key, "Constructor") == 1)
		{
			syntax_error(program, syntax->token->position, "wrong name");
			return NULL;
		}
	}

	node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			syntax_error(program, syntax->token->position, "empty generic types");
			return NULL;
		}

		generics = syntax_generics(program, syntax, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}
		
		if (syntax_match(program, syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = syntax_parameters(program, syntax, scope, node);
			if (!parameters)
			{
				return NULL;
			}
		}

		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	node_t *body = NULL;
	if (syntax->token->type == TOKEN_MINUS_GT)
	{
		if (syntax_match(program, syntax, TOKEN_MINUS_GT) == -1)
		{
			return NULL;
		}
		body = syntax_expression(program, syntax, scope, node);
		if (!body)
		{
			return NULL;
		}
	}
	else
	{
		body = syntax_body(program, syntax, scope, node);
		if (!body)
		{
			return NULL;
		}
	}

	return node_make_lambda(node, key, generics, parameters, body);
}

static node_t *
syntax_primary(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	if (syntax->token->type == TOKEN_NUMBER)
	{
		return syntax_number(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_STRING)
	{
		return syntax_string(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_CHAR)
	{
		return syntax_char(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_NULL_KEYWORD)
	{
		return syntax_null(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INFINITY_KEYWORD)
	{
		return syntax_infinity(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LBRACKET)
	{
		return syntax_array(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LBRACE)
	{
		return syntax_object(program, syntax, scope, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LPAREN)
	{
		return syntax_parenthesis(program, syntax, scope, parent);
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		return syntax_lambda(program, syntax, scope, parent);
	}
	else 
	{
		return syntax_id(program, syntax, scope, parent);
	}
}

static node_t *
syntax_field(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_postfix(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		value = syntax_postfix(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_field(node, key, value);
}

static node_t *
syntax_fields(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
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
		node_t *node2 = syntax_field(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(fields, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_fields(node, fields);
}

static node_t *
syntax_argument(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_expression(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		value = syntax_expression(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_argument(node, key, value);
}

static node_t *
syntax_arguments(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *argements = list_create();
	if (argements == NULL)
	{
		return NULL;
	}

	node_t *node2;
	while (true)
	{
		node2 = syntax_argument(program, syntax, scope, node);
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(argements, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_arguments(node, argements);
}

static node_t *
syntax_postfix(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_primary(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}

	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_LT)
		{
			if (syntax_save(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			if (syntax_gt(program, syntax) == -1)
			{
				return NULL;
			}

			if (syntax->token->type == TOKEN_GT)
			{
				syntax_error(program, syntax->token->position, "empty generic types");
				return NULL;
			}

			node_t *concepts = syntax_fields(program, syntax, scope, node);
			if (concepts == NULL)
			{
				if (syntax_restore(program, syntax) == -1)
				{
					return NULL;
				}
				node_destroy(node);
				return node2;
			}
			else
			{
				if (syntax_gt(program, syntax) == -1)
				{
					return NULL;
				}

				if (syntax->token->type != TOKEN_GT)
				{
					if (syntax_restore(program, syntax) == -1)
					{
						return NULL;
					}
					node_destroy(node);
					return node2;
				}
				else
				{
					if (syntax_match(program, syntax, TOKEN_GT) == -1)
					{
						return NULL;
					}

					if (syntax_release(program, syntax) == -1)
					{
						return NULL;
					}

					node2 = node_make_pseudonym(node, node2, concepts);
					continue;
				}
			}
			continue;
		}
		else
		if (syntax->token->type == TOKEN_LBRACKET)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			if (syntax->token->type == TOKEN_RBRACKET)
			{
				syntax_error(program, syntax->token->position, "empty array");
				return NULL;
			}

			node_t *argements = syntax_arguments(program, syntax, scope, node);
			if (argements == NULL)
			{
				return NULL;
			}

			if (syntax_match(program, syntax, TOKEN_RBRACKET) == -1)
			{
				return NULL;
			}

			node2 = node_make_item(node, node2, argements);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_DOT)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}
	
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_id(program, syntax, scope, node);
			if (!right)
			{
				return NULL;
			}
      
			node2 = node_make_attribute(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_LPAREN)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}	

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *argements = NULL;
			if (syntax->token->type != TOKEN_RPAREN)
			{
				argements = syntax_arguments(program, syntax, scope, node);
				if (argements == NULL)
				{
					return NULL;
				}
			}

			if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
			{
				return NULL;
			}

			node2 = node_make_call(node, node2, argements);
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
syntax_prefix(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	if (syntax->token->type == TOKEN_TILDE)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_prefix(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_tilde(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_NOT)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_not(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_MINUS)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_neg(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_PLUS)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_pos(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_SIZEOF_KEYWORD)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_sizeof(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_TYPEOF_KEYWORD)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_typeof(node, right);
	}
	else
	{
		return syntax_postfix(program, syntax, scope, parent);
	}
}

static node_t *
syntax_power(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_prefix(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_POWER)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_prefix(program, syntax, scope, node);
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
syntax_multiplicative(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_power(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_STAR)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_mul(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_SLASH)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			
			
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_div(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_PERCENT)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			
			
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_mod(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_BACKSLASH)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			
			
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, scope, node);
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
syntax_addative(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_multiplicative(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}	
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_PLUS)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_multiplicative(program, syntax, scope, node);
			if (!right)
			{
				return NULL;
			}

			node2 = node_make_plus(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_MINUS)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_multiplicative(program, syntax, scope, node);
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
syntax_shifting(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_addative(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_LT_LT)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_addative(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_shl(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_GT)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_addative(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_shr(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_relational(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_shifting(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_LT)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_lt(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_LT_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_le(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_gt(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_ge(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_equality(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_relational(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_EQ_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_relational(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_eq(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_NOT_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_relational(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_neq(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_bitwise_and(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_equality(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_AND)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_equality(program, syntax, scope, node);
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
syntax_bitwise_xor(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_bitwise_and(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_CARET)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_bitwise_and(program, syntax, scope, node);
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
syntax_bitwise_or(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_bitwise_xor(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_OR)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_bitwise_xor(program, syntax, scope, node);
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
syntax_logical_and(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_bitwise_or(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_AND_AND)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_bitwise_or(program, syntax, scope, node);
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
syntax_logical_or(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_logical_and(program, syntax, scope, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_OR_OR)
		{
			node_t *node = node_create(scope, parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}
			
			node_t *right = syntax_logical_and(program, syntax, scope, node);
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
syntax_expression(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	return syntax_logical_or(program, syntax, scope, parent);
}

static node_t *
syntax_assign(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node2 = syntax_expression(program, syntax, scope, parent);
	if (!node2)
	{
		return NULL;
	}

	if (syntax->token->type == TOKEN_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_PLUS_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_add_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_MINUS_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_sub_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_STAR_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_mul_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_SLASH_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_div_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_PERCENT_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_mod_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_AND_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_and_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_OR_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_or_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_LT_LT_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_shl_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_GT_EQ)
	{
		node_t *node = node_create(scope, parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, scope, node);
		if (!right)
		{
			return NULL;
		}

		return node_make_shr_assign(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_if(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_IF_KEYWORD) == -1)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	node_t *condition;
	condition = syntax_expression(program, syntax, scope, node);
	if (!condition)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	node_t *then_body;
	then_body = syntax_body(program, syntax, scope, node);
	if (!then_body)
	{
		return NULL;
	}

	node_t *else_body;
	else_body = NULL;
	if (syntax->token->type == TOKEN_ELSE_KEYWORD)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_IF_KEYWORD)
		{
			else_body = syntax_if(program, syntax, scope, node);
		}
		else
		{
			else_body = syntax_body(program, syntax, scope, node);
		}

		if (!else_body)
		{
			return NULL;
		}
	}

	return node_make_if(node, condition, then_body, else_body);
}

static node_t *
syntax_entiery(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (key == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *type = syntax_postfix(program, syntax, scope, node);
	if (type == NULL)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		value = syntax_expression(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_entiery(node, flag, key, type, value);
}

static node_t *
syntax_set(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	list_t *set = list_create();
	if (set == NULL)
	{
		return NULL;
	}

	if (syntax->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *entiery = syntax_entiery(program, syntax, scope, node, flag);
			if (!entiery)
			{
				return NULL;
			}

			if (!list_rpush(set, entiery))
			{
				return NULL;
			}

			if (syntax->token->type != TOKEN_COMMA)
			{
				break;
			}
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}
		}
	}

	if (syntax_match(program, syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_set(node, set);
}

static node_t *
syntax_var(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_VAR_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t object_used = 0;
	node_t *key = NULL;
	if (syntax->token->type != TOKEN_ID)
	{
		key = syntax_set(program, syntax, scope, node, flag);
		if (!key)
		{
			return NULL;
		}
		object_used = 1;
	}
	else
	{
		key = syntax_id(program, syntax, scope, node);
		if (!key)
		{
			return NULL;
		}
	}

	node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		if (object_used == 1)
		{
			syntax_error(program, syntax->token->position, "multiple variables with type");
			return NULL;
		}
		type = syntax_expression(program, syntax, scope, node);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		value = syntax_expression(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_var(node, flag, key, type, value);
}

static node_t *
syntax_for(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FOR_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = syntax_id(program, syntax, scope, node);
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
	if (syntax->token->type == TOKEN_LBRACE)
	{
		syntax->loop_depth += 1;

		node_t *body = syntax_body(program, syntax, scope, node);
		if (!body)
		{
			return NULL;
		}

		syntax->loop_depth -= 1;

		node_t *node2 = node_create(scope, node, syntax->token->position);
		if (node2 == NULL)
		{
			return NULL;
		}

		node_t *initializer_block = node_make_block(node2, initializer);
		if (initializer_block == NULL)
		{
			return NULL;
		}

		node_t *node3 = node_create(scope, node, syntax->token->position);
		if (node3 == NULL)
		{
			return NULL;
		}

		node_t *incrementor_block = node_make_block(node3, incrementor);
		if (incrementor_block == NULL)
		{
			return NULL;
		}

		return node_make_for(node, flag, key, initializer_block, condition, incrementor_block, body);
	}

	if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	if (syntax->token->type == TOKEN_SEMICOLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		goto region_condition;
	}

	int32_t use_readonly = 0;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		use_readonly = 1;
	}

	while (true)
	{
		node_t *node2 = NULL;
		if (use_readonly || (syntax->token->type == TOKEN_VAR_KEYWORD))
		{
			node2 = syntax_var(program, syntax, scope, node, use_readonly ? SYNTAX_MODIFIER_READONLY:SYNTAX_MODIFIER_NONE);
		}
		else
		{
			node2 = syntax_assign(program, syntax, scope, node);
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

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
	{
		return NULL;
	}

	region_condition:
	if (syntax->token->type == TOKEN_SEMICOLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		goto region_step;
	}

	condition = syntax_expression(program, syntax, scope, node);
	if (!condition)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
	{
		return NULL;
	}

	region_step:
	if (syntax->token->type == TOKEN_RPAREN)
	{
		goto region_finish;
	}

	while (true)
	{
		node_t *node2 = syntax_assign(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(incrementor, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	region_finish:
	if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	syntax->loop_depth += 1;

	node_t *body = syntax_body(program, syntax, scope, node);
	if (!body)
	{
		return NULL;
	}

	syntax->loop_depth -= 1;

	node_t *node2 = node_create(scope, node, syntax->token->position);
	if (node2 == NULL)
	{
		return NULL;
	}

	node_t *initializer_block = node_make_initializer(node2, initializer);
	if (initializer_block == NULL)
	{
		return NULL;
	}

	node_t *node3 = node_create(scope, node, syntax->token->position);
	if (node3 == NULL)
	{
		return NULL;
	}

	node_t *incrementor_block = node_make_incrementor(node3, incrementor);
	if (incrementor_block == NULL)
	{
		return NULL;
	}

	return node_make_for(node, flag, key, initializer_block, condition, incrementor_block, body);

	region_forin:
	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *iterator = syntax_expression(program, syntax, scope, node);
	if (!iterator)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	syntax->loop_depth += 1;

	body = syntax_body(program, syntax, scope, node);
	if (!body)
	{
		return NULL;
	}

	syntax->loop_depth -= 1;
	initializer_block = node_make_initializer(node, initializer);
	if (initializer_block == NULL)
	{
		return NULL;
	}

	return node_make_forin(node, flag, key, initializer_block, iterator, body);
}

static node_t *
syntax_break(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_next(program, syntax) == -1)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_BREAK_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = syntax_id(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_break(node, value);
}

static node_t *
syntax_continue(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_next(program, syntax) == -1)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_CONTINUE_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = syntax_id(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_continue(node, value);
}

static node_t *
syntax_catch(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_CATCH_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = syntax_parameters(program, syntax, scope, node);
			if (!parameters)
			{
				return NULL;
			}
		}
		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	node_t *body;
	body = syntax_body(program, syntax, scope, node);
	if (!body)
	{
		return NULL;
	}

	return node_make_catch(node, parameters, body);
}

static node_t *
syntax_catchs(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *list = list_create();
	if (!list)
	{
		return NULL;
	}
	while (syntax->token->type == TOKEN_CATCH_KEYWORD)
	{
		node_t *node2 = syntax_catch(program, syntax, scope, node);
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
syntax_try(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_TRY_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			syntax_error(program, syntax->token->position, "empty generic types");
			return NULL;
		}

		generics = syntax_generics(program, syntax, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}
		
		if (syntax_match(program, syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	node_t *body = syntax_body(program, syntax, scope, node);
	if (!body)
	{
		return NULL;
	}

	node_t *catchs = NULL;
	if (syntax->token->type == TOKEN_CATCH_KEYWORD)
	{
		catchs = syntax_catchs(program, syntax, scope, node);
		if (!catchs)
		{
			return NULL;
		}
	}

	return node_make_try(node, key, generics, body, catchs);
}

static node_t *
syntax_return(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RETURN_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	value = syntax_expression(program, syntax, scope, node);
	if (!value)
	{
		return NULL;
	}

	return node_make_return(node, value);
}

static node_t *
syntax_body(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	list_t *stmt_list = list_create();
	if (!stmt_list)
	{
		return NULL;
	}

	while (syntax->token->type != TOKEN_RBRACE)
	{
		if (syntax->token->type == TOKEN_SEMICOLON)
		{
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			continue;
		}

		node_t *stmt = syntax_statement(program, syntax, scope, node);
		if (!stmt)
		{
			return NULL;
		}

		if (!list_rpush(stmt_list, stmt))
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_body(node, stmt_list);
}

static node_t *
syntax_parameter(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_NONE;
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		flag |= SYNTAX_MODIFIER_REFERENCE;
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}
	else 
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		flag |= SYNTAX_MODIFIER_READONLY;
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *type = syntax_expression(program, syntax, scope, node);
	if (!type)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if ((flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
		{
			syntax_error(program, syntax->token->position, "reference parameter by value");
			return NULL;
		}

		value = syntax_expression(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_parameter(node, flag, key, type, value);
}

static node_t *
syntax_parameters(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
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
		node_t *node2 = syntax_parameter(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(parameters, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_parameters(node, parameters);
}

static node_t *
syntax_generic(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key;
	key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (syntax->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		type = syntax_postfix(program, syntax, scope, node);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		value = syntax_postfix(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_generic(node, key, type, value);
}

static node_t *
syntax_generics(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
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
		node_t *node2 = syntax_generic(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(generics, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_generics(node, generics);
}

static node_t *
syntax_func(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FUN_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t used_constructor = 0, used_operator = 0;
	node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = syntax_id(program, syntax, node, node);
		if (!key)
		{
			return NULL;
		}
		
		if (syntax_idstrcmp(key, "Constructor") == 1)
		{
			used_constructor = 1;
		}
	}
	else
	{
		key = syntax_operator(program, syntax, node, node);
		if (key == NULL)
		{
			return NULL;
		}

		used_operator = 1;
	}

	node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (used_constructor == 1)
		{
			syntax_error(program, syntax->token->position, "Constructor with generic types");
			return NULL;
		}

		if (used_operator == 1)
		{
			syntax_error(program, syntax->token->position, "operator with generic types");
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			syntax_error(program, syntax->token->position, "empty generic types");
			return NULL;
		}

		generics = syntax_generics(program, syntax, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}
		
		if (syntax_match(program, syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = syntax_parameters(program, syntax, scope, node);
			if (!parameters)
			{
				return NULL;
			}
		}

		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	node_t *result = NULL;
	if (used_constructor == 0)
	{
		if (syntax_match(program, syntax, TOKEN_COLON) == -1)
		{
			return NULL;
		}
		result = syntax_expression(program, syntax, scope, node);
		if (result == NULL)
		{
			return NULL;
		}
	}

	node_t *body = syntax_body(program, syntax, scope, node);
	if (!body)
	{
		return NULL;
	}

	return node_make_func(node, flag, key, generics, parameters, result, body);
}

static node_t *
syntax_readonly(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	node_t *node = NULL;
	switch (syntax->token->type)
	{
	case TOKEN_VAR_KEYWORD:
		node = syntax_var(program, syntax, scope, parent, flag);
		break;

	default:
		syntax_error(program, syntax->token->position, "incorrect use of modifier 'readonly'");
		break;
	}

	flag &= ~SYNTAX_MODIFIER_READONLY;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
syntax_async(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_ASYNC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_ASYNC;

	node_t *node = NULL;
	switch (syntax->token->type)
	{
	case TOKEN_FOR_KEYWORD:
		node = syntax_for(program, syntax, scope, parent, flag);
		break;

	default:
		syntax_error(program, syntax->token->position, "incorrect use of modifier 'async'");
		break;
	}

	flag &= ~SYNTAX_MODIFIER_ASYNC;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
syntax_statement(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = NULL;
	if (syntax->token->type == TOKEN_IF_KEYWORD)
	{
		node = syntax_if(program, syntax, scope, parent);
	}
	else
	if (syntax->token->type == TOKEN_TRY_KEYWORD)
	{
		node = syntax_try(program, syntax, scope, parent);
	}
	else
	if (syntax->token->type == TOKEN_FOR_KEYWORD)
	{
		node = syntax_for(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
	}
	else
	if (syntax->token->type == TOKEN_VAR_KEYWORD)
	{
		node = syntax_var(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_readonly(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
	}
	else
	if (syntax->token->type == TOKEN_ASYNC_KEYWORD)
	{
		node = syntax_async(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
	}
	else
	if (syntax->token->type == TOKEN_BREAK_KEYWORD)
	{
		node = syntax_break(program, syntax, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CONTINUE_KEYWORD)
	{
		node = syntax_continue(program, syntax, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_RETURN_KEYWORD)
	{
		node = syntax_return(program, syntax, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_assign(program, syntax, scope, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	return node;
}

static node_t *
syntax_member(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		value = syntax_expression(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_member(node, key, value);
}

static node_t *
syntax_members(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *members = list_create();
	if (!members)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = syntax_member(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(members, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_members(node, members);
}

static node_t *
syntax_enum(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_ENUM_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key;
	key = syntax_id(program, syntax, node, node);
	if (!key)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	node_t *members = syntax_members(program, syntax, node, node);
	if (!members)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_enum(node, flag, key, members);
}

static node_t *
syntax_property(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (key == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *type = syntax_postfix(program, syntax, scope, node);
	if (!type)
	{
		return NULL;
	}

	node_t *set = NULL, *get = NULL;
	if (syntax->token->type == TOKEN_AT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_RPAREN)
		{
			syntax_error(program, syntax->token->position, "forgotten accessories");
			return NULL;
		}

		if (syntax->token->type != TOKEN_ID)
		{
			syntax_error(program, syntax->token->position, "get/set is expected");
			return NULL;
		}

		node_t *accessor;
		accessor = syntax_id(program, syntax, scope, node);
		if (accessor == NULL)
		{
			return NULL;
		}
		
		if(syntax_idstrcmp(accessor, "set") == 1)
		{
			if (syntax_match(program, syntax, TOKEN_EQ) == -1)
			{
				return NULL;
			}

			set = syntax_postfix(program, syntax, scope, node);
			if (set == NULL)
			{
				return NULL;
			}

			if (syntax->token->type == TOKEN_COMMA)
			{
				if (syntax_next(program, syntax) == -1)
				{
					return NULL;
				}

				if (syntax->token->type != TOKEN_ID)
				{
					syntax_error(program, syntax->token->position, "get is expected");
					return NULL;
				}

				accessor = syntax_id(program, syntax, scope, node);
				if (accessor == NULL)
				{
					return NULL;
				}

				if(syntax_idstrcmp(accessor, "get") == 1)
				{
					if (syntax_match(program, syntax, TOKEN_EQ) == -1)
					{
						return NULL;
					}

					get = syntax_postfix(program, syntax, scope, node);
					if (get == NULL)
					{
						return NULL;
					}
				}
				else
				{
					syntax_error(program, syntax->token->position, "get is expected");
					return NULL;
				}
			}
		}
		else
		if(syntax_idstrcmp(accessor, "get") == 1)
		{
			if (syntax_match(program, syntax, TOKEN_EQ) == -1)
			{
				return NULL;
			}

			get = syntax_postfix(program, syntax, scope, node);
			if (get == NULL)
			{
				return NULL;
			}

			if (syntax->token->type == TOKEN_COMMA)
			{
				if (syntax_next(program, syntax) == -1)
				{
					return NULL;
				}

				if (syntax->token->type != TOKEN_ID)
				{
					syntax_error(program, syntax->token->position, "set is expected");
					return NULL;
				}

				accessor = syntax_id(program, syntax, scope, node);
				if (accessor == NULL)
				{
					return NULL;
				}

				if(syntax_idstrcmp(accessor, "set") == 1)
				{
					if (syntax_match(program, syntax, TOKEN_EQ) == -1)
					{
						return NULL;
					}

					set = syntax_postfix(program, syntax, scope, node);
					if (set == NULL)
					{
						return NULL;
					}
				}
				else
				{
					syntax_error(program, syntax->token->position, "set is expected");
					return NULL;
				}
			}
		}
		else
		{
			syntax_error(program, syntax->token->position, "get/set is expected");
			return NULL;
		}


		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if ((syntax->token->type == TOKEN_EQ) || ((flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC))
	{
		if (syntax_match(program, syntax, TOKEN_EQ) == -1)
		{
			return NULL;
		}
		value = syntax_expression(program, syntax, scope, node);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_property(node, flag, key, type, value, set, get);
}

static node_t *
syntax_class_readonly(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		node = syntax_property(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	{
		syntax_error(program, syntax->token->position, "incorrect use of modifier 'readonly'");
		return NULL;
	}

	flag &= ~SYNTAX_MODIFIER_READONLY;

	

	return node;
}

static node_t *
syntax_class_static(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_class_readonly(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_func(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = syntax_class(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_ENUM_KEYWORD)
	{
		node = syntax_enum(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_property(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	flag &= ~SYNTAX_MODIFIER_STATIC;

	return node;
}

static node_t *
syntax_class_protected(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_PROTECTED_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_PROTECT;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_class_readonly(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = syntax_class_static(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_func(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = syntax_class(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_ENUM_KEYWORD)
	{
		node = syntax_enum(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_property(program, syntax, scope, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	flag &= ~SYNTAX_MODIFIER_PROTECT;

	return node;
}

static node_t *
syntax_class_export(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_EXPORT;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_class_readonly(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_PROTECTED_KEYWORD)
	{
		node = syntax_class_protected(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = syntax_class_static(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_func(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = syntax_class(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_ENUM_KEYWORD)
	{
		node = syntax_enum(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_property(program, syntax, scope, parent, flag);
		if (!node)
		{
			return NULL;
		}
		if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	

	return node;
}

static node_t *
syntax_class_block(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *list = list_create();
	if (!list)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	while (syntax->token->type != TOKEN_RBRACE)
	{
		node_t *decl = NULL;
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			decl = syntax_class_readonly(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_STATIC_KEYWORD)
		{
			decl = syntax_class_static(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			decl = syntax_class_export(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_FUN_KEYWORD)
		{
			decl = syntax_func(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_CLASS_KEYWORD)
		{
			decl = syntax_class(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_ENUM_KEYWORD)
		{
			decl = syntax_enum(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		{
			decl = syntax_property(program, syntax, scope, parent, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
			if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
			{
				return NULL;
			}
		}

		if (list_rpush(list, decl) == NULL)
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return node_make_block(node, list);
}

static node_t *
syntax_heritage(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}
	
	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}
	
	node_t *type = syntax_expression(program, syntax, scope, node);
	if (!type)
	{
		return NULL;
	}

	return node_make_heritage(node, key, type);
}

static node_t *
syntax_heritages(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
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
		node_t *node2 = syntax_heritage(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(heritage, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_heritages(node, heritage);
}

static node_t *
syntax_class(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_CLASS_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			syntax_error(program, syntax->token->position, "empty generic types");
			return NULL;
		}

		generics = syntax_generics(program, syntax, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	node_t *heritages = NULL;
	if (syntax->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_RPAREN)
		{
			syntax_error(program, syntax->token->position, "empty heritages");
			return NULL;
		}

		heritages = syntax_heritages(program, syntax, node, node);
		if (!heritages)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}

	}

	node_t *block = syntax_class_block(program, syntax, node, node);
	if (!block)
	{
		return NULL;
	}

	return node_make_class(node, flag, key, generics, heritages, block);
}

static node_t *
syntax_package(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, scope, node);
	if (!key)
	{
		return NULL;
	}

	node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			syntax_error(program, syntax->token->position, "empty generic types");
			return NULL;
		}

		generics = syntax_generics(program, syntax, scope, node);
		if (!generics)
		{
			return NULL;
		}

		if (syntax_gt(program, syntax) == -1)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *route = syntax_postfix(program, syntax, node, node);
	if (!route)
	{
		return NULL;
	}

	return node_make_package(node, key, generics, route);
}

static node_t *
syntax_packages(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *packages = list_create();
	if (!packages)
	{
		return NULL;
	}

	while (true)
	{
		node_t *node2 = syntax_package(program, syntax, scope, node);
		if (!node2)
		{
			return NULL;
		}

		if (!list_rpush(packages, node2))
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	return node_make_packages(node, packages);
}

static node_t *
syntax_import(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	node_t *node = node_create(scope, parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_IMPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}
	
	node_t *packages = syntax_packages(program, syntax, scope, node);
	if (!packages)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}

	node_t *path = syntax_string(program, syntax, scope, node);
	if (path == NULL)
	{
		return NULL;
	}

	return node_make_import(node, path, packages);
}

static node_t *
syntax_static(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	node_t *node = NULL;
	switch (syntax->token->type)
	{
	case TOKEN_CLASS_KEYWORD:
		node = syntax_class(program, syntax, scope, parent, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = syntax_enum(program, syntax, scope, parent, flag);
		break;

	default:
		syntax_error(program, syntax->token->position, "incorrect use of modifier 'static'");
		break;
	}

	flag &= ~SYNTAX_MODIFIER_STATIC;

	if (!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
syntax_export(program_t *program, syntax_t *syntax, node_t *scope, node_t *parent)
{
	if (syntax_match(program, syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	node_t *object = NULL;

	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		object = syntax_static(program, syntax, scope, parent, flag);
	}
	else
	if (syntax->token->type == TOKEN_ENUM_KEYWORD)
	{
		object = syntax_enum(program, syntax, scope, parent, flag);
	}
	else
	{
		object = syntax_class(program, syntax, scope, parent, flag);
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	return object;
}

node_t *
syntax_module(program_t *program, syntax_t *syntax)
{
	node_t *node = node_create(NULL, NULL, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *items = list_create();
	if (!items)
	{
		return NULL;
	}

	while (syntax->token->type != TOKEN_EOF)
	{

		if (syntax->token->type == TOKEN_SEMICOLON)
		{
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}
			continue;
		}

		node_t *item = NULL;

		if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = syntax_export(program, syntax, node, node);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_IMPORT_KEYWORD)
		{
			item = syntax_import(program, syntax, node, node);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_ENUM_KEYWORD)
		{
			item = syntax_enum(program, syntax, node, node, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		{
			item = syntax_class(program, syntax, node, node, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}

		if (!list_rpush(items, item))
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_EOF) == -1)
	{
		return NULL;
	}

	return node_make_module(node, syntax->file_source->path, items);
}

syntax_t *
syntax_create(program_t *program, char *path)
{
	syntax_t *syntax;
	syntax = (syntax_t *)malloc(sizeof(syntax_t));
	if (!syntax)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(syntax_t));
		return NULL;
	}
	memset(syntax, 0, sizeof(syntax_t));

	file_source_t *file_source = file_create_source(path);
	if (file_source == NULL)
	{
		return NULL;
	}

	scanner_t *scanner = scanner_create(file_source, program->errors);
	if (scanner == NULL)
	{
		return NULL;
	}

	syntax->scanner = scanner;
	syntax->file_source = file_source;
	syntax->loop_depth = 0;
	syntax->token = &scanner->token;

	syntax->states = list_create();
	if (syntax->states == NULL)
	{
		return NULL;
	}

	return syntax;
}
