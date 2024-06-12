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


static sy_node_t *
sy_syntax_expression(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_statement(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag);

static sy_node_t *
sy_syntax_body(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_class(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag);

static sy_node_t *
sy_syntax_class_fun(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag);

static sy_node_t *
sy_syntax_parameters(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_generics(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_parameter(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_class_property(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag);

static sy_node_t *
sy_syntax_parenthesis(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_heritages(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_prefix(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_export(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note);

static sy_node_t *
sy_syntax_postfix(sy_syntax_t *syntax, sy_node_t *parent);

static sy_node_t *
sy_syntax_notes(sy_syntax_t *syntax, sy_node_t *parent);


int32_t
sy_syntax_save(sy_syntax_t *syntax)
{
	sy_scanner_t *scanner = syntax->scanner;

	sy_syntax_state_t *state = (sy_syntax_state_t *)sy_memory_calloc(1, sizeof(sy_syntax_state_t));
	if (state == NULL)
	{
		sy_error_no_memory();
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

	if (sy_queue_right_push(syntax->states, state) == NULL)
	{
		return -1;
	}

	return 1;
}

int32_t
sy_syntax_restore(sy_syntax_t *syntax)
{
	sy_scanner_t *scanner = syntax->scanner;

	sy_queue_entry_t *queue_entry = sy_queue_right_pop(syntax->states);
	if (queue_entry == NULL)
	{
		return -1;
	}

	sy_syntax_state_t *state = (sy_syntax_state_t *)queue_entry->value;
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

	sy_memory_free(state);

	return 1;
}

int32_t
sy_syntax_release(sy_syntax_t *syntax)
{
	sy_queue_entry_t *queue_entry = sy_queue_right_pop(syntax->states);
	if (queue_entry == NULL)
	{
		return -1;
	}

	sy_syntax_state_t *state = (sy_syntax_state_t *)queue_entry->value;
	if (state == NULL)
	{
		return -1;
	}

	sy_memory_free(state);
	return 1;
}

static void
sy_syntax_expected(sy_syntax_t *syntax, int32_t type)
{
	char message[1024];
	memset(message, 0 , 1024);

	switch (syntax->token->type)
	{
		case TOKEN_ID:
		case TOKEN_NUMBER:
		case TOKEN_CHAR:
		case TOKEN_STRING:
			sprintf(
					message,
					"expected '%s', got '%s'\n\tMajor:%s-%u",
					sy_token_get_name(type),
					syntax->token->value, __FILE__, __LINE__);
			break;

		default:
			sprintf(
					message,
					"expected '%s', got '%s'\n\tMajor:%s-%u",
					sy_token_get_name(type),
					sy_token_get_name(syntax->token->type), __FILE__, __LINE__);
			break;
	}

	sy_error_syntax_by_position(syntax->token->position, message);
}

static int32_t
sy_syntax_match(sy_syntax_t *syntax, int32_t type)
{
	if (syntax->token->type == type)
	{
		if (sy_scanner_advance(syntax->scanner) == -1)
		{
			return -1;
		}
	}
	else
	{
		sy_syntax_expected(syntax, type);
		return -1;
	}
	return 1;
}

static int32_t
sy_syntax_next(sy_syntax_t *syntax)
{
	if (sy_scanner_advance(syntax->scanner) == -1)
	{
		return -1;
	}
	return 1;
}

static int32_t
sy_syntax_gt(sy_syntax_t *syntax)
{
	return sy_scanner_gt(syntax->scanner);
}

static int32_t
sy_syntax_id_strcmp(sy_node_t *n1, char *name)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}


static sy_node_t *
sy_syntax_id(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_id(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_ID) == -1)
	{
		return NULL;
	}

	return node2;
}


static sy_node_t *
sy_syntax_number(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}
	
	sy_node_t *node2 = sy_node_make_number(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_NUMBER) == -1)
	{
		return NULL;
	}
	
	return node2;
}

static sy_node_t *
sy_syntax_char(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_char(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_CHAR) == -1)
	{
		return NULL;
	}

	return node2;
}

static sy_node_t *
sy_syntax_string(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_string(node, syntax->token->value);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_STRING) == -1)
	{
		return NULL;
	}

	return node2;
}


static sy_node_t *
sy_syntax_null(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_null(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_NULL_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static sy_node_t *
sy_syntax_kint(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_kint(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_INT_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static sy_node_t *
sy_syntax_kfloat(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_kfloat(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_FLOAT_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static sy_node_t *
sy_syntax_kchar(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_kchar(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_CHAR_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}

static sy_node_t *
sy_syntax_kstring(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_kstring(node);
	if (node2 == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_STRING_KEYWORD) == -1)
	{
		return NULL;
	}

	return node2;
}


static sy_node_t *
sy_syntax_operator(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	char *operator = NULL;
	if (syntax->token->type == TOKEN_PLUS)
	{
		if (sy_syntax_match(syntax, TOKEN_PLUS) == -1)
		{
			return NULL;
		}
		operator = "+";
	}
	else 
	if (syntax->token->type == TOKEN_MINUS)
	{
		if (sy_syntax_match(syntax, TOKEN_MINUS) == -1)
		{
			return NULL;
		}
		operator = "-";
	}
	else
	if (syntax->token->type == TOKEN_STAR)
	{
		if (sy_syntax_match(syntax, TOKEN_STAR) == -1)
		{
			return NULL;
		}
		operator = "*";
	}
	else 
	if (syntax->token->type == TOKEN_SLASH)
	{
		if (sy_syntax_match(syntax, TOKEN_SLASH) == -1)
		{
			return NULL;
		}
		operator = "/";
	}
	else 
	if (syntax->token->type == TOKEN_BACKSLASH)
	{
		if (sy_syntax_match(syntax, TOKEN_BACKSLASH) == -1)
		{
			return NULL;
		}
		operator = "\\";
	}
	else 
	if (syntax->token->type == TOKEN_POWER)
	{
		if (sy_syntax_match(syntax, TOKEN_POWER) == -1)
		{
			return NULL;
		}
		operator = "**";
	}
	else 
	if (syntax->token->type == TOKEN_PERCENT)
	{
		if (sy_syntax_match(syntax, TOKEN_PERCENT) == -1)
		{
			return NULL;
		}
		operator = "%";
	}
	else 
	if (syntax->token->type == TOKEN_AND)
	{
		if (sy_syntax_match(syntax, TOKEN_AND) == -1)
		{
			return NULL;
		}
		operator = "&";
	}
	else 
	if (syntax->token->type == TOKEN_OR)
	{
		if (sy_syntax_match(syntax, TOKEN_OR) == -1)
		{
			return NULL;
		}
		operator = "|";
	}
	else
	if (syntax->token->type == TOKEN_CARET)
	{
		if (sy_syntax_match(syntax, TOKEN_CARET) == -1)
		{
			return NULL;
		}
		operator = "^";
	}
	else 
	if (syntax->token->type == TOKEN_LT_LT)
	{
		if (sy_syntax_match(syntax, TOKEN_LT_LT) == -1)
		{
			return NULL;
		}
		operator = "<<";
	}
	else 
	if (syntax->token->type == TOKEN_GT_GT)
	{
		if (sy_syntax_match(syntax, TOKEN_GT_GT) == -1)
		{
			return NULL;
		}
		operator = ">>";
	}
	else 
	if (syntax->token->type == TOKEN_LT)
	{
		if (sy_syntax_match(syntax, TOKEN_LT) == -1)
		{
			return NULL;
		}
		operator = "<";
	}
	else 
	if (syntax->token->type == TOKEN_GT)
	{
		if (sy_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
		operator = ">";
	}
	else 
	if (syntax->token->type == TOKEN_LBRACKET)
	{
		if (sy_syntax_match(syntax, TOKEN_LBRACKET) == -1)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_RBRACKET) == -1)
		{
			return NULL;
		}
		operator = "[]";
	}
	else 
	{
		sy_error_syntax_by_position(syntax->token->position, "operator expected\n\tMajor:%s-%u", __FILE__, __LINE__);
		return NULL;
	}

	sy_node_t *node2 = sy_node_make_id(node, operator);
	if (node2 == NULL)
	{
		return NULL;
	}

	return node2;
}

static sy_node_t *
sy_syntax_tuple(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_LBRACKET) == -1)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	if (syntax->token->type != TOKEN_RBRACKET)
	{
		while (true)
		{
			sy_node_t *item = sy_syntax_expression(syntax, node);
			if (!item)
			{
				return NULL;
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

			if (sy_syntax_match(syntax, TOKEN_COMMA) == -1)
			{
				return NULL;
			}
		}
	}

	if (sy_syntax_match(syntax, TOKEN_RBRACKET) == -1)
	{
		return NULL;
	}

	return sy_node_make_tuple(node, declaration);
}

static sy_node_t *
sy_syntax_pair(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_expression(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_pair(node, key, value);
}

static sy_node_t *
sy_syntax_object(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;
	if (syntax->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			sy_node_t *item = sy_syntax_pair(syntax, node);
			if (item == NULL)
			{
				return NULL;
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

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}
		}
	}

	if (sy_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return sy_node_make_object(node, declaration);
}

static sy_node_t *
sy_syntax_parenthesis(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	sy_node_t *value = sy_syntax_expression(syntax, node);
	if (value == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	return sy_node_make_parenthesis(node, value);
}

static sy_node_t *
sy_syntax_lambda(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_FUN_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (sy_syntax_gt(syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			sy_error_syntax_by_position(syntax->token->position, "empty generic types\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		generics = sy_syntax_generics(syntax, node);
		if (generics == NULL)
		{
			return NULL;
		}

		if (sy_syntax_gt(syntax) == -1)
		{
			return NULL;
		}
		
		if (sy_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	sy_node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = sy_syntax_parameters(syntax, node);
			if (parameters == NULL)
			{
				return NULL;
			}
		}

		if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *return_type = sy_syntax_postfix(syntax, node);
		if (return_type == NULL)
		{
			return NULL;
		}

		sy_node_t *body = NULL;
		if (syntax->token->type == TOKEN_MINUS_GT)
		{
			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			body = sy_syntax_expression(syntax, node);
			if (body == NULL)
			{
				return NULL;
			}
		}
		else
		{
			body = sy_syntax_body(syntax, node);
			if (body == NULL)
			{
				return NULL;
			}
		}

		return sy_node_make_lambda(node, generics, parameters, body, return_type);
	}
	
	if (syntax->token->type == TOKEN_MINUS_GT)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *body = sy_syntax_expression(syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		return sy_node_make_lambda(node, generics, parameters, body, NULL);
	}

	if (syntax->token->type == TOKEN_LBRACE)
	{
		syntax->fun_depth += 1;

		sy_node_t *body = sy_syntax_body(syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		syntax->fun_depth -= 1;

		return sy_node_make_lambda(node, generics, parameters, body, NULL);
	}
	else
	{
		return sy_node_make_lambda(node, generics, parameters, NULL, NULL);
	}
}

static sy_node_t *
sy_syntax_primary(sy_syntax_t *syntax, sy_node_t *parent)
{
	if (syntax->token->type == TOKEN_NUMBER)
	{
		return sy_syntax_number(syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_CHAR)
	{
		return sy_syntax_char(syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_STRING)
	{
		return sy_syntax_string(syntax, parent);
	}
	else
	
	if (syntax->token->type == TOKEN_NULL_KEYWORD)
	{
		return sy_syntax_null(syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_INT_KEYWORD)
	{
		return sy_syntax_kint(syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_FLOAT_KEYWORD)
	{
		return sy_syntax_kfloat(syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_CHAR_KEYWORD)
	{
		return sy_syntax_kchar(syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_STRING_KEYWORD)
	{
		return sy_syntax_kstring(syntax, parent);
	}
	else

	if (syntax->token->type == TOKEN_LBRACKET)
	{
		return sy_syntax_tuple(syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LBRACE)
	{
		return sy_syntax_object(syntax, parent);
	}
	else 
	if (syntax->token->type == TOKEN_LPAREN)
	{
		return sy_syntax_parenthesis(syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		return sy_syntax_lambda(syntax, parent);
	}
	else 
	{
		return sy_syntax_id(syntax, parent);
	}
}

static sy_node_t *
sy_syntax_field(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_postfix(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		value = sy_syntax_postfix(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_field(node, key, value);
}

static sy_node_t *
sy_syntax_fields(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_field(syntax, node);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_fields(node, declaration);
}

static sy_node_t *
sy_syntax_argument(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_expression(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_argument(node, key, value);
}

static sy_node_t *
sy_syntax_arguments(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_argument(syntax, node);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_arguments(node, declaration);
}

static sy_node_t *
sy_syntax_postfix(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node2 = sy_syntax_primary(syntax, parent);
	if (node2 == NULL)
	{
		return NULL;
	}

	while (node2 != NULL)
	{
		if (syntax->token->type == TOKEN_LT)
		{
			if (sy_syntax_save(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *node = sy_node_create(node2, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			if (sy_syntax_gt(syntax) == -1)
			{
				return NULL;
			}

			if (syntax->token->type == TOKEN_GT)
			{
				sy_error_syntax_by_position(syntax->token->position, "empty generic types\n\tMajor:%s-%u", __FILE__, __LINE__);
				return NULL;
			}

			sy_node_t *concepts = sy_syntax_fields(syntax, node);
			if (concepts == NULL)
			{
				if (sy_syntax_restore(syntax) == -1)
				{
					return NULL;
				}
				sy_node_destroy(node);
				return node2;
			}
			else
			{
				if (sy_syntax_gt(syntax) == -1)
				{
					return NULL;
				}

				if (syntax->token->type != TOKEN_GT)
				{
					if (sy_syntax_restore(syntax) == -1)
					{
						return NULL;
					}
					sy_node_destroy(node);
					return node2;
				}
				else
				{
					if (sy_syntax_match(syntax, TOKEN_GT) == -1)
					{
						return NULL;
					}

					if (sy_syntax_release(syntax) == -1)
					{
						return NULL;
					}

					node2 = sy_node_make_pseudonym(node, node2, concepts);
					continue;
				}
			}
			continue;
		}
		else
		if (syntax->token->type == TOKEN_LBRACKET)
		{
			sy_node_t *node = sy_node_create(node2, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *arguments = NULL;
			if (syntax->token->type != TOKEN_RBRACKET)
			{
				arguments = sy_syntax_arguments(syntax, node);
				if (arguments == NULL)
				{
					return NULL;
				}
			}

			if (sy_syntax_match(syntax, TOKEN_RBRACKET) == -1)
			{
				return NULL;
			}

			node2 = sy_node_make_array(node, node2, arguments);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_DOT)
		{
			sy_node_t *node = sy_node_create(node2, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_id(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}
      
			node2 = sy_node_make_attribute(node, node2, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_LPAREN)
		{
			sy_node_t *node = sy_node_create(node2, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}	

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *arguments = NULL;
			if (syntax->token->type != TOKEN_RPAREN)
			{
				arguments = sy_syntax_arguments(syntax, node);
				if (arguments == NULL)
				{
					return NULL;
				}
			}

			if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
			{
				return NULL;
			}

			node2 = sy_node_make_call(node, node2, arguments);
			continue;
		}
		else
		{
			break;
		}
	}

	return node2;
}

static sy_node_t *
sy_syntax_prefix(sy_syntax_t *syntax, sy_node_t *parent)
{
	if (syntax->token->type == TOKEN_TILDE)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_prefix(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_tilde(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_NOT)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		
		sy_node_t *right = sy_syntax_prefix(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_not(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_MINUS)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		
		sy_node_t *right = sy_syntax_prefix(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_neg(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_PLUS)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		
		sy_node_t *right = sy_syntax_prefix(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_pos(node, right);
	}
	else
	if (syntax->token->type == TOKEN_SIZEOF_KEYWORD)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		
		sy_node_t *right = sy_syntax_prefix(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_sizeof(node, right);
	}
	else 
	if (syntax->token->type == TOKEN_TYPEOF_KEYWORD)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		
		sy_node_t *right = sy_syntax_prefix(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_typeof(node, right);
	}
	else
	{
		return sy_syntax_postfix(syntax, parent);
	}
}

static sy_node_t *
sy_syntax_pow(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_prefix(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_POWER)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_prefix(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_pow(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_multiplicative(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_pow(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_STAR)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_pow(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_mul(node, left, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_SLASH)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_pow(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_div(node, left, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_PERCENT)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_pow(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_mod(node, left, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_BACKSLASH)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_pow(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_epi(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_addative(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_multiplicative(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}	
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_PLUS)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_multiplicative(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_plus(node, left, right);
			continue;
		}
		else
		if (syntax->token->type == TOKEN_MINUS)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_multiplicative(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_minus(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_shifting(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_addative(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_LT_LT)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_addative(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_shl(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_GT)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_addative(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_shr(node, left, right);
	}

	return left;
}

static sy_node_t *
sy_syntax_relational(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_shifting(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_LT)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_shifting(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_lt(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_LT_EQ)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_shifting(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_le(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_GT)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_shifting(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_gt(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_EQ)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_shifting(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_ge(node, left, right);
	}

	return left;
}

static sy_node_t *
sy_syntax_equality(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_relational(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_EQ_EQ)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_relational(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_eq(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_NOT_EQ)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_relational(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_neq(node, left, right);
	}

	return left;
}

static sy_node_t *
sy_syntax_bitwise_and(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_equality(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_AND)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_equality(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_and(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_bitwise_xor(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_bitwise_and(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_CARET)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_bitwise_and(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_xor(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_bitwise_or(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_bitwise_xor(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_OR)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_bitwise_xor(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_or(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_logical_and(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_bitwise_or(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_AND_AND)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			sy_node_t *right = sy_syntax_bitwise_or(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_land(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_logical_or(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_logical_and(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	while (left != NULL)
	{
		if (syntax->token->type == TOKEN_OR_OR)
		{
			sy_node_t *node = sy_node_create(left, syntax->token->position);
			if (node == NULL)
			{
				return NULL;
			}

			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}
			
			sy_node_t *right = sy_syntax_logical_and(syntax, node);
			if (right == NULL)
			{
				return NULL;
			}

			left = sy_node_make_lor(node, left, right);
			continue;
		}
		else
		{
			break;
		}
	}

	return left;
}

static sy_node_t *
sy_syntax_instanceof(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_logical_or(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_INSTANCEOF_KEYWORD)
	{
		sy_node_t *node = sy_node_create(left, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_logical_or(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_instanceof(node, left, right);
	}

	return left;
}

static sy_node_t *
sy_syntax_conditional(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *condition = sy_syntax_instanceof(syntax, parent);
	if (condition == NULL)
	{
		return NULL;
	}
	
	if (syntax->token->type == TOKEN_QUESTION)
	{
		sy_node_t *node = sy_node_create(condition, syntax->token->position);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *left = sy_syntax_conditional(syntax, node);
		if (left == NULL)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_COLON) == -1)
		{
			return NULL;
		}
		
		sy_node_t *right = sy_syntax_conditional(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_conditional(node, condition, left, right);
	}

	return condition;
}

static sy_node_t *
sy_syntax_expression(sy_syntax_t *syntax, sy_node_t *parent)
{
	return sy_syntax_conditional(syntax, parent);
}

static sy_node_t *
sy_syntax_assign(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *left = sy_syntax_expression(syntax, parent);
	if (left == NULL)
	{
		return NULL;
	}

	if (syntax->token->type == TOKEN_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_PLUS_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_add_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_MINUS_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_sub_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_STAR_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_mul_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_SLASH_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_div_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_BACKSLASH_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_epi_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_PERCENT_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_mod_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_POWER_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_pow_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_AND_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_and_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_OR_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_or_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_LT_LT_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_shl_assign(node, left, right);
	}
	else
	if (syntax->token->type == TOKEN_GT_GT_EQ)
	{
		sy_node_t *node = sy_node_create(parent, syntax->token->position);
		
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		sy_node_t *right = sy_syntax_expression(syntax, node);
		if (right == NULL)
		{
			return NULL;
		}

		return sy_node_make_shr_assign(node, left, right);
	}

	return left;
}

static sy_node_t *
sy_syntax_if_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_IF_KEYWORD) == -1)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	sy_node_t *condition;
	condition = sy_syntax_expression(syntax, node);
	if (!condition)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	sy_node_t *then_body;
	then_body = sy_syntax_body(syntax, node);
	if (!then_body)
	{
		return NULL;
	}

	sy_node_t *else_body;
	else_body = NULL;
	if (syntax->token->type == TOKEN_ELSE_KEYWORD)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_IF_KEYWORD)
		{
			else_body = sy_syntax_if_stmt(syntax, node);
		}
		else
		{
			else_body = sy_syntax_body(syntax, node);
		}

		if (!else_body)
		{
			return NULL;
		}
	}

	return sy_node_make_if(node, condition, then_body, else_body);
}

static sy_node_t *
sy_syntax_entity(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		type = sy_syntax_id(syntax, node);
		if (type == NULL)
		{
			return NULL;
		}
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_entity(node, flag, key, type, value);
}

static sy_node_t *
sy_syntax_set(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_entity(syntax, parent, flag);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	if (sy_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return sy_node_make_set(node, declaration);
}

static sy_node_t *
sy_syntax_var_stmt(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}
	
	if (sy_syntax_match(syntax, TOKEN_VAR_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t set_used = 0;
	sy_node_t *key = NULL;
	if (syntax->token->type != TOKEN_ID)
	{
		key = sy_syntax_set(syntax, node, flag);
		if (key == NULL)
		{
			return NULL;
		}
		set_used = 1;
	}
	else
	{
		key = sy_syntax_id(syntax, node);
		if (key == NULL)
		{
			return NULL;
		}
	}
	
	sy_node_t *type = NULL;
	if ((syntax->token->type == TOKEN_COLON) && (set_used == 0))
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		type = sy_syntax_postfix(syntax, node);
		if (type == NULL)
		{
			return NULL;
		}
	}
	
	sy_node_t *value = NULL;
	if ((syntax->token->type == TOKEN_EQ) || (set_used == 1))
	{
		if (sy_syntax_match(syntax, TOKEN_EQ) == -1)
		{
			return NULL;
		}
		
		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}
	
	return sy_node_make_var(node, flag, key, type, value);
}

static sy_node_t *
sy_syntax_for_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_FOR_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = sy_syntax_id(syntax, node);
		if (key == NULL)
		{
			return NULL;
		}
	}

	sy_node_t *initializer = NULL;
	sy_node_t *incrementor = NULL;

	sy_node_t *condition = NULL;
	if (syntax->token->type == TOKEN_LBRACE)
	{
		syntax->loop_depth += 1;

		sy_node_t *body = sy_syntax_body(syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		syntax->loop_depth -= 1;

		return sy_node_make_for(node, key, initializer, condition, incrementor, body);
	}

	if (sy_syntax_match(syntax, TOKEN_LPAREN) == -1)
	{
		return NULL;
	}

	if (syntax->token->type == TOKEN_SEMICOLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		goto region_condition;
	}

	if (sy_syntax_save(syntax) < 0)
	{
		return NULL;
	}

	int32_t use_readonly = 0;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		use_readonly = 1;
	}

	if (use_readonly || (syntax->token->type == TOKEN_VAR_KEYWORD))
	{
		if (sy_syntax_match(syntax, TOKEN_VAR_KEYWORD) == -1)
		{
			return NULL;
		}

		sy_node_t *field = sy_syntax_id(syntax, node);
		if (field == NULL)
		{
			return NULL;
		}

		sy_node_t *value = NULL;
		if (syntax->token->type == TOKEN_COMMA)
		{
			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			value = sy_syntax_id(syntax, node);
			if (value == NULL)
			{
				return NULL;
			}
		}

		if (syntax->token->type != TOKEN_IN_KEYWORD)
		{
			if (sy_syntax_restore(syntax) < 0)
			{
				return NULL;
			}
			goto region_for_loop;
		}

		if (sy_syntax_release(syntax) < 0)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_IN_KEYWORD) == -1)
		{
			return NULL;
		}

		sy_node_t *iterator = sy_syntax_expression(syntax, node);
		if (iterator == NULL)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}

		syntax->loop_depth += 1;

		sy_node_t *body = sy_syntax_body(syntax, node);
		if (body == NULL)
		{
			return NULL;
		}

		syntax->loop_depth -= 1;

		return sy_node_make_forin(node, key, field, value, iterator, body);
	}

	if (sy_syntax_release(syntax) < 0)
	{
		return NULL;
	}

	region_for_loop:
	sy_node_t *top = NULL;

	while (true)
	{
		int32_t use_readonly = 0;
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}
			use_readonly = 1;
		}

		sy_node_t *item = NULL;
		if (use_readonly || (syntax->token->type == TOKEN_VAR_KEYWORD))
		{
			item = sy_syntax_var_stmt(syntax, node, use_readonly ? SYNTAX_MODIFIER_READONLY:SYNTAX_MODIFIER_NONE);
		}
		else
		{
			item = sy_syntax_assign(syntax, node);
		}
		
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
	{
		return NULL;
	}

	region_condition:
	if (syntax->token->type == TOKEN_SEMICOLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		goto region_step;
	}

	condition = sy_syntax_expression(syntax, node);
	if (condition == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
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
		sy_node_t *item = sy_syntax_assign(syntax, node);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	region_finish:
	if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
	{
		return NULL;
	}

	syntax->loop_depth += 1;

	sy_node_t *body = sy_syntax_body(syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	syntax->loop_depth -= 1;

	return sy_node_make_for(node, key, initializer, condition, incrementor, body);
}

static sy_node_t *
sy_syntax_break_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_BREAK_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = sy_syntax_id(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_break(node, value);
}

static sy_node_t *
sy_syntax_continue_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_CONTINUE_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = sy_syntax_id(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_continue(node, value);
}

static sy_node_t *
sy_syntax_catch_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_CATCH_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = sy_syntax_parameters(syntax, node);
			if (!parameters)
			{
				return NULL;
			}
		}
		if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	sy_node_t *body = sy_syntax_body(syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	return sy_node_make_catch(node, parameters, body);
}

static sy_node_t *
sy_syntax_try_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_TRY_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *body = sy_syntax_body(syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	sy_node_t *catchs = NULL;
	if (syntax->token->type == TOKEN_CATCH_KEYWORD)
	{
		catchs = sy_syntax_catch_stmt(syntax, node);
		if (catchs == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_try(node, body, catchs);
}

static sy_node_t *
sy_syntax_throw_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_THROW_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *value = sy_syntax_expression(syntax, node);
	if (value == NULL)
	{
		return NULL;
	}

	return sy_node_make_throw(node, value);
}

static sy_node_t *
sy_syntax_return_stmt(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_RETURN_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type != TOKEN_SEMICOLON)
	{
		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_return(node, value);
}

static sy_node_t *
sy_syntax_reference_stmt(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_REFERENCE_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_REFERENCE;

	sy_node_t *node = sy_syntax_var_stmt(syntax, parent, flag);

	flag &= ~SYNTAX_MODIFIER_REFERENCE;

	if (node == NULL)
	{
		return NULL;
	}

	return node;
}

static sy_node_t *
sy_syntax_readonly_stmt(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	sy_node_t *node = NULL;
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = sy_syntax_reference_stmt(syntax, parent, flag);
	}
	else
	{
		node = sy_syntax_var_stmt(syntax, parent, flag);
	}

	flag &= ~SYNTAX_MODIFIER_READONLY;

	if (node == NULL)
	{
		return NULL;
	}

	return node;
}

static sy_node_t *
sy_syntax_statement(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	sy_node_t *node = NULL;

	if (syntax->token->type == TOKEN_IF_KEYWORD)
	{
		node = sy_syntax_if_stmt(syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_TRY_KEYWORD)
	{
		node = sy_syntax_try_stmt(syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_FOR_KEYWORD)
	{
		node = sy_syntax_for_stmt(syntax, parent);
	}
	else
	if (syntax->token->type == TOKEN_VAR_KEYWORD)
	{
		node = sy_syntax_var_stmt(syntax, parent, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = sy_syntax_readonly_stmt(syntax, parent, flag);
	}
	else
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = sy_syntax_reference_stmt(syntax, parent, flag);
	}
	else
	if ((syntax->token->type == TOKEN_BREAK_KEYWORD) && (syntax->loop_depth > 0))
	{
		node = sy_syntax_break_stmt(syntax, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if ((syntax->token->type == TOKEN_CONTINUE_KEYWORD) && (syntax->loop_depth > 0))
	{
		node = sy_syntax_continue_stmt(syntax, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if ((syntax->token->type == TOKEN_RETURN_KEYWORD) && (syntax->fun_depth > 0))
	{
		node = sy_syntax_return_stmt(syntax, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_THROW_KEYWORD)
	{
		node = sy_syntax_throw_stmt(syntax, parent);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	{
		node = sy_syntax_assign(syntax, parent);
		if (node == NULL)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	return node;
}

static sy_node_t *
sy_syntax_body(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;
	
	while (syntax->token->type != TOKEN_RBRACE)
	{
		if (syntax->token->type == TOKEN_SEMICOLON)
		{
			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}

			continue;
		}

		sy_node_t *item = sy_syntax_statement(syntax, node, SYNTAX_MODIFIER_NONE);
		if (item == NULL)
		{
			return NULL;
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

	if (sy_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return sy_node_make_body(node, declaration);
}

static sy_node_t *
sy_syntax_parameter(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_NONE;
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		flag |= SYNTAX_MODIFIER_REFERENCE;
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}
	else 
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		flag |= SYNTAX_MODIFIER_READONLY;
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	if (syntax->token->type == TOKEN_STAR)
	{
		flag |= SYNTAX_MODIFIER_KARG;
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_POWER)
	{
		flag |= SYNTAX_MODIFIER_KWARG;
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}	
	
	sy_node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		type = sy_syntax_postfix(syntax, node);
		if (type == NULL)
		{
			return NULL;
		}
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if ((flag & SYNTAX_MODIFIER_REFERENCE) == SYNTAX_MODIFIER_REFERENCE)
		{
			sy_error_syntax_by_position(syntax->token->position, "reference parameter by value\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_parameter(node, flag, key, type, value);
}

static sy_node_t *
sy_syntax_parameters(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_parameter(syntax, parent);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_parameters(node, declaration);
}

static sy_node_t *
sy_syntax_generic(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key;
	key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *type = NULL;
	if (syntax->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		type = sy_syntax_postfix(syntax, node);
		if (type == NULL)
		{
			return NULL;
		}
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_EQ)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		value = sy_syntax_postfix(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_generic(node, key, type, value);
}

static sy_node_t *
sy_syntax_generics(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_generic(syntax, parent);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_generics(node, declaration);
}

static sy_node_t *
sy_syntax_class_fun(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_FUN_KEYWORD) == -1)
	{
		return NULL;
	}

	int32_t used_constructor = 0, used_operator = 0;
	sy_node_t *key = NULL;
	if (syntax->token->type == TOKEN_ID)
	{
		key = sy_syntax_id(syntax, node);
		if (key == NULL)
		{
			return NULL;
		}
		
		if (sy_syntax_id_strcmp(key, CONSTRUCTOR_STR) == 1)
		{
			used_constructor = 1;
		}
	}
	else
	{
		key = sy_syntax_operator(syntax, node);
		if (key == NULL)
		{
			return NULL;
		}

		used_operator = 1;
	}

	sy_node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (used_constructor == 1)
		{
			sy_error_syntax_by_position(syntax->token->position, "constructor with generic types\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		if (used_operator == 1)
		{
			sy_error_syntax_by_position(syntax->token->position, "operator with generic types\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		if (sy_syntax_gt(syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			sy_error_syntax_by_position(syntax->token->position, "empty generic types\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		generics = sy_syntax_generics(syntax, node);
		if (!generics)
		{
			return NULL;
		}

		if (sy_syntax_gt(syntax) == -1)
		{
			return NULL;
		}
		
		if (sy_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	sy_node_t *parameters = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type != TOKEN_RPAREN)
		{
			parameters = sy_syntax_parameters(syntax, node);
			if (!parameters)
			{
				return NULL;
			}
		}

		if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	sy_node_t *result = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		result = sy_syntax_postfix(syntax, node);
		if (result == NULL)
		{
			return NULL;
		}
	}

	syntax->fun_depth += 1;

	sy_node_t *body = sy_syntax_body(syntax, node);
	if (body == NULL)
	{
		return NULL;
	}

	syntax->fun_depth -= 1;

	return sy_node_make_func(node, note, flag, key, generics, parameters, result, body);
}

static sy_node_t *
sy_syntax_class_property(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *type = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		type = sy_syntax_postfix(syntax, node);
		if (type == NULL)
		{
			return NULL;
		}
	}

	sy_node_t *value = NULL;
	if ((syntax->token->type == TOKEN_EQ) || ((flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC))
	{
		if (sy_syntax_match(syntax, TOKEN_EQ) == -1)
		{
			return NULL;
		}

		value = sy_syntax_expression(syntax, node);
		if (value == NULL)
		{
			return NULL;
		}
	}

	return sy_node_make_property(node, note, flag, key, type, value);
}

static sy_node_t *
sy_syntax_class_reference(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_REFERENCE_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_REFERENCE;

	sy_node_t *node = sy_syntax_class_property(syntax, parent, note, flag);
	if (node == NULL)
	{
		return NULL;
	}

	flag &= ~SYNTAX_MODIFIER_REFERENCE;

	return node;
}

static sy_node_t *
sy_syntax_class_readonly(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	sy_node_t *node = sy_syntax_class_property(syntax, parent, note, flag);
	if (node == NULL)
	{
		return NULL;
	}

	flag &= ~SYNTAX_MODIFIER_READONLY;

	return node;
}

static sy_node_t *
sy_syntax_class_static(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	sy_node_t *node = NULL;
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = sy_syntax_class_readonly(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = sy_syntax_class_fun(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = sy_syntax_class(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = sy_syntax_class_property(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	flag &= ~SYNTAX_MODIFIER_STATIC;

	return node;
}

static sy_node_t *
sy_syntax_class_export(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note)
{
	if (sy_syntax_match(syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	sy_node_t *node = NULL;
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = sy_syntax_class_reference(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = sy_syntax_class_readonly(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = sy_syntax_class_static(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = sy_syntax_class_fun(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = sy_syntax_class(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = sy_syntax_class_property(syntax, parent, note, flag);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	return node;
}

static sy_node_t *
sy_syntax_class_annotation(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *note = sy_syntax_notes(syntax, parent);
	if (note == NULL)
	{
		return NULL;
	}

	sy_node_t *node = NULL;
	if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
	{
		node = sy_syntax_class_export(syntax, parent, note);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
	{
		node = sy_syntax_class_reference(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_READONLY_KEYWORD)
	{
		node = sy_syntax_class_readonly(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = sy_syntax_class_static(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_FUN_KEYWORD)
	{
		node = sy_syntax_class_fun(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = sy_syntax_class(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = sy_syntax_class_property(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
		if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
		{
			return NULL;
		}
	}

	return node;
}

static sy_node_t *
sy_syntax_class_block(sy_syntax_t *syntax, sy_node_t *parent)
{
	if (sy_syntax_match(syntax, TOKEN_LBRACE) == -1)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;
	while (syntax->token->type != TOKEN_RBRACE)
	{
		sy_node_t *item = NULL;

		if (syntax->token->type == TOKEN_AT)
		{
			item = sy_syntax_class_annotation(syntax, parent);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = sy_syntax_class_export(syntax, parent, NULL);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_REFERENCE_KEYWORD)
		{
			item = sy_syntax_class_reference(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			item = sy_syntax_class_readonly(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_STATIC_KEYWORD)
		{
			item = sy_syntax_class_static(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_FUN_KEYWORD)
		{
			item = sy_syntax_class_fun(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_CLASS_KEYWORD)
		{
			item = sy_syntax_class(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		{
			item = sy_syntax_class_property(syntax, parent, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
			if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NULL;
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

	if (sy_syntax_match(syntax, TOKEN_RBRACE) == -1)
	{
		return NULL;
	}

	return declaration;
}

static sy_node_t *
sy_syntax_heritage(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}
	
	if (sy_syntax_match(syntax, TOKEN_COLON) == -1)
	{
		return NULL;
	}
	
	sy_node_t *type = sy_syntax_expression(syntax, node);
	if (type == NULL)
	{
		return NULL;
	}

	return sy_node_make_heritage(node, key, type);
}

static sy_node_t *
sy_syntax_heritages(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;
	while (true)
	{
		sy_node_t *item = sy_syntax_heritage(syntax, parent);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_heritages(node, declaration);
}

static sy_node_t *
sy_syntax_class(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_CLASS_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *generics = NULL;
	if (syntax->token->type == TOKEN_LT)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (sy_syntax_gt(syntax) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_GT)
		{
			sy_error_syntax_by_position(syntax->token->position, "empty generic types\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		generics = sy_syntax_generics(syntax, node);
		if (!generics)
		{
			return NULL;
		}

		if (sy_syntax_gt(syntax) == -1)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_GT) == -1)
		{
			return NULL;
		}
	}

	sy_node_t *heritages = NULL;
	if (syntax->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_LPAREN) == -1)
		{
			return NULL;
		}

		if (syntax->token->type == TOKEN_RPAREN)
		{
			sy_error_syntax_by_position(syntax->token->position, "empty heritages\n\tMajor:%s-%u", __FILE__, __LINE__);
			return NULL;
		}

		heritages = sy_syntax_heritages(syntax, node);
		if (!heritages)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}

	}

	sy_node_t *block = sy_syntax_class_block(syntax, node);
	if (!block)
	{
		return NULL;
	}

	return sy_node_make_class(node, note, flag, key, generics, heritages, block);
}

static sy_node_t *
sy_syntax_package(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *value = NULL;
	if (syntax->token->type == TOKEN_COLON)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
		value = sy_syntax_expression(syntax, node);
	}

	return sy_node_make_package(node, key, value);
}

static sy_node_t *
sy_syntax_packages(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_package(syntax, parent);
		if (item == NULL)
		{
			return NULL;
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

		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_packages(node, declaration);
}

static sy_node_t *
sy_syntax_using(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_USING_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *packages = sy_syntax_packages(syntax, node);
	if (packages == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_FROM_KEYWORD) == -1)
	{
		return NULL;
	}

	sy_node_t *path = sy_syntax_string(syntax, node);
	if (path == NULL)
	{
		return NULL;
	}

	return sy_node_make_using(node, path, packages);
}

static sy_node_t *
sy_syntax_readonly(sy_syntax_t *syntax, sy_node_t *parent, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_READONLY_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_READONLY;

	sy_node_t *node = sy_syntax_var_stmt(syntax, parent, flag);

	flag &= ~SYNTAX_MODIFIER_READONLY;

	if (node == NULL)
	{
		return NULL;
	}

	return node;
}

static sy_node_t *
sy_syntax_static(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note, uint64_t flag)
{
	if (sy_syntax_match(syntax, TOKEN_STATIC_KEYWORD) == -1)
	{
		return NULL;
	}

	flag |= SYNTAX_MODIFIER_STATIC;

	sy_node_t *node = sy_syntax_class(syntax, parent, note, flag);

	flag &= ~SYNTAX_MODIFIER_STATIC;

	if (node == NULL)
	{
		return NULL;
	}

	return node;
}

static sy_node_t *
sy_syntax_export(sy_syntax_t *syntax, sy_node_t *parent, sy_node_t *note)
{
	if (sy_syntax_match(syntax, TOKEN_EXPORT_KEYWORD) == -1)
	{
		return NULL;
	}

	uint64_t flag = SYNTAX_MODIFIER_EXPORT;

	sy_node_t *node = NULL;

	if (syntax->token->type == TOKEN_STATIC_KEYWORD)
	{
		node = sy_syntax_static(syntax, parent, note, flag);
	}
	else
	if (syntax->token->type == TOKEN_CLASS_KEYWORD)
	{
		node = sy_syntax_class(syntax, parent, note, flag);
	}
	else
	if ((syntax->token->type == TOKEN_READONLY_KEYWORD) && (note == NULL))
	{
		node = sy_syntax_readonly(syntax, parent, flag);
	}
	else
	{
		if (note != NULL)
		{
			sy_error_syntax_by_position(syntax->token->position, 
				"'Annotation' is not defined for variable\n\tMajor:%s-%u",
				__FILE__, __LINE__);
			return NULL;
		}
		node = sy_syntax_var_stmt(syntax, parent, flag);
	}

	flag &= ~SYNTAX_MODIFIER_EXPORT;

	return node;
}

static sy_node_t *
sy_syntax_note(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	if (sy_syntax_match(syntax, TOKEN_AT) == -1)
	{
		return NULL;
	}

	sy_node_t *key = sy_syntax_id(syntax, node);
	if (key == NULL)
	{
		return NULL;
	}

	sy_node_t *arguments = NULL;
	if (syntax->token->type == TOKEN_LPAREN)
	{
		if (sy_syntax_next(syntax) == -1)
		{
			return NULL;
		}

		arguments = sy_syntax_arguments(syntax, node);
		if (arguments == NULL)
		{
			return NULL;
		}

		if (sy_syntax_match(syntax, TOKEN_RPAREN) == -1)
		{
			return NULL;
		}
	}

	return sy_node_make_note(node, key, arguments);
}

static sy_node_t *
sy_syntax_notes(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *node = sy_node_create(parent, syntax->token->position);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;

	while (true)
	{
		sy_node_t *item = sy_syntax_note(syntax, node);
		if (item == NULL)
		{
			return NULL;
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

	return sy_node_make_notes(node, declaration);
}

static sy_node_t *
sy_syntax_annotation(sy_syntax_t *syntax, sy_node_t *parent)
{
	sy_node_t *note = sy_syntax_notes(syntax, parent);
	if (note == NULL)
	{
		return NULL;
	}

	sy_node_t *node = NULL;
	if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
	{
		node = sy_syntax_export(syntax, parent, note);
		if (node == NULL)
		{
			return NULL;
		}
	}
	else
	{
		node = sy_syntax_class(syntax, parent, note, SYNTAX_MODIFIER_NONE);
		if (node == NULL)
		{
			return NULL;
		}
	}

	return node;
}

sy_node_t *
sy_syntax_module(sy_syntax_t *syntax)
{
	sy_position_t pos = (sy_position_t) {
		.line = 0,
		.column = 0,
		.offset = 0,
		.length = 0,
		.path = syntax->token->position.path
	};

	sy_node_t *node = sy_node_create(NULL, pos);
	if (node == NULL)
	{
		return NULL;
	}

	sy_node_t *declaration = NULL, *top = NULL;
	while (syntax->token->type != TOKEN_EOF)
	{
		if (syntax->token->type == TOKEN_SEMICOLON)
		{
			if (sy_syntax_next(syntax) == -1)
			{
				return NULL;
			}
			continue;
		}

		sy_node_t *item = NULL;
		if (syntax->token->type == TOKEN_USING_KEYWORD)
		{
			item = sy_syntax_using(syntax, node);
			if (item == NULL)
			{
				return NULL;
			}
			if (sy_syntax_match(syntax, TOKEN_SEMICOLON) == -1)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_AT)
		{
			item = sy_syntax_annotation(syntax, node);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_EXPORT_KEYWORD)
		{
			item = sy_syntax_export(syntax, node, NULL);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_READONLY_KEYWORD)
		{
			item = sy_syntax_readonly(syntax, node, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		if (syntax->token->type == TOKEN_CLASS_KEYWORD)
		{
			item = sy_syntax_class(syntax, node, NULL, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
			}
		}
		else
		{
			item = sy_syntax_statement(syntax, node, SYNTAX_MODIFIER_NONE);
			if (item == NULL)
			{
				return NULL;
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

	if (sy_syntax_match(syntax, TOKEN_EOF) == -1)
	{
		return NULL;
	}

	return sy_node_make_module(node, declaration);
}

sy_syntax_t *
sy_syntax_create(char *path)
{
	sy_syntax_t *syntax = (sy_syntax_t *)sy_memory_calloc(1, sizeof(sy_syntax_t));
	if (!syntax)
	{
		sy_error_no_memory();
		return NULL;
	}

	sy_scanner_t *scanner = sy_scanner_create(path);
	if (!scanner)
	{
		return NULL;
	}

	syntax->scanner = scanner;
	syntax->loop_depth = 0;
	syntax->token = &scanner->token;

	syntax->states = sy_queue_create();
	if (!syntax->states)
	{
		return NULL;
	}

	return syntax;
}

void
sy_syntax_destroy(sy_syntax_t *syntax)
{
	sy_queue_destroy(syntax->states);
	sy_memory_free(syntax);
}