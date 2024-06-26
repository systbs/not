#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../error.h"
#include "../../memory.h"
#include "../../mutex.h"
#include "../../config.h"
#include "../../scanner/scanner.h"
#include "syntax.h"

static not_node_t *
not_syntax_expression(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_statement(not_syntax_t *syntax, not_node_t *parent, uint64_t flag);

static not_node_t *
not_syntax_body(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_class(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag);

static not_node_t *
not_syntax_class_fun(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag);

static not_node_t *
not_syntax_parameters(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_generics(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_parameter(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_class_property(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag);

static not_node_t *
not_syntax_parenthesis(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_heritages(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_prefix(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_export(not_syntax_t *syntax, not_node_t *parent, not_node_t *note);

static not_node_t *
not_syntax_postfix(not_syntax_t *syntax, not_node_t *parent);

static not_node_t *
not_syntax_notes(not_syntax_t *syntax, not_node_t *parent);

int32_t
not_syntax_save(not_syntax_t *syntax)
{
	not_scanner_t *scanner = syntax->scanner;

	not_syntax_state_t *state = (not_syntax_state_t *)not_memory_calloc(1, sizeof(not_syntax_state_t));
	if (state == NULL)
	{
		not_error_no_memory();
		return -1;
	}

	state->fun_depth = syntax->fun_depth;
	state->loop_depth = syntax->loop_depth;
	state->offset = scanner->offset;
	state->reading_offset = scanner->reading_offset;
	state->line = scanner->line;
	state->column = scanner->column;
	state->ch = scanner->ch;
	state->token = scanner->token;

	if (not_queue_right_push(syntax->states, state) == NULL)
	{
		return -1;
	}

	return 1;
}

int32_t
not_syntax_restore(not_syntax_t *syntax)
{
	not_scanner_t *scanner = syntax->scanner;

	not_queue_entry_t *queue_entry = not_queue_right_pop(syntax->states);
	if (queue_entry == NULL)
	{
		return -1;
	}

	not_syntax_state_t *state = (not_syntax_state_t *)queue_entry->value;
	if (state == NULL)
	{
		return -1;
	}

	syntax->fun_depth = state->fun_depth;
	syntax->loop_depth = state->loop_depth;
	scanner->offset = state->offset;
	scanner->reading_offset = state->reading_offset;
	scanner->line = state->line;
	scanner->column = state->column;
	scanner->ch = state->ch;

	scanner->token = state->token;

	not_memory_free(state);
	not_memory_free(queue_entry);

	return 1;
}

int32_t
not_syntax_release(not_syntax_t *syntax)
{
	not_queue_entry_t *queue_entry = not_queue_right_pop(syntax->states);
	if (queue_entry == NULL)
	{
		return -1;
	}

	not_syntax_state_t *state = (not_syntax_state_t *)queue_entry->value;
	if (state == NULL)
	{
		return -1;
	}

	not_memory_free(state);
	return 1;
}

static void
not_syntax_expected(not_syntax_t *syntax, int32_t type)
{
	char message[1024];
	memset(message, 0, 1024);

	switch (syntax->token->type)
	{
	case TOKEN_ID:
	case TOKEN_NUMBER:
	case TOKEN_CHAR:
	case TOKEN_STRING:
		sprintf(
			message,
			"expected '%s', got '%s'",
			not_token_get_name(type),
			syntax->token->value);
		break;

	default:
		sprintf(
			message,
			"expected '%s', got '%s'",
			not_token_get_name(type),
			not_token_get_name(syntax->token->type));
		break;
	}

	not_error_syntax_by_position(syntax->token->position, message);
}

static int32_t
not_syntax_match(not_syntax_t *syntax, int32_t type)
{
	if (syntax->token->type == type)
	{
		if (not_scanner_advance(syntax->scanner) == -1)
		{
			return -1;
		}
	}
	else
	{
		not_syntax_expected(syntax, type);
		return -1;
	}
	return 1;
}

static int32_t
not_syntax_next(not_syntax_t *syntax)
{
	if (not_scanner_advance(syntax->scanner) == -1)
	{
		return -1;
	}
	return 1;
}

static int32_t
not_syntax_gt(not_syntax_t *syntax)
{
	return not_scanner_gt(syntax->scanner);
}

static int32_t
not_syntax_id_strcmp(not_node_t *n1, char *name)
{
	not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}

static not_node_t *
not_syntax_id(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_id(node, syntax->token->value);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_ID) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_number(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_number(node, syntax->token->value);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_NUMBER) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_char(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_char(node, syntax->token->value);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_CHAR) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_string(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_string(node, syntax->token->value);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_STRING) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_null(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_null(node);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_NULL_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_kint(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_kint(node);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_INT_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_kfloat(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_kfloat(node);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_FLOAT_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_kchar(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_kchar(node);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_CHAR_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_kstring(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_kstring(node);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_STRING_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_operator(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	char *operator= NULL;
	if (syntax->token->type == TOKEN_PLUS)
	{
		if (not_syntax_match(syntax, TOKEN_PLUS) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "+";
	}
	else if (syntax->token->type == TOKEN_MINUS)
	{
		if (not_syntax_match(syntax, TOKEN_MINUS) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "-";
	}
	else if (syntax->token->type == TOKEN_STAR)
	{
		if (not_syntax_match(syntax, TOKEN_STAR) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "*";
	}
	else if (syntax->token->type == TOKEN_SLASH)
	{
		if (not_syntax_match(syntax, TOKEN_SLASH) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "/";
	}
	else if (syntax->token->type == TOKEN_BACKSLASH)
	{
		if (not_syntax_match(syntax, TOKEN_BACKSLASH) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "\\";
	}
	else if (syntax->token->type == TOKEN_POWER)
	{
		if (not_syntax_match(syntax, TOKEN_POWER) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "**";
	}
	else if (syntax->token->type == TOKEN_PERCENT)
	{
		if (not_syntax_match(syntax, TOKEN_PERCENT) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "%";
	}
	else if (syntax->token->type == TOKEN_AND)
	{
		if (not_syntax_match(syntax, TOKEN_AND) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "&";
	}
	else if (syntax->token->type == TOKEN_OR)
	{
		if (not_syntax_match(syntax, TOKEN_OR) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "|";
	}
	else if (syntax->token->type == TOKEN_CARET)
	{
		if (not_syntax_match(syntax, TOKEN_CARET) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "^";
	}
	else if (syntax->token->type == TOKEN_LT_LT)
	{
		if (not_syntax_match(syntax, TOKEN_LT_LT) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "<<";
	}
	else if (syntax->token->type == TOKEN_GT_GT)
	{
		if (not_syntax_match(syntax, TOKEN_GT_GT) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= ">>";
	}
	else if (syntax->token->type == TOKEN_LT)
	{
		if (not_syntax_match(syntax, TOKEN_LT) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "<";
	}
	else if (syntax->token->type == TOKEN_GT)
	{
		if (not_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= ">";
	}
	else if (syntax->token->type == TOKEN_LBRACKET)
	{
		if (not_syntax_match(syntax, TOKEN_LBRACKET) == -1)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_RBRACKET) == -1)
		{
			return NOT_PTR_ERROR;
		}
		operator= "[]";
	}
	else
	{
		not_error_syntax_by_position(syntax->token->position, "operator expected");
		return NOT_PTR_ERROR;
	}

	not_node_t *node2 = not_node_make_id(node, operator);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return node2;
}

static not_node_t *
not_syntax_tuple(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_LBRACKET) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	if (syntax->token->type != TOKEN_RBRACKET)
	{
		while (true)
		{
			not_node_t *item = not_syntax_expression(syntax, node);
			if (!item)
			{
				return NOT_PTR_ERROR;
			}

			if (declaration == NULL)
			{
				declaration = item;
				top = item;
			}
			else
			{
				top->next = item;
				item->previous = top;
				top = item;
			}

			if (syntax->token->type != TOKEN_COMMA)
			{
				break;
			}

			if (not_syntax_match(syntax, TOKEN_COMMA) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}
	}

	if (not_syntax_match(syntax, TOKEN_RBRACKET) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_tuple(node, declaration);
}

static not_node_t *
not_syntax_pair(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_expression(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_pair(node, key, value);
}

static not_node_t *
not_syntax_object(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;
	if (syntax->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			not_node_t *item = not_syntax_pair(syntax, node);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (declaration == NULL)
			{
				declaration = item;
				top = item;
			}
			else
			{
				top->next = item;
				item->previous = top;
				top = item;
			}

			if (syntax->token->type != TOKEN_COMMA)
			{
				break;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}
	}

	if (not_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_object(node, declaration);
}

static not_node_t *
not_syntax_parenthesis(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_LPAREN) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = not_syntax_expression(syntax, node);
	if (value == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_parenthesis(node, value);
}

static not_node_t *
not_syntax_lambda(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_FUN_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = not_syntax_id(syntax, node);
		if (key == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_gt(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			not_error_syntax_by_position(syntax->token->position, "empty generic types");
			return NOT_PTR_ERROR;
		}

		generics = not_syntax_generics(syntax, node);
		if (generics == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_gt(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = not_syntax_parameters(syntax, node);
			if (parameters == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}

		if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *return_type = not_syntax_postfix(syntax, node);
		if (return_type == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *body = NULL;
		if (syntax->token->type == TOKEN_MINUS_GT)
		{
			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			body = not_syntax_expression(syntax, node);
			if (body == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else
		{
			body = not_syntax_body(syntax, node);
			if (body == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}

		return not_node_make_lambda(node, key, generics, parameters, body, return_type);
	}

	if (syntax->token->type == TOKEN_MINUS_GT)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *body = not_syntax_expression(syntax, node);
		if (body == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_lambda(node, key, generics, parameters, body, NULL);
	}

	if ((syntax->token->type == TOKEN_LBRACE) || (key != NULL))
	{
		syntax->fun_depth += 1;

		not_node_t *body = not_syntax_body(syntax, node);
		if (body == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		syntax->fun_depth -= 1;

		return not_node_make_lambda(node, key, generics, parameters, body, NULL);
	}
	else
	{
		return not_node_make_lambda(node, key, generics, parameters, NULL, NULL);
	}
}

static not_node_t *
not_syntax_primary(not_syntax_t *syntax, not_node_t *parent)
{
	if (syntax->token->type == TOKEN_NUMBER)
	{
		return not_syntax_number(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_CHAR)
	{
		return not_syntax_char(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_STRING)
	{
		return not_syntax_string(syntax, parent);
	}
	else

		if (syntax->token->type == TOKEN_NULL_KEYWORD)
	{
		return not_syntax_null(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_INT_KEYWORD)
	{
		return not_syntax_kint(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_FLOAT_KEYWORD)
	{
		return not_syntax_kfloat(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_CHAR_KEYWORD)
	{
		return not_syntax_kchar(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_STRING_KEYWORD)
	{
		return not_syntax_kstring(syntax, parent);
	}
	else

		if (syntax->token->type == TOKEN_LBRACKET)
	{
		return not_syntax_tuple(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_LBRACE)
	{
		return not_syntax_object(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_LPAREN)
	{
		return not_syntax_parenthesis(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		return not_syntax_lambda(syntax, parent);
	}
	else
	{
		return not_syntax_id(syntax, parent);
	}
}

static not_node_t *
not_syntax_field(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_postfix(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		value = not_syntax_postfix(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_field(node, key, value);
}

static not_node_t *
not_syntax_fields(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_field(syntax, node);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_fields(node, declaration);
}

static not_node_t *
not_syntax_argument(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_expression(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_argument(node, key, value);
}

static not_node_t *
not_syntax_arguments(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_argument(syntax, node);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_arguments(node, declaration);
}

static not_node_t *
not_syntax_postfix(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node2 = not_syntax_primary(syntax, parent);
	if (node2 == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_LT)
		{
			if (not_syntax_save(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *node = not_node_create(node2, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_gt(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			if (syntax->token->type == TOKEN_GT)
			{
				not_error_syntax_by_position(syntax->token->position, "empty generic types");
				return NOT_PTR_ERROR;
			}

			not_node_t *concepts = not_syntax_fields(syntax, node);
			if (concepts == NOT_PTR_ERROR)
			{
				if (not_syntax_restore(syntax) == -1)
				{
					return NOT_PTR_ERROR;
				}
				not_node_destroy(node);
				return node2;
			}
			else
			{
				if (not_syntax_gt(syntax) == -1)
				{
					return NOT_PTR_ERROR;
				}

				if (syntax->token->type != TOKEN_GT)
				{
					if (not_syntax_restore(syntax) == -1)
					{
						return NOT_PTR_ERROR;
					}
					not_node_destroy(concepts);
					not_node_destroy(node);
					return node2;
				}
				else
				{
					if (not_syntax_match(syntax, TOKEN_GT) == -1)
					{
						return NOT_PTR_ERROR;
					}

					if (not_syntax_release(syntax) == -1)
					{
						return NOT_PTR_ERROR;
					}

					node2 = not_node_make_pseudonym(node, node2, concepts);
					continue;
				}
			}
			continue;
		}
		else if (syntax->token->type == TOKEN_LBRACKET)
		{
			not_node_t *node = not_node_create(node2, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *arguments = NULL;
			if (syntax->token->type != TOKEN_RBRACKET)
			{
				arguments = not_syntax_arguments(syntax, node);
				if (arguments == NOT_PTR_ERROR)
				{
					return NOT_PTR_ERROR;
				}
			}

			if (not_syntax_match(syntax, TOKEN_RBRACKET) == -1)
			{
				return NOT_PTR_ERROR;
			}

			node2 = not_node_make_array(node, node2, arguments);
			continue;
		}
		else if (syntax->token->type == TOKEN_DOT)
		{
			not_node_t *node = not_node_create(node2, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_id(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			node2 = not_node_make_attribute(node, node2, right);
			continue;
		}
		else if (syntax->token->type == TOKEN_LPAREN)
		{
			not_node_t *node = not_node_create(node2, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *arguments = NULL;
			if (syntax->token->type != TOKEN_RPAREN)
			{
				arguments = not_syntax_arguments(syntax, node);
				if (arguments == NOT_PTR_ERROR)
				{
					return NOT_PTR_ERROR;
				}
			}

			if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
			{
				return NOT_PTR_ERROR;
			}

			node2 = not_node_make_call(node, node2, arguments);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static not_node_t *
not_syntax_prefix(not_syntax_t *syntax, not_node_t *parent)
{
	if (syntax->token->type == TOKEN_TILDE)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_prefix(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_tilde(node, right);
	}
	else if (syntax->token->type == TOKEN_NOT)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_prefix(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_not(node, right);
	}
	else if (syntax->token->type == TOKEN_MINUS)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_prefix(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_neg(node, right);
	}
	else if (syntax->token->type == TOKEN_PLUS)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_prefix(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_pos(node, right);
	}
	else if (syntax->token->type == TOKEN_SIZEOF_KEYWORD)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_prefix(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_sizeof(node, right);
	}
	else if (syntax->token->type == TOKEN_TYPEOF_KEYWORD)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_prefix(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_typeof(node, right);
	}
	else
	{
		return not_syntax_postfix(syntax, parent);
	}
}

static not_node_t *
not_syntax_pow(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_prefix(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_POWER)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_prefix(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_pow(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_multiplicative(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_pow(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_STAR)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_pow(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_mul(node, left, right);
			continue;
		}
		else if (syntax->token->type == TOKEN_SLASH)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_pow(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_div(node, left, right);
			continue;
		}
		else if (syntax->token->type == TOKEN_PERCENT)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_pow(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_mod(node, left, right);
			continue;
		}
		else if (syntax->token->type == TOKEN_BACKSLASH)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_pow(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_epi(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_addative(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_multiplicative(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_PLUS)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_multiplicative(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_plus(node, left, right);
			continue;
		}
		else if (syntax->token->type == TOKEN_MINUS)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_multiplicative(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_minus(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_shifting(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_addative(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_LT_LT)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_addative(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_shl(node, left, right);
	}
	else if (syntax->token->type == TOKEN_GT_GT)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_addative(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_shr(node, left, right);
	}

	return left;
}

static not_node_t *
not_syntax_relational(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_shifting(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_LT)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_shifting(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_lt(node, left, right);
	}
	else if (syntax->token->type == TOKEN_LT_EQ)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_shifting(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_le(node, left, right);
	}
	else if (syntax->token->type == TOKEN_GT)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_shifting(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_gt(node, left, right);
	}
	else if (syntax->token->type == TOKEN_GT_EQ)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_shifting(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_ge(node, left, right);
	}

	return left;
}

static not_node_t *
not_syntax_equality(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_relational(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_EQ_EQ)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_relational(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_eq(node, left, right);
	}
	else if (syntax->token->type == TOKEN_NOT_EQ)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_relational(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_neq(node, left, right);
	}

	return left;
}

static not_node_t *
not_syntax_bitwise_and(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_equality(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_AND)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_equality(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_and(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_bitwise_xor(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_bitwise_and(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_CARET)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_bitwise_and(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_xor(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_bitwise_or(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_bitwise_xor(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_OR)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_bitwise_xor(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_or(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_logical_and(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_bitwise_or(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_AND_AND)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_bitwise_or(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_land(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_logical_or(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_logical_and(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_OR_OR)
		{
			not_node_t *node = not_node_create(left, syntax->token->position);
			if (node == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			not_node_t *right = not_syntax_logical_and(syntax, node);
			if (right == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}

			left = not_node_make_lor(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static not_node_t *
not_syntax_instanceof(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_logical_or(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_INSTANCEOF_KEYWORD)
	{
		not_node_t *node = not_node_create(left, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_logical_or(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_instanceof(node, left, right);
	}

	return left;
}

static not_node_t *
not_syntax_conditional(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *condition = not_syntax_instanceof(syntax, parent);
	if (condition == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_QUESTION)
	{
		not_node_t *node = not_node_create(condition, syntax->token->position);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *left = not_syntax_conditional(syntax, node);
		if (left == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_COLON) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_conditional(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_conditional(node, condition, left, right);
	}

	return condition;
}

static not_node_t *
not_syntax_expression(not_syntax_t *syntax, not_node_t *parent)
{
	return not_syntax_conditional(syntax, parent);
}

static not_node_t *
not_syntax_assign(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *left = not_syntax_expression(syntax, parent);
	if (left == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_PLUS_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_add_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_MINUS_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_sub_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_STAR_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_mul_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_SLASH_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_div_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_BACKSLASH_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_epi_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_PERCENT_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_mod_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_POWER_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_pow_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_AND_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_and_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_OR_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_or_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_LT_LT_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_shl_assign(node, left, right);
	}
	else if (syntax->token->type == TOKEN_GT_GT_EQ)
	{
		not_node_t *node = not_node_create(parent, syntax->token->position);

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *right = not_syntax_expression(syntax, node);
		if (right == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		return not_node_make_shr_assign(node, left, right);
	}

	return left;
}

static not_node_t *
not_syntax_if_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_IF_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_LPAREN) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *condition;
	condition = not_syntax_expression(syntax, node);
	if (!condition)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *then_body;
	then_body = not_syntax_body(syntax, node);
	if (!then_body)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *else_body;
	else_body = NULL;
	if (syntax->token->type == TOKEN_ELSE_KEYWORD)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type == TOKEN_IF_KEYWORD)
		{
			else_body = not_syntax_if_stmt(syntax, node);
		}
		else
		{
			else_body = not_syntax_body(syntax, node);
		}

		if (!else_body)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_if(node, condition, then_body, else_body);
}

static not_node_t *
not_syntax_entity(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		type = not_syntax_id(syntax, node);
		if (type == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_entity(node, flag, key, type, value);
}

static not_node_t *
not_syntax_set(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_entity(syntax, parent, flag);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	if (not_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_set(node, declaration);
}

static not_node_t *
not_syntax_var_stmt(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_VAR_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	int32_t set_used = 0;
	not_node_t *key = NULL;
	if (syntax->token->type != TOKEN_ID)
	{
		key = not_syntax_set(syntax, node, flag);
		if (key == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		set_used = 1;
	}
	else
	{
		key = not_syntax_id(syntax, node);
		if (key == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *type = NULL;
	if ((syntax->token->type == TOKEN_COLON) && (set_used == 0))
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		type = not_syntax_postfix(syntax, node);
		if (type == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *value = NULL;
	if ((syntax->token->type == TOKEN_EQ) || (set_used == 1))
	{
		if (not_syntax_match(syntax, TOKEN_EQ) == -1)
		{
			return NOT_PTR_ERROR;
		}

		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_var(node, flag, key, type, value);
}

static not_node_t *
not_syntax_for_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_FOR_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = not_syntax_id(syntax, node);
		if (key == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *initializer = NULL;
	not_node_t *incrementor = NULL;

	not_node_t *condition = NULL;
	if (syntax->token->type == TOKEN_LBRACE)
	{
		syntax->loop_depth += 1;

		not_node_t *body = not_syntax_body(syntax, node);
		if (body == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		syntax->loop_depth -= 1;

		return not_node_make_for(node, key, initializer, condition, incrementor, body);
	}

	if (not_syntax_match(syntax, TOKEN_LPAREN) == -1)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_SEMICOLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		goto region_condition;
	}

	if (not_syntax_save(syntax) < 0)
	{
		return NOT_PTR_ERROR;
	}

	if (syntax->token->type == TOKEN_VAR_KEYWORD)
	{
		if (not_syntax_match(syntax, TOKEN_VAR_KEYWORD) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *field = not_syntax_id(syntax, node);
		if (field == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *value = NULL;
		if (syntax->token->type == TOKEN_COMMA)
		{
			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			value = not_syntax_id(syntax, node);
			if (value == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}

		if (syntax->token->type != TOKEN_IN_KEYWORD)
		{
			if (not_syntax_restore(syntax) < 0)
			{
				return NOT_PTR_ERROR;
			}
			goto region_for_loop;
		}

		if (not_syntax_release(syntax) < 0)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_IN_KEYWORD) == -1)
		{
			return NOT_PTR_ERROR;
		}

		not_node_t *iterator = not_syntax_expression(syntax, node);
		if (iterator == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}

		syntax->loop_depth += 1;

		not_node_t *body = not_syntax_body(syntax, node);
		if (body == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		syntax->loop_depth -= 1;

		return not_node_make_forin(node, key, field, value, iterator, body);
	}

	if (not_syntax_release(syntax) < 0)
	{
		return NOT_PTR_ERROR;
	}

region_for_loop:
	not_node_t *top = NULL;

	while (true)
	{
		int32_t use_readonly = 0;
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}
			use_readonly = 1;
		}

		not_node_t *item = NULL;
		if (use_readonly || (syntax->token->type == TOKEN_VAR_KEYWORD))
		{
			item = not_syntax_var_stmt(syntax, node, use_readonly ? SYNTAX_MODIFIER_READONLY : SYNTAX_MODIFIER_NONE);
		}
		else
		{
			item = not_syntax_assign(syntax, node);
		}

		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (initializer == NULL)
		{
			initializer = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
	{
		return NOT_PTR_ERROR;
	}

region_condition:
	if (syntax->token->type == TOKEN_SEMICOLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		goto region_step;
	}

	condition = not_syntax_expression(syntax, node);
	if (condition == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
	{
		return NOT_PTR_ERROR;
	}

region_step:
	if (syntax->token->type == TOKEN_RPAREN)
	{
		goto region_finish;
	}

	while (true)
	{
		not_node_t *item = not_syntax_assign(syntax, node);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (incrementor == NULL)
		{
			incrementor = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

region_finish:
	if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
	{
		return NOT_PTR_ERROR;
	}

	syntax->loop_depth += 1;

	not_node_t *body = not_syntax_body(syntax, node);
	if (body == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	syntax->loop_depth -= 1;

	return not_node_make_for(node, key, initializer, condition, incrementor, body);
}

static not_node_t *
not_syntax_break_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_BREAK_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = not_syntax_id(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_break(node, value);
}

static not_node_t *
not_syntax_continue_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_CONTINUE_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = not_syntax_id(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_continue(node, value);
}

static not_node_t *
not_syntax_catch_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_CATCH_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = not_syntax_parameters(syntax, node);
			if (!parameters)
			{
				return NOT_PTR_ERROR;
			}
		}
		if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *body = not_syntax_body(syntax, node);
	if (body == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_catch(node, parameters, body);
}

static not_node_t *
not_syntax_try_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_TRY_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *body = not_syntax_body(syntax, node);
	if (body == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *catchs = NULL;
	if (syntax->token->type == TOKEN_CATCH_KEYWORD)
	{
		catchs = not_syntax_catch_stmt(syntax, node);
		if (catchs == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_try(node, body, catchs);
}

static not_node_t *
not_syntax_throw_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_THROW_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = not_syntax_expression(syntax, node);
	if (value == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_throw(node, value);
}

static not_node_t *
not_syntax_return_stmt(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_RETURN_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_return(node, value);
}

static not_node_t *
not_syntax_reference_stmt(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_REFERENCE_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_REFERENCE;

	not_node_t *node = not_syntax_var_stmt(syntax, parent, flag);

	flag &= ~SYNTAX_MODIFIER_REFERENCE;

	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return node;
}

static not_node_t *
not_syntax_readonly_stmt(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	not_node_t *node = not_syntax_var_stmt(syntax, parent, flag);

	flag &= ~SYNTAX_MODIFIER_READONLY;

	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return node;
}

static not_node_t *
not_syntax_statement(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	not_node_t *node = NULL;

	if (syntax->token->type == TOKEN_IF_KEYWORD)
	{
		node = not_syntax_if_stmt(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_TRY_KEYWORD)
	{
		node = not_syntax_try_stmt(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_FOR_KEYWORD)
	{
		node = not_syntax_for_stmt(syntax, parent);
	}
	else if (syntax->token->type == TOKEN_VAR_KEYWORD)
	{
		node = not_syntax_var_stmt(syntax, parent, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = not_syntax_readonly_stmt(syntax, parent, flag);
	}
	else if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = not_syntax_reference_stmt(syntax, parent, flag);
	}
	else if ((syntax->token->type == TOKEN_BREAK_KEYWORD) && (syntax->loop_depth > 0))
	{
		node = not_syntax_break_stmt(syntax, parent);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if ((syntax->token->type == TOKEN_CONTINUE_KEYWORD) && (syntax->loop_depth > 0))
	{
		node = not_syntax_continue_stmt(syntax, parent);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if ((syntax->token->type == TOKEN_RETURN_KEYWORD) && (syntax->fun_depth > 0))
	{
		node = not_syntax_return_stmt(syntax, parent);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_THROW_KEYWORD)
	{
		node = not_syntax_throw_stmt(syntax, parent);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else
	{
		node = not_syntax_assign(syntax, parent);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return node;
}

static not_node_t *
not_syntax_body(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (syntax->token->type != TOKEN_RBRACE)
	{
		if (syntax->token->type == TOKEN_SEMICOLON)
		{
			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}

			continue;
		}

		not_node_t *item = not_syntax_statement(syntax, node, SYNTAX_MODIFIER_NONE);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}
	}

	if (not_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_body(node, declaration);
}

static not_node_t *
not_syntax_parameter(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	uint64_t flag = SYNTAX_MODIFIER_NONE;
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		flag |= SYNTAX_MODIFIER_REFERENCE;
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		flag |= SYNTAX_MODIFIER_READONLY;
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	if (syntax->token->type == TOKEN_STAR)
	{
		flag |= SYNTAX_MODIFIER_KARG;
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_POWER)
	{
		flag |= SYNTAX_MODIFIER_KWARG;
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		type = not_syntax_postfix(syntax, node);
		if (type == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if ((flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
		{
			not_error_syntax_by_position(syntax->token->position, "reference parameter by value");
			return NOT_PTR_ERROR;
		}

		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_parameter(node, flag, key, type, value);
}

static not_node_t *
not_syntax_parameters(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_parameter(syntax, parent);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_parameters(node, declaration);
}

static not_node_t *
not_syntax_generic(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key;
	key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *type = NULL;
	if (syntax->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		type = not_syntax_postfix(syntax, node);
		if (type == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		value = not_syntax_postfix(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_generic(node, key, type, value);
}

static not_node_t *
not_syntax_generics(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_generic(syntax, parent);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_generics(node, declaration);
}

static not_node_t *
not_syntax_class_fun(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_FUN_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	int32_t used_constructor = 0, used_operator = 0;
	not_node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = not_syntax_id(syntax, node);
		if (key == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_id_strcmp(key, CONSTRUCTOR_STR) == 1)
		{
			used_constructor = 1;
		}
	}
	else
	{
		key = not_syntax_operator(syntax, node);
		if (key == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		used_operator = 1;
	}

	not_node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (used_constructor == 1)
		{
			not_error_syntax_by_position(syntax->token->position, "constructor with generic types");
			return NOT_PTR_ERROR;
		}

		if (used_operator == 1)
		{
			not_error_syntax_by_position(syntax->token->position, "operator with generic types");
			return NOT_PTR_ERROR;
		}

		if (not_syntax_gt(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			not_error_syntax_by_position(syntax->token->position, "empty generic types");
			return NOT_PTR_ERROR;
		}

		generics = not_syntax_generics(syntax, node);
		if (!generics)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_gt(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = not_syntax_parameters(syntax, node);
			if (!parameters)
			{
				return NOT_PTR_ERROR;
			}
		}

		if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *result = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		result = not_syntax_postfix(syntax, node);
		if (result == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	syntax->fun_depth += 1;

	not_node_t *body = not_syntax_body(syntax, node);
	if (body == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	syntax->fun_depth -= 1;

	return not_node_make_func(node, note, flag, key, generics, parameters, result, body);
}

static not_node_t *
not_syntax_class_property(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		type = not_syntax_postfix(syntax, node);
		if (type == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *value = NULL;
	if ((syntax->token->type == TOKEN_EQ) || ((flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC))
	{
		if (not_syntax_match(syntax, TOKEN_EQ) == -1)
		{
			return NOT_PTR_ERROR;
		}

		value = not_syntax_expression(syntax, node);
		if (value == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_property(node, note, flag, key, type, value);
}

static not_node_t *
not_syntax_class_reference(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_REFERENCE_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_REFERENCE;

	not_node_t *node = not_syntax_class_property(syntax, parent, note, flag);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	flag &= ~SYNTAX_MODIFIER_REFERENCE;

	return node;
}

static not_node_t *
not_syntax_class_readonly(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	not_node_t *node = not_syntax_class_property(syntax, parent, note, flag);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	flag &= ~SYNTAX_MODIFIER_READONLY;

	return node;
}

static not_node_t *
not_syntax_class_static(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	not_node_t *node = NULL;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = not_syntax_class_readonly(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = not_syntax_class_fun(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = not_syntax_class(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else
	{
		node = not_syntax_class_property(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	flag &= ~SYNTAX_MODIFIER_STATIC;

	return node;
}

static not_node_t *
not_syntax_class_export(not_syntax_t *syntax, not_node_t *parent, not_node_t *note)
{
	if (not_syntax_match(syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	not_node_t *node = NULL;
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = not_syntax_class_reference(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = not_syntax_class_readonly(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = not_syntax_class_static(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = not_syntax_class_fun(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = not_syntax_class(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else
	{
		node = not_syntax_class_property(syntax, parent, note, flag);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	return node;
}

static not_node_t *
not_syntax_class_annotation(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *note = not_syntax_notes(syntax, parent);
	if (note == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node = NULL;
	if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
	{
		node = not_syntax_class_export(syntax, parent, note);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = not_syntax_class_reference(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = not_syntax_class_readonly(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = not_syntax_class_static(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = not_syntax_class_fun(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = not_syntax_class(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else
	{
		node = not_syntax_class_property(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return node;
}

static not_node_t *
not_syntax_class_block(not_syntax_t *syntax, not_node_t *parent)
{
	if (not_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;
	while (syntax->token->type != TOKEN_RBRACE)
	{
		not_node_t *item = NULL;

		if (syntax->token->type == TOKEN_AT)
		{
			item = not_syntax_class_annotation(syntax, parent);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = not_syntax_class_export(syntax, parent, NULL);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
		{
			item = not_syntax_class_reference(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
			if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			item = not_syntax_class_readonly(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
			if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_STATIC_KEYWORD)
		{
			item = not_syntax_class_static(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_FUN_KEYWORD)
		{
			item = not_syntax_class_fun(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_CLASS_KEYWORD)
		{
			item = not_syntax_class(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else
		{
			item = not_syntax_class_property(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
			if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}
	}

	if (not_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return declaration;
}

static not_node_t *
not_syntax_heritage(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_COLON) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *type = not_syntax_expression(syntax, node);
	if (type == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_heritage(node, key, type);
}

static not_node_t *
not_syntax_heritages(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;
	while (true)
	{
		not_node_t *item = not_syntax_heritage(syntax, parent);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_heritages(node, declaration);
}

static not_node_t *
not_syntax_class(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_CLASS_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_gt(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			not_error_syntax_by_position(syntax->token->position, "empty generic types");
			return NOT_PTR_ERROR;
		}

		generics = not_syntax_generics(syntax, node);
		if (!generics)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_gt(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *heritages = NULL;
	if (syntax->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_LPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}

		if (syntax->token->type == TOKEN_RPAREN)
		{
			not_error_syntax_by_position(syntax->token->position, "empty heritages");
			return NOT_PTR_ERROR;
		}

		heritages = not_syntax_heritages(syntax, node);
		if (!heritages)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	not_node_t *block = not_syntax_class_block(syntax, node);
	if (!block)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_class(node, note, flag, key, generics, heritages, block);
}

static not_node_t *
not_syntax_package(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *value = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
		value = not_syntax_expression(syntax, node);
	}

	return not_node_make_package(node, key, value);
}

static not_node_t *
not_syntax_packages(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_package(syntax, parent);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_COMMA)
		{
			break;
		}

		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_packages(node, declaration);
}

static not_node_t *
not_syntax_using(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_USING_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *packages = not_syntax_packages(syntax, node);
	if (packages == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_FROM_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *path = not_syntax_string(syntax, node);
	if (path == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_using(node, syntax->scanner->path, path, packages);
}

static not_node_t *
not_syntax_readonly(not_syntax_t *syntax, not_node_t *parent, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	not_node_t *node = not_syntax_var_stmt(syntax, parent, flag);

	flag &= ~SYNTAX_MODIFIER_READONLY;

	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return node;
}

static not_node_t *
not_syntax_static(not_syntax_t *syntax, not_node_t *parent, not_node_t *note, uint64_t flag)
{
	if (not_syntax_match(syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	not_node_t *node = not_syntax_class(syntax, parent, note, flag);

	flag &= ~SYNTAX_MODIFIER_STATIC;

	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return node;
}

static not_node_t *
not_syntax_export(not_syntax_t *syntax, not_node_t *parent, not_node_t *note)
{
	if (not_syntax_match(syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NOT_PTR_ERROR;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	not_node_t *node = NULL;

	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = not_syntax_static(syntax, parent, note, flag);
	}
	else if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = not_syntax_class(syntax, parent, note, flag);
	}
	else if ((syntax->token->type == TOKEN_READONLY_KEYWORD) && (note == NULL))
	{
		node = not_syntax_readonly(syntax, parent, flag);
		if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}
	else
	{
		if (note != NULL)
		{
			not_error_syntax_by_position(syntax->token->position,
										 "'Annotation' is not defined for variable",
										 __FILE__, __LINE__);
			return NOT_PTR_ERROR;
		}
		node = not_syntax_var_stmt(syntax, parent, flag);
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	return node;
}

static not_node_t *
not_syntax_note(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	if (not_syntax_match(syntax, TOKEN_AT) == -1)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *key = not_syntax_id(syntax, node);
	if (key == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *arguments = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (not_syntax_next(syntax) == -1)
		{
			return NOT_PTR_ERROR;
		}

		arguments = not_syntax_arguments(syntax, node);
		if (arguments == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (not_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NOT_PTR_ERROR;
		}
	}

	return not_node_make_note(node, key, arguments);
}

static not_node_t *
not_syntax_notes(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *node = not_node_create(parent, syntax->token->position);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		not_node_t *item = not_syntax_note(syntax, node);
		if (item == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}

		if (syntax->token->type != TOKEN_AT)
		{
			break;
		}
	}

	return not_node_make_notes(node, declaration);
}

static not_node_t *
not_syntax_annotation(not_syntax_t *syntax, not_node_t *parent)
{
	not_node_t *note = not_syntax_notes(syntax, parent);
	if (note == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *node = NULL;
	if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
	{
		node = not_syntax_export(syntax, parent, note);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}
	else
	{
		node = not_syntax_class(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NOT_PTR_ERROR)
		{
			return NOT_PTR_ERROR;
		}
	}

	return node;
}

not_node_t *
not_syntax_module(not_syntax_t *syntax)
{
	not_position_t pos = (not_position_t){
		.line = 0,
		.column = 0,
		.offset = 0,
		.length = 0,
		.path = syntax->token->position.path};

	not_node_t *node = not_node_create(NULL, pos);
	if (node == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	not_node_t *declaration = NULL, *top = NULL;
	while (syntax->token->type != TOKEN_EOF)
	{
		if (syntax->token->type == TOKEN_SEMICOLON)
		{
			if (not_syntax_next(syntax) == -1)
			{
				return NOT_PTR_ERROR;
			}
			continue;
		}

		not_node_t *item = NULL;
		if (syntax->token->type == TOKEN_USING_KEYWORD)
		{
			item = not_syntax_using(syntax, node);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
			if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_AT)
		{
			item = not_syntax_annotation(syntax, node);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = not_syntax_export(syntax, node, NULL);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			item = not_syntax_readonly(syntax, node, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
			if (not_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NOT_PTR_ERROR;
			}
		}
		else if (syntax->token->type == TOKEN_CLASS_KEYWORD)
		{
			item = not_syntax_class(syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}
		else
		{
			item = not_syntax_statement(syntax, node, SYNTAX_MODIFIER_NONE);
			if (item == NOT_PTR_ERROR)
			{
				return NOT_PTR_ERROR;
			}
		}

		if (declaration == NULL)
		{
			declaration = item;
			top = item;
		}
		else
		{
			top->next = item;
			item->previous = top;
			top = item;
		}
	}

	if (not_syntax_match(syntax, TOKEN_EOF) == -1)
	{
		return NOT_PTR_ERROR;
	}

	return not_node_make_module(node, declaration);
}

not_syntax_t *
not_syntax_create(char *path)
{
	not_syntax_t *syntax = (not_syntax_t *)not_memory_calloc(1, sizeof(not_syntax_t));
	if (!syntax)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	not_scanner_t *scanner = not_scanner_create(path);
	if (scanner == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	syntax->scanner = scanner;
	syntax->loop_depth = 0;
	syntax->token = &scanner->token;

	syntax->states = not_queue_create();
	if (syntax->states == NOT_PTR_ERROR)
	{
		return NOT_PTR_ERROR;
	}

	return syntax;
}

void not_syntax_destroy(not_syntax_t *syntax)
{
	not_queue_destroy(syntax->states);
	not_scanner_destroy(syntax->scanner);
	not_memory_free(syntax);
}