#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../../types/types.h"
#include "../../container/list.h"
#include "../../container/stack.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../error.h"
#include "syntax.h"


static node_t *
syntax_expression(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_statement(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_body(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_class(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag);

static node_t *
syntax_fun(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag);

static node_t *
syntax_parameters(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_generics(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_parameter(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_property(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag);

static node_t *
syntax_parenthesis(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_heritages(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_prefix(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_export(program_t *program, syntax_t *syntax, node_t *parent, node_t *note);

static node_t *
syntax_postfix(program_t *program, syntax_t *syntax, node_t *parent);

static node_t *
syntax_note(program_t *program, syntax_t *syntax, node_t *parent, node_t *note);


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
	char *message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	switch (syntax->token->type)
	{
	case TOKEN_ID:
	case TOKEN_INT8:
	case TOKEN_INT16:
	case TOKEN_INT32:
	case TOKEN_INT64:

	case TOKEN_UINT8:
	case TOKEN_UINT16:
	case TOKEN_UINT32:
	case TOKEN_UINT64:
	case TOKEN_BIGINT:

	case TOKEN_FLOAT32:
	case TOKEN_FLOAT64:
	case TOKEN_BIGFLOAT:

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

	error_t *error = error_create(syntax->token->position, message);
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
		if (scanner_advance(program, syntax->scanner) == -1)
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
	if (scanner_advance(program, syntax->scanner) == -1)
	{
		return -1;
	}
	return 1;
}

static int32_t
syntax_gt(program_t *program, syntax_t *syntax)
{
	return scanner_gt(program, syntax->scanner);
}

static int32_t
syntax_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}




static node_t *
syntax_id(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_id(node, syntax->token->value);
	if (node2 == NULL)
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
syntax_int8(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_int8(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT8) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_int16(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_int16(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT16) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_int32(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_int32(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT32) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_int64(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_int64(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT64) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_uint8(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_uint8(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT8) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_uint16(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_uint16(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT16) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_uint32(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_uint32(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT32) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_uint64(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_uint64(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT64) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_bigint(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_bigint(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_BIGINT) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_float32(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_float32(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FLOAT32) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_float64(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_float64(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FLOAT64) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_bigfloat(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_bigfloat(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_BIGFLOAT) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_char(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_char(node, syntax->token->value);
	if (node2 == NULL)
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
syntax_string(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_string(node, syntax->token->value);
	if (node2 == NULL)
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
syntax_null(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_null(node);
	if (node2 == NULL)
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
syntax_kint8(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kint8(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT8_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kint16(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kint16(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT16_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kint32(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kint32(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT32_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kint64(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kint64(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_INT64_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_kuint8(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kuint8(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT8_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kuint16(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kuint16(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT16_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kuint32(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kuint32(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT32_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kuint64(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kuint64(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_UINT64_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kbigint(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kbigint(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_BIGINT_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_kfloat32(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kfloat32(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FLOAT32_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kfloat64(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kfloat64(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FLOAT64_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kbigfloat(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kbigfloat(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_BIGFLOAT_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_kchar(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kchar(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_CHAR_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_kstring(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_kstring(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_STRING_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}


static node_t *
syntax_this(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_this(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_THIS_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_self(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *node2 = node_make_self(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_SELF_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_operator(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
	if (node2 == NULL)
	{
		return NULL;
	}

	return node2;
}

static node_t *
syntax_tuple(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACKET) == -1)
	{
		return NULL;
	}

	list_t *items = list_create();
	if (items == NULL)
	{
		return NULL;
	}

	if (syntax->token->type != TOKEN_RBRACKET)
	{
		while (true)
		{
			node_t *expr = syntax_expression(program, syntax, node);
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

	return node_make_tuple(node, items);
}

static node_t *
syntax_pair(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_expression(program, syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_pair(node, key, value);
}

static node_t *
syntax_object(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	list_t *items = list_create();
	if (items == NULL)
	{
		return NULL;
	}

	if (syntax->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *pair = syntax_pair(program, syntax, node);
			if (pair == NULL)
			{
				return NULL;
			}

			if (list_rpush(items, pair) == NULL)
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

	return node_make_object(node, items);
}

static node_t *
syntax_parenthesis(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	node_t *value = syntax_expression(program, syntax, node);
	if (value == NULL)
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
syntax_lambda(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_FUN_KEYWORD) == -1)
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

		generics = syntax_generics(program, syntax, node);
		if (generics == NULL)
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
			parameters = syntax_parameters(program, syntax, node);
			if (parameters == NULL)
			{
				return NULL;
			}
		}

		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *return_type = syntax_postfix(program, syntax, node);
		if (return_type == NULL)
		{
			return NULL;
		}

		node_t *body = NULL;
		if (syntax->token->type == TOKEN_MINUS_GT)
		{
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			body = syntax_expression(program, syntax, node);
			if (body == NULL)
			{
				return NULL;
			}
		}
		else
		{
			body = syntax_body(program, syntax, node);
			if (body == NULL)
			{
				return NULL;
			}
		}

		return node_make_lambda(node, generics, parameters, body, return_type);
	}
	
	if (syntax->token->type == TOKEN_MINUS_GT)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *body = syntax_expression(program, syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		return node_make_lambda(node, generics, parameters, body, NULL);
	}

	if (syntax->token->type == TOKEN_LBRACE)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *body = syntax_body(program, syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		return node_make_lambda(node, generics, parameters, body, NULL);
	}
	else
	{
		return node_make_lambda(node, generics, parameters, NULL, NULL);
	}
}

static node_t *
syntax_primary(program_t *program, syntax_t *syntax, node_t *parent)
{
	if (syntax->token->type == TOKEN_INT8)
	{
		return syntax_int8(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT16)
	{
		return syntax_int16(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT32)
	{
		return syntax_int32(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT64)
	{
		return syntax_int64(program, syntax, parent);
	}
	else 

	if (syntax->token->type == TOKEN_UINT8)
	{
		return syntax_uint8(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_UINT16)
	{
		return syntax_uint16(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_UINT32)
	{
		return syntax_uint32(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_UINT64)
	{
		return syntax_uint64(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_BIGINT)
	{
		return syntax_bigint(program, syntax, parent);
	}
	else 

	if (syntax->token->type == TOKEN_FLOAT32)
	{
		return syntax_float32(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_FLOAT64)
	{
		return syntax_float64(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_BIGFLOAT)
	{
		return syntax_bigfloat(program, syntax, parent);
	}
	else

	if (syntax->token->type == TOKEN_CHAR)
	{
		return syntax_char(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_STRING)
	{
		return syntax_string(program, syntax, parent);
	}
	else
	
	if (syntax->token->type == TOKEN_NULL_KEYWORD)
	{
		return syntax_null(program, syntax, parent);
	}
	else 

	if (syntax->token->type == TOKEN_INT8_KEYWORD)
	{
		return syntax_kint8(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT16_KEYWORD)
	{
		return syntax_kint16(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT32_KEYWORD)
	{
		return syntax_kint32(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT64_KEYWORD)
	{
		return syntax_kint64(program, syntax, parent);
	}
	else

	if (syntax->token->type == TOKEN_UINT8_KEYWORD)
	{
		return syntax_kuint8(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_UINT16_KEYWORD)
	{
		return syntax_kuint16(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_UINT32_KEYWORD)
	{
		return syntax_kuint32(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_UINT64_KEYWORD)
	{
		return syntax_kuint64(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_BIGINT_KEYWORD)
	{
		return syntax_kbigint(program, syntax, parent);
	}
	else

	if (syntax->token->type == TOKEN_FLOAT32_KEYWORD)
	{
		return syntax_kfloat32(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_FLOAT64_KEYWORD)
	{
		return syntax_kfloat64(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_BIGFLOAT_KEYWORD)
	{
		return syntax_kbigfloat(program, syntax, parent);
	}
	else

	if (syntax->token->type == TOKEN_CHAR_KEYWORD)
	{
		return syntax_kchar(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_STRING_KEYWORD)
	{
		return syntax_kstring(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_THIS_KEYWORD)
	{
		return syntax_this(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_SELF_KEYWORD)
	{
		return syntax_self(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LBRACKET)
	{
		return syntax_tuple(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LBRACE)
	{
		return syntax_object(program, syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LPAREN)
	{
		return syntax_parenthesis(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		return syntax_lambda(program, syntax, parent);
	}
	else 
	{
		return syntax_id(program, syntax, parent);
	}
}

static node_t *
syntax_field(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_postfix(program, syntax, node);
	if (key == NULL)
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

		value = syntax_postfix(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_field(node, key, value);
}

static node_t *
syntax_fields(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node_t *node2 = syntax_field(program, syntax, node);
		if (node2 == NULL)
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
syntax_argument(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_expression(program, syntax, node);
	if (key == NULL)
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

		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_argument(node, key, value);
}

static node_t *
syntax_arguments(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node2 = syntax_argument(program, syntax, node);
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(arguments, node2))
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

	return node_make_arguments(node, arguments);
}

static node_t *
syntax_postfix(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_primary(program, syntax, parent);
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

			node_t *node = node_create(parent, syntax->token->position);
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

			node_t *concepts = syntax_fields(program, syntax, node);
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
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *arguments = NULL;
			if (syntax->token->type != TOKEN_RBRACKET)
			{
				arguments = syntax_arguments(program, syntax, node);
				if (arguments == NULL)
				{
					return NULL;
				}
			}

			if (syntax_match(program, syntax, TOKEN_RBRACKET) == -1)
			{
				return NULL;
			}

			node2 = node_make_array(node, node2, arguments);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_DOT)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_id(program, syntax, node);
			if (right == NULL)
			{
				return NULL;
			}
      
			node2 = node_make_attribute(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_LPAREN)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}	

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *arguments = NULL;
			if (syntax->token->type != TOKEN_RPAREN)
			{
				arguments = syntax_arguments(program, syntax, node);
				if (arguments == NULL)
				{
					return NULL;
				}
			}

			if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
			{
				return NULL;
			}

			node2 = node_make_call(node, node2, arguments);
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
syntax_prefix(program_t *program, syntax_t *syntax, node_t *parent)
{
	if (syntax->token->type == TOKEN_TILDE)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_prefix(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_tilde(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_NOT)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_not(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_MINUS)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_neg(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_PLUS)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_pos(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_SIZEOF_KEYWORD)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_sizeof(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_TYPEOF_KEYWORD)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		
		node_t *right = syntax_prefix(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_typeof(node, right);
	}
	else
	{
		return syntax_postfix(program, syntax, parent);
	}
}

static node_t *
syntax_power(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_prefix(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_POWER)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_prefix(program, syntax, node);
			if (right == NULL)
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
syntax_multiplicative(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_power(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_STAR)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			node2 = node_make_mul(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_SLASH)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			node2 = node_make_div(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_PERCENT)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			node2 = node_make_mod(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_BACKSLASH)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_power(program, syntax, node);
			if (right == NULL)
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
syntax_addative(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_multiplicative(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}	
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_PLUS)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_multiplicative(program, syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			node2 = node_make_plus(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_MINUS)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_multiplicative(program, syntax, node);
			if (right == NULL)
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
syntax_shifting(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_addative(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_LT_LT)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_addative(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_shl(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_GT)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_addative(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_shr(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_relational(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_shifting(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_LT)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_lt(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_LT_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_le(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_gt(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_shifting(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_ge(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_equality(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_relational(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_EQ_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_relational(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_eq(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_NOT_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_relational(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_neq(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_bitwise_and(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_equality(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_AND)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_equality(program, syntax, node);
			if (right == NULL)
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
syntax_bitwise_xor(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_bitwise_and(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_CARET)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_bitwise_and(program, syntax, node);
			if (right == NULL)
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
syntax_bitwise_or(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_bitwise_xor(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_OR)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_bitwise_xor(program, syntax, node);
			if (right == NULL)
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
syntax_logical_and(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_bitwise_or(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_AND_AND)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}

			node_t *right = syntax_bitwise_or(program, syntax, node);
			if (right == NULL)
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
syntax_logical_or(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_logical_and(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}
	
	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_OR_OR)
		{
			node_t *node = node_create(parent, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}
			
			node_t *right = syntax_logical_and(program, syntax, node);
			if (right == NULL)
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
syntax_expression(program_t *program, syntax_t *syntax, node_t *parent)
{
	return syntax_logical_or(program, syntax, parent);
}

static node_t *
syntax_assign(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node2 = syntax_expression(program, syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (syntax->token->type == TOKEN_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_PLUS_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_add_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_MINUS_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_sub_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_STAR_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_mul_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_SLASH_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_div_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_BACKSLASH_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_epi_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_PERCENT_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_mod_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_AND_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_and_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_OR_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_or_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_LT_LT_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_shl_assign(node, node2, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_GT_EQ)
	{
		node_t *node = node_create(parent, syntax->token->position);
		
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		node_t *right = syntax_expression(program, syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return node_make_shr_assign(node, node2, right);
	}

	return node2;
}

static node_t *
syntax_if(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
	condition = syntax_expression(program, syntax, node);
	if (!condition)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	node_t *then_body;
	then_body = syntax_body(program, syntax, node);
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
			else_body = syntax_if(program, syntax, node);
		}
		else
		{
			else_body = syntax_body(program, syntax, node);
		}

		if (!else_body)
		{
			return NULL;
		}
	}

	return node_make_if(node, condition, then_body, else_body);
}

static node_t *
syntax_entity(program_t *program, syntax_t *syntax, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		type = syntax_id(program, syntax, node);
		if (type == NULL)
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
		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_entity(node, flag, key, type, value);
}

static node_t *
syntax_set(program_t *program, syntax_t *syntax, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(parent, syntax->token->position);
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
			node_t *entity = syntax_entity(program, syntax, node, flag);
			if (!entity)
			{
				return NULL;
			}

			if (!list_rpush(set, entity))
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
syntax_var(program_t *program, syntax_t *syntax, node_t *parent, uint64_t flag)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_VAR_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t set_used = 0;
	node_t *key = NULL;
	if (syntax->token->type != TOKEN_ID)
	{
		key = syntax_set(program, syntax, node, flag);
		if (key == NULL)
		{
			return NULL;
		}
		set_used = 1;
	}
	else
	{
		key = syntax_id(program, syntax, node);
		if (key == NULL)
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
		if (set_used == 1)
		{
			syntax_error(program, syntax->token->position, "Variable set with type");
			return NULL;
		}
		type = syntax_postfix(program, syntax, node);
		if (type == NULL)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if ((syntax->token->type == TOKEN_EQ) || (set_used == 1))
	{
		if (syntax_match(program, syntax, TOKEN_EQ) == -1)
		{
			return NULL;
		}

		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_var(node, flag, key, type, value);
}

static node_t *
syntax_for(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		key = syntax_id(program, syntax, node);
		if (key == NULL)
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

		node_t *body = syntax_body(program, syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		syntax->loop_depth -= 1;

		node_t *node2 = node_create(node, syntax->token->position);
		if (node2 == NULL)
		{
			return NULL;
		}

		node_t *initializer_block = node_make_block(node2, initializer);
		if (initializer_block == NULL)
		{
			return NULL;
		}

		node_t *node3 = node_create(node, syntax->token->position);
		if (node3 == NULL)
		{
			return NULL;
		}

		node_t *incrementor_block = node_make_block(node3, incrementor);
		if (incrementor_block == NULL)
		{
			return NULL;
		}

		return node_make_for(node, key, initializer_block, condition, incrementor_block, body);
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

	while (true)
	{
		int32_t use_readonly = 0;
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			if (syntax_next(program, syntax) == -1)
			{
				return NULL;
			}
			use_readonly = 1;
		}

		node_t *node2 = NULL;
		if (use_readonly || (syntax->token->type == TOKEN_VAR_KEYWORD))
		{
			node2 = syntax_var(program, syntax, node, use_readonly ? SYNTAX_MODIFIER_READONLY:SYNTAX_MODIFIER_NONE);
		}
		else
		{
			node2 = syntax_assign(program, syntax, node);
		}
		
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(initializer, node2))
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

	condition = syntax_expression(program, syntax, node);
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
		node_t *node2 = syntax_assign(program, syntax, node);
		if (node2 == NULL)
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

	node_t *body = syntax_body(program, syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	syntax->loop_depth -= 1;

	node_t *node2 = node_create(node, syntax->token->position);
	if (node2 == NULL)
	{
		return NULL;
	}

	node_t *initializer_block = node_make_initializer(node2, initializer);
	if (initializer_block == NULL)
	{
		return NULL;
	}

	node_t *node3 = node_create(node, syntax->token->position);
	if (node3 == NULL)
	{
		return NULL;
	}

	node_t *incrementor_block = node_make_incrementor(node3, incrementor);
	if (incrementor_block == NULL)
	{
		return NULL;
	}

	return node_make_for(node, key, initializer_block, condition, incrementor_block, body);
}

static node_t *
syntax_break(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		value = syntax_id(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_break(node, value);
}

static node_t *
syntax_continue(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		value = syntax_id(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_continue(node, value);
}

static node_t *
syntax_catch(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
			parameters = syntax_parameters(program, syntax, node);
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
	body = syntax_body(program, syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	return node_make_catch(node, parameters, body);
}

static node_t *
syntax_catchs(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node_t *node2 = syntax_catch(program, syntax, node);
		if (node2 == NULL)
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
syntax_try(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_TRY_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *body = syntax_body(program, syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	node_t *catchs = NULL;
	if (syntax->token->type == TOKEN_CATCH_KEYWORD)
	{
		catchs = syntax_catchs(program, syntax, node);
		if (catchs == NULL)
		{
			return NULL;
		}
	}

	return node_make_try(node, body, catchs);
}

static node_t *
syntax_throw(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_THROW_KEYWORD) == -1)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	node_t *arguments = syntax_arguments(program, syntax, node);
	if (arguments == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	return node_make_throw(node, arguments);
}

static node_t *
syntax_return(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_RETURN_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_return(node, value);
}

static node_t *
syntax_readonly(program_t *program, syntax_t *syntax, node_t *parent, uint64_t flag)
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
		node = syntax_var(program, syntax, parent, flag);
		break;

	default:
		syntax_error(program, syntax->token->position, "incorrect use of modifier 'readonly'");
		break;
	}

	flag &= ~SYNTAX_MODIFIER_READONLY;

	if (node == NULL)
	{
		return NULL;
	}

	return node;
}

static node_t *
syntax_statement(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = NULL;
	if (syntax->token->type == TOKEN_IF_KEYWORD)
	{
		node = syntax_if(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_TRY_KEYWORD)
	{
		node = syntax_try(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_FOR_KEYWORD)
	{
		node = syntax_for(program, syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_VAR_KEYWORD)
	{
		node = syntax_var(program, syntax, parent, SYNTAX_MODIFIER_NONE);
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
		node = syntax_readonly(program, syntax, parent, SYNTAX_MODIFIER_NONE);
	}
	else
	if (syntax->token->type == TOKEN_BREAK_KEYWORD)
	{
		node = syntax_break(program, syntax, parent);
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
		node = syntax_continue(program, syntax, parent);
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
		node = syntax_return(program, syntax, parent);
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
	if (syntax->token->type == TOKEN_THROW_KEYWORD)
	{
		node = syntax_throw(program, syntax, parent);
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
		node = syntax_assign(program, syntax, parent);
		if (node == NULL)
		{
			return NULL;
		}

		if (node->kind == NODE_KIND_ASSIGN)
		{
			node_binary_t *binary1 = (node_binary_t *)node->value;
			node_t *right1 = binary1->right;
			if (right1->kind != NODE_KIND_LAMBDA)
			{
				if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
				{
					return NULL;
				}
			}
		}
		else
		{
			if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
			{
				return NULL;
			}
		}
	}

	return node;
}

static node_t *
syntax_body(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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

		node_t *stmt = syntax_statement(program, syntax, node);
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
syntax_parameter(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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

	if (syntax->token->type == TOKEN_STAR)
	{
		flag |= SYNTAX_MODIFIER_KARG;
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_POWER)
	{
		flag |= SYNTAX_MODIFIER_KWARG;
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
	{
		return NULL;
	}	
	
	node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		type = syntax_postfix(program, syntax, node);
		if (type == NULL)
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

		if ((flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
		{
			syntax_error(program, syntax->token->position, "reference parameter by value");
			return NULL;
		}

		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_parameter(node, flag, key, type, value);
}

static node_t *
syntax_parameters(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node_t *node2 = syntax_parameter(program, syntax, node);
		if (node2 == NULL)
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
syntax_generic(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key;
	key = syntax_id(program, syntax, node);
	if (key == NULL)
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
		type = syntax_postfix(program, syntax, node);
		if (type == NULL)
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
		value = syntax_postfix(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_generic(node, key, type, value);
}

static node_t *
syntax_generics(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node_t *node2 = syntax_generic(program, syntax, node);
		if (node2 == NULL)
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
syntax_fun(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		key = syntax_id(program, syntax, node);
		if (key == NULL)
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
		key = syntax_operator(program, syntax, node);
		if (key == NULL)
		{
			return NULL;
		}

		used_operator = 1;
	}

	uint64_t flag2 = flag;
	if (used_operator == 1)
	{
		flag2 |= SYNTAX_MODIFIER_OPERATOR;
	}

	if (used_constructor == 1)
	{
		flag2 |= SYNTAX_MODIFIER_CONSTRUCTOR;
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

		generics = syntax_generics(program, syntax, node);
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
			parameters = syntax_parameters(program, syntax, node);
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
	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
		result = syntax_postfix(program, syntax, node);
		if (result == NULL)
		{
			return NULL;
		}
	}

	node_t *body = syntax_body(program, syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	return node_make_func(node, note, flag2, key, generics, parameters, result, body);
}

static node_t *
syntax_property(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		type = syntax_postfix(program, syntax, node);
		if (type == NULL)
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

		value = syntax_expression(program, syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return node_make_property(node, note, flag, key, type, value);
}

static node_t *
syntax_class_override(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_OVERRIDE_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_OVERRIDE;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_fun(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		syntax_error(program, syntax->token->position, "incorrect use of modifier 'override'");
		return NULL;
	}

	flag &= ~SYNTAX_MODIFIER_OVERRIDE;

	return node;
}

static node_t *
syntax_class_readonly(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		node = syntax_property(program, syntax, parent, note, flag);
		if (node == NULL)
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
syntax_class_static(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_class_readonly(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_fun(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = syntax_class(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_property(program, syntax, parent, note, flag);
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
syntax_class_export(program_t *program, syntax_t *syntax, node_t *parent, node_t *note)
{
	if (syntax_match(program, syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_OVERRIDE_KEYWORD)
	{
		node = syntax_class_override(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_class_readonly(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = syntax_class_static(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_fun(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = syntax_class(program, syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_property(program, syntax, parent, note, flag);
		if (node == NULL)
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
syntax_class_annotation(program_t *program, syntax_t *syntax, node_t *parent, node_t *note)
{
	node_t *note2 = syntax_note(program, syntax, parent, note);
	if (note2 == NULL)
	{
		return NULL;
	}

	node_t *node = NULL;
	if (syntax->token->type == TOKEN_AT)
	{
		node = syntax_class_annotation(program, syntax, parent, note2);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
	{
		node = syntax_class_export(program, syntax, parent, note2);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = syntax_class_readonly(program, syntax, parent, note2, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = syntax_class_static(program, syntax, parent, note2, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = syntax_fun(program, syntax, parent, note2, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = syntax_class(program, syntax, parent, note2, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = syntax_property(program, syntax, parent, note2, SYNTAX_MODIFIER_NONE);
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
syntax_class_block(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		if (syntax->token->type == TOKEN_AT)
		{
			decl = syntax_class_annotation(program, syntax, node, NULL);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			decl = syntax_class_export(program, syntax, node, NULL);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_OVERRIDE_KEYWORD)
		{
			decl = syntax_class_override(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			decl = syntax_class_readonly(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_STATIC_KEYWORD)
		{
			decl = syntax_class_static(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_FUN_KEYWORD)
		{
			decl = syntax_fun(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_CLASS_KEYWORD)
		{
			decl = syntax_class(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (decl == NULL)
			{
				return NULL;
			}
		}
		else
		{
			decl = syntax_property(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
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
syntax_heritage(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
	{
		return NULL;
	}
	
	if (syntax_match(program, syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}
	
	node_t *type = syntax_expression(program, syntax, node);
	if (type == NULL)
	{
		return NULL;
	}

	return node_make_heritage(node, key, type);
}

static node_t *
syntax_heritages(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node_t *node2 = syntax_heritage(program, syntax, node);
		if (node2 == NULL)
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
syntax_class(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_CLASS_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
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

		generics = syntax_generics(program, syntax, node);
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

		heritages = syntax_heritages(program, syntax, node);
		if (!heritages)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}

	}

	node_t *block = syntax_class_block(program, syntax, node);
	if (!block)
	{
		return NULL;
	}

	return node_make_class(node, note, flag, key, generics, heritages, block);
}

static node_t *
syntax_package(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
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

		generics = syntax_generics(program, syntax, node);
		if (generics == NULL)
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

	node_t *address = syntax_postfix(program, syntax, node);
	if (address == NULL)
	{
		return NULL;
	}

	return node_make_package(node, key, generics, address);
}

static node_t *
syntax_packages(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
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
		node_t *node2 = syntax_package(program, syntax, node);
		if (node2 == NULL)
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
syntax_using(program_t *program, syntax_t *syntax, node_t *parent)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_USING_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *packages = NULL;
	if (syntax->token->type == TOKEN_STAR)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}
	}
	else
	{
		packages = syntax_packages(program, syntax, node);
		if (packages == NULL)
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_FROM_KEYWORD) == -1)
	{
		return NULL;
	}

	node_t *path = syntax_string(program, syntax, node);
	if (path == NULL)
	{
		return NULL;
	}

	return node_make_using(node, path, packages);
}

static node_t *
syntax_static(program_t *program, syntax_t *syntax, node_t *parent, node_t *note, uint64_t flag)
{
	if (syntax_match(program, syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	node_t *node = syntax_class(program, syntax, parent, note, flag);

	flag &= ~SYNTAX_MODIFIER_STATIC;

	if (node == NULL)
	{
		return NULL;
	}

	return node;
}

static node_t *
syntax_export(program_t *program, syntax_t *syntax, node_t *parent, node_t *note)
{
	if (syntax_match(program, syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	node_t *object = NULL;

	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		object = syntax_static(program, syntax, parent, note, flag);
	}
	else
	{
		object = syntax_class(program, syntax, parent, note, flag);
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	return object;
}

static node_t *
syntax_note(program_t *program, syntax_t *syntax, node_t *parent, node_t *note)
{
	node_t *node = node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (syntax_match(program, syntax, TOKEN_AT) == -1)
	{
		return NULL;
	}

	node_t *key = syntax_id(program, syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	node_t *arguments = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (syntax_next(program, syntax) == -1)
		{
			return NULL;
		}

		arguments = syntax_arguments(program, syntax, node);
		if (arguments == NULL)
		{
			return NULL;
		}

		if (syntax_match(program, syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	return node_make_annotation(node, key, arguments, note);
}

static node_t *
syntax_annotation(program_t *program, syntax_t *syntax, node_t *parent, node_t *note)
{
	node_t *note2 = syntax_note(program, syntax, parent, note);
	if (note2 == NULL)
	{
		return NULL;
	}

	node_t *result = NULL;
	if (syntax->token->type == TOKEN_AT)
	{
		result = syntax_annotation(program, syntax, parent, note2);
		if (result == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
	{
		result = syntax_export(program, syntax, parent, note2);
		if (result == NULL)
		{
			return NULL;
		}
	}
	else
	{
		result = syntax_class(program, syntax, parent, note2, SYNTAX_MODIFIER_NONE);
		if (result == NULL)
		{
			return NULL;
		}
	}

	return result;
}

node_t *
syntax_module(program_t *program, syntax_t *syntax)
{
	node_t *node = node_create(NULL, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	list_t *items = list_create();
	if (items == NULL)
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

		if (syntax->token->type == TOKEN_USING_KEYWORD)
		{
			item = syntax_using(program, syntax, node);
			if (item == NULL)
			{
				return NULL;
			}
			if (syntax_match(program, syntax, TOKEN_SEMICOLON) == -1)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_AT)
		{
			item = syntax_annotation(program, syntax, node, NULL);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = syntax_export(program, syntax, node, NULL);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		{
			item = syntax_class(program, syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}

		if (list_rpush(items, item) == NULL)
		{
			return NULL;
		}
	}

	if (syntax_match(program, syntax, TOKEN_EOF) == -1)
	{
		return NULL;
	}

	return node_make_module(node, syntax->scanner->path, items);
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

	scanner_t *scanner = scanner_create(program, path);
	if (scanner == NULL)
	{
		return NULL;
	}

	syntax->scanner = scanner;
	syntax->loop_depth = 0;
	syntax->token = &scanner->token;

	syntax->states = list_create();
	if (syntax->states == NULL)
	{
		return NULL;
	}

	return syntax;
}
