#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>

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
#include "forecast.h"
#include "error.h"

static node_t *
parser_expression(parser_t *parser);

static node_t *
parser_statement(parser_t *parser);

static node_t *
parser_block_stmt(parser_t *parser);

static node_t *
parser_class(parser_t *parser, uint64_t flag);

static node_t *
parser_func_stmt(parser_t *parser);

static node_t *
parser_namespace(parser_t *parser);

static list_t *
parser_parameters(parser_t *parser);

static list_t *
parser_type_parameters(parser_t *parser);

static list_t *
parser_type_arguments(parser_t *parser);

static node_t *
parser_expression(parser_t *parser);

static node_t *
parser_parameter(parser_t *parser);

static node_t *
parser_parenthesis(parser_t *parser);

static list_t *
parser_heritages(parser_t *parser);

static node_t *
parser_prefix(parser_t *parser);

static node_t *
parser_object(parser_t *parser);

static node_t *
parser_export(parser_t *parser);


int32_t
parser_save_state(parser_t *parser)
{
	scanner_t * scanner;
	scanner = parser->scanner;

	parser_state_t *state;
	if(!(state = (parser_state_t *)malloc(sizeof(parser_state_t))))
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

	if(!list_rpush(parser->states, (list_value_t)state))
	{
		return 0;
	}

	return 1;
}

int32_t
parser_restore_state(parser_t *parser)
{
	scanner_t * scanner;
	scanner = parser->scanner;

	parser_state_t *state;
	state = (parser_state_t *)list_content(list_rpop(parser->states));
	if(!state)
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
parser_release_state(parser_t *parser)
{
	parser_state_t *state;
	state = (parser_state_t *)list_content(list_rpop(parser->states));
	if(!state)
	{
		return 0;
	}

	free(state);
	return 1;
}


static error_t *
parser_error(parser_t *parser, position_t position, const char *format, ...)
{
	char *message;
	message = malloc(1024);
	if(!message)
	{
		return NULL;
	}

    va_list arg;
    if (format)
	{
        va_start (arg, format);
        vsprintf(message, format, arg);
        va_end (arg);
    }

	error_t * error;
	error = error_create(position, message);
	if(!error)
	{
		return NULL;
	}

	if(list_rpush(parser->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}

static error_t *
parser_expected(parser_t *parser, int32_t type)
{
	char *message;
	message = malloc(1024);
	if(!message)
	{
		return NULL;
	}

	switch(parser->token->type)
	{
	case TOKEN_ID:
	case TOKEN_NUMBER:
	case TOKEN_CHAR:
	case TOKEN_STRING:
		sprintf(
			message,
			"expected '%s', got '%s'\n",
			token_get_name(type),
			parser->token->value
		);
		break;

	default:
		sprintf(
			message,
			"expected '%s', got '%s'\n",
			token_get_name(type),
			token_get_name(parser->token->type)
		);
		break;
	}

	error_t * error;
	error = error_create(parser->token->position, message);
	if(!error)
	{
		return NULL;
	}

	if(list_rpush(parser->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}

static int32_t
parser_match(parser_t *parser, int32_t type)
{
	if(parser->token->type == type){
		if(!scanner_advance(parser->scanner)){
			return 0;
		} 
	} else {
		parser_expected(parser, type);
		return 0;
	}
	return 1;
}


static node_t *
parser_id(parser_t *parser)
{
	node_t *node;
	node = node_make_id(parser->token->position, parser->token->value);
	if(!(node)){
		return NULL;
	}
	if(!parser_match(parser, TOKEN_ID)){
		return NULL;
	}
	return node;
}

static node_t *
parser_number(parser_t *parser)
{
	node_t *node;
	node = node_make_number(parser->token->position, parser->token->value);
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_NUMBER)){
		return NULL;
	}
	return node;
}
 
static node_t *
parser_char(parser_t *parser)
{
	node_t *node;
	node = node_make_char(parser->token->position, parser->token->value);
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_CHAR)){
		return NULL;
	}
	return node;
}

static node_t *
parser_string(parser_t *parser)
{
	node_t *node;
	node = node_make_string(parser->token->position, parser->token->value);
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_STRING)){
		return NULL;
	}
	return node;
}

static node_t *
parser_null(parser_t *parser)
{
	node_t *node;
	node = node_make_null(parser->token->position);
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_NULL_KEYWORD)){
		return NULL;
	}
	return node;
}

static node_t *
parser_true(parser_t *parser)
{
	node_t *node;
	node = node_make_true(parser->token->position);
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_TRUE_KEYWORD)){
		return NULL;
	}
	return node;
}

static node_t *
parser_false(parser_t *parser)
{
	node_t *node;
	node = node_make_false(parser->token->position);
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_FALSE_KEYWORD)){
		return NULL;
	}
	return node;
}

static node_t *
parser_array(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_LBRACKET))
	{
		return NULL;
	}

	list_t *expr_list;
	expr_list = list_create();
	if(!expr_list)
	{
		return NULL;
	}
	
	node_t *expr = NULL;
	while (parser->token->type != TOKEN_RBRACKET)
	{
		expr = parser_expression(parser);
		if (!expr)
		{
			return NULL;
		}

		if(!list_rpush(expr_list, (list_value_t)expr))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA))
		{
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_RBRACKET))
	{
		return NULL;
	}

	return node_make_array(position, expr_list);
}

static node_t *
parser_object_property(parser_t *parser)
{
	position_t position = parser->token->position;

	int32_t use_string = 0;
	node_t *name;
	if(parser->token->type == TOKEN_LBRACE)
	{
		name = parser_object(parser);
	}
	else if(parser->token->type == TOKEN_STRING)
	{
		name = parser_string(parser);
		use_string = 1;
	}
	else
	{
		name = parser_id(parser);
	}
	if (!name)
	{
		return NULL;
	}

	node_t *value = NULL;
	if(parser->token->type == TOKEN_COLON || use_string)
	{
		if(!parser_match(parser, TOKEN_COLON))
		{
			return NULL;
		}

		value = parser_expression(parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_object_property(position, name, value);
}

static node_t *
parser_object(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_LBRACE))
	{
		return NULL;
	}
	
	list_t *property_list;
	property_list = list_create();
	if(!property_list)
	{
		return NULL;
	}
	
	if(parser->token->type != TOKEN_RBRACE)
	{
		while (true)
		{
			node_t *property;
			property = parser_object_property(parser);
			if (!property)
			{
				return NULL;
			}

			if(!list_rpush(property_list, (list_value_t)property))
			{
				return NULL;
			}

			if (parser->token->type != TOKEN_COMMA)
			{
				break;
			}
			if(!parser_match(parser, TOKEN_COMMA))
			{
				return NULL;
			}
		}
	}

	if(!parser_match(parser, TOKEN_RBRACE))
	{
		return NULL;
	}

	return node_make_object(position, property_list);
}

static node_t *
parser_parenthesis(parser_t *parser)
{
	position_t position = parser->token->position;
	if(!parser_match(parser, TOKEN_LPAREN))
	{
		return NULL;
	}
	node_t *expr;
	expr = parser_expression(parser);
	if(!expr)
	{
		return NULL;
	}
	if(!parser_match(parser, TOKEN_RPAREN))
	{
		return NULL;
	}
	return node_make_parenthesis(position, expr);
}

static node_t *
parser_primary(parser_t *parser)
{
	node_t *node = NULL;
	switch (parser->token->type)
	{
	case TOKEN_ID:
		node = parser_id(parser);
		break;
	
	case TOKEN_NUMBER:
		node = parser_number(parser);
		break;
	
	case TOKEN_STRING:
		node = parser_string(parser);
		break;

	case TOKEN_CHAR:
		node = parser_char(parser);
		break;
	
	case TOKEN_NULL_KEYWORD:
		node = parser_null(parser);
		break;

	case TOKEN_TRUE_KEYWORD:
		node = parser_true(parser);
		break;

	case TOKEN_FALSE_KEYWORD:
		node = parser_false(parser);
		break;
		
	case TOKEN_LBRACKET:
		node = parser_array(parser);
		break;
		
	case TOKEN_LBRACE:
		node = parser_object(parser);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func_stmt(parser);
		break;

	case TOKEN_LPAREN:
		node = parser_parenthesis(parser);
		break;
		
	default:
		parser_error(parser, parser->token->position, "expression expected\n");
		return NULL;
	}
	
	return node;
}




static list_t *
parser_type_arguments(parser_t *parser)
{
	list_t *type_arguments;
	type_arguments = list_create();
	if(!type_arguments)
	{
		return NULL;
	}
	
	while (true)
	{
		node_t *argument;
		argument = parser_expression(parser);
		if (!argument)
		{
			return NULL;
		}

		if(!list_rpush(type_arguments, (list_value_t)argument))
		{
			return NULL;
		}

		if (parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		
		if(!parser_match(parser, TOKEN_COMMA))
		{
			return NULL;
		}
	}
	
	return type_arguments;
}

static list_t *
parser_arguments(parser_t *parser)
{
	list_t *arguments;
	arguments = list_create();
	if(!arguments){
		return NULL;
	}

	node_t *node;
	while(true){
		node = parser_expression(parser);
		if(!(node)){
			return NULL;
		}
	
		if(!list_rpush(arguments, (list_value_t)node)){
			return NULL;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	
	return arguments;
}

static node_t *
parser_postfix(parser_t *parser)
{
	node_t *node;
	node = parser_primary(parser);
	if(!node)
	{
		return NULL;
	}
	
	node_t *start;
	node_t *step;
	node_t *stop;
	int32_t slice;
	int32_t use_trinary;
	list_t *arguments;
	node_t *right;
	list_t *type_arguments;
	while(node)
	{
		position_t position = parser->token->position;

		switch(parser->token->type)
		{
		case TOKEN_LBRACE:
			if(!parser_match(parser, TOKEN_LBRACE))
			{
				return NULL;
			}

			if(parser->token->type == TOKEN_RBRACE)
			{
				parser_error(parser, parser->token->position, "type arguments is empty");
			}

			type_arguments = parser_type_arguments(parser);
			if(!(type_arguments))
			{
				return NULL;
			}

			if(!parser_match(parser, TOKEN_RBRACE))
			{
				return NULL;
			}

			node = node_make_composite(position, node, type_arguments);
			break;

		case TOKEN_LPAREN:
			if(!parser_match(parser, TOKEN_LPAREN))
			{
				return NULL;
			}
			
			arguments = NULL;
			if(parser->token->type != TOKEN_RPAREN)
			{
				arguments = parser_arguments(parser);
				if(!(arguments)){
					return NULL;
				}
			}
			
			if(!parser_match(parser, TOKEN_RPAREN))
			{
				return NULL;
			}
			node = node_make_call(position, node, arguments);
			break;
		
		case TOKEN_LBRACKET:
			if(!parser_match(parser, TOKEN_LBRACKET))
			{
				return NULL;
			}

			slice = 0;
			use_trinary = 0;
			start = NULL;
			if (parser->token->type == TOKEN_COLON)
			{
				if(!parser_match(parser, TOKEN_COLON))
				{
					return NULL;
				}
				slice = 1;
			} 
			else if(parser->token->type == TOKEN_RBRACKET)
			{
				parser_error(parser, position, "The index must contain a value");
				return NULL;
			}
			else 
			{
				start = parser_expression(parser);
				if (!start)
				{
					return NULL;
				}
				if (parser->token->type == TOKEN_COLON)
				{
					if(!parser_match(parser, TOKEN_COLON))
					{
						return NULL;
					}
					slice = 1;
				}
			}
			
			step = NULL;
			stop = NULL;
			if(slice)
			{
				if (parser->token->type != TOKEN_RBRACKET)
				{
					if(parser->token->type == TOKEN_COLON)
					{
						parser_error(parser, position, "The slice index must contain a step value");
						return NULL;
					}
					step = parser_expression(parser);
					if (!step)
					{
						return NULL;
					}
					
					if (parser->token->type == TOKEN_COLON)
					{
						if(!parser_match(parser, TOKEN_COLON))
						{
							return NULL;
						}
						use_trinary = 1;

						if (parser->token->type != TOKEN_RBRACKET)
						{
							stop = parser_expression(parser);
							if (!stop)
							{
								return NULL;
							}
						}
					}
				}
			}
			if(!parser_match(parser, TOKEN_RBRACKET))
			{
				return NULL;
			}

			if (slice)
			{
				if(!stop && !use_trinary)
				{
					stop = step;
					step = NULL;
				}
				node = node_make_get_slice(position, node, start, stop, step);
			}
			else
			{
				node = node_make_get_item(position, node, start);
			}
			break;
			
		case TOKEN_DOT:
			if(!parser_match(parser, TOKEN_DOT))
			{
				return NULL;
			}
			
			right = parser_id(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_get_attr(position, node, right);
			break;

		default:
			return node;
			
		}
	}
	
	return node;
}

static node_t *
parser_prefix(parser_t *parser)
{
	position_t position = parser->token->position;
	
	node_t *node = NULL;
	node_t *right;
	switch (parser->token->type)
	{
	case TOKEN_TILDE:
		if(!parser_match(parser, TOKEN_TILDE))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_tilde(position, right);
		break;

	case TOKEN_NOT:
		if(!parser_match(parser, TOKEN_NOT))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_not(position, right);
		break;

	case TOKEN_MINUS:
		if(!parser_match(parser, TOKEN_MINUS))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_neg(position, right);
		break;

	case TOKEN_PLUS:
		if(!parser_match(parser, TOKEN_PLUS))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_pos(position, right);
		break;

	case TOKEN_STAR:
		if(!parser_match(parser, TOKEN_STAR))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_get_value(position, right);
		break;

	case TOKEN_AND:
		if(!parser_match(parser, TOKEN_AND))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_get_address(position, right);
		break;

	case TOKEN_DOT_DOT:
		if(!parser_match(parser, TOKEN_DOT_DOT))
		{
			return NULL;
		}
		right = parser_postfix(parser);
		if (!right)
		{
			return NULL;
		}
		node = node_make_ellipsis(position, right);
		break;

	case TOKEN_SIZEOF_KEYWORD:
		if(!parser_match(parser, TOKEN_SIZEOF_KEYWORD))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if(!right)
		{
			return NULL;
		}

		node = node_make_sizeof(position, right);
		break;

	case TOKEN_TYPEOF_KEYWORD:
		if(!parser_match(parser, TOKEN_TYPEOF_KEYWORD))
		{
			return NULL;
		}
		right = parser_prefix(parser);
		if(!right)
		{
			return NULL;
		}

		node = node_make_typeof(position, right);
		break;

	default:
		node = parser_postfix(parser);
		break;
	}

	return node;
}

static node_t *
parser_multiplicative(parser_t *parser)
{	
	node_t *node;
	node = parser_prefix(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_STAR))
			{
				return NULL;
			}
			
			right = parser_prefix(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_mul(position, node, right);
			break;

		case TOKEN_SLASH:
			if(!parser_match(parser, TOKEN_SLASH))
			{
				return NULL;
			}

			right = parser_prefix(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_div(position, node, right);
			break;

		case TOKEN_PERCENT:
			if(!parser_match(parser, TOKEN_PERCENT))
			{
				return NULL;
			}
			
			right = parser_prefix(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_mod(position, node, right);
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static node_t *
parser_addative(parser_t *parser)
{	
	node_t *node;
	node = parser_multiplicative(parser);
	if(!node){
		return NULL;
	}
	
	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_PLUS:
			if(!parser_match(parser, TOKEN_PLUS))
			{
				return NULL;
			}

			right = parser_multiplicative(parser);
			if (!right)
			{
				return NULL;
			}

			node = node_make_plus(position, node, right);
			break;

		case TOKEN_MINUS:
			if(!parser_match(parser, TOKEN_MINUS))
			{
				return NULL;
			}
			
			right = parser_multiplicative(parser);
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
parser_shifting(parser_t *parser)
{	
	node_t *node;
	node = parser_addative(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_LT_LT))
			{
				return NULL;
			}
			
			right = parser_addative(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_shl(position, node, right);
			break;

		case TOKEN_GT_GT:
			if(!parser_match(parser, TOKEN_GT_GT))
			{
				return NULL;
			}
			
			right = parser_addative(parser);
			if (!right) {
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
parser_relational(parser_t *parser)
{	
	node_t *node;
	node = parser_shifting(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_LT))
			{
				return NULL;
			}
			right = parser_shifting(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_lt(position, node, right);
			break;

		case TOKEN_LT_EQ:
			if(!parser_match(parser, TOKEN_LT_EQ))
			{
				return NULL;
			}
			
			right = parser_shifting(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_le(position, node, right);
			break;

		case TOKEN_GT:
			if(!parser_match(parser, TOKEN_GT))
			{
				return NULL;
			}
			right = parser_shifting(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_gt(position, node, right);
			break;

		case TOKEN_GT_EQ:
			if(!parser_match(parser, TOKEN_GT_EQ))
			{
				return NULL;
			}
			
			right = parser_shifting(parser);
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
parser_equality(parser_t *parser)
{
	node_t *node;
	node = parser_relational(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_EQ_EQ))
			{
				return NULL;
			}
			
			right = parser_relational(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_eq(position, node, right);
			break;

		case TOKEN_NOT_EQ:
			if(!parser_match(parser, TOKEN_NOT_EQ))
			{
				return NULL;
			}
			
			right = parser_relational(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_neq(position, node, right);
			break;

		case TOKEN_IN_KEYWORD:
			if(!parser_match(parser, TOKEN_IN_KEYWORD))
			{
				return NULL;
			}
			right = parser_relational(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_in(position, node, right);
			break;

		case TOKEN_EXTENDS_KEYWORD:
			if(!parser_match(parser, TOKEN_EXTENDS_KEYWORD))
			{
				return NULL;
			}
			right = parser_relational(parser);
			if (!right)
			{
				return NULL;
			}
			
			node = node_make_extends(position, node, right);
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static node_t *
parser_bitwise_and(parser_t *parser)
{
	node_t *node;
	node = parser_equality(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_AND))
			{
				return NULL;
			}
			
			right = parser_equality(parser);
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
parser_bitwise_xor(parser_t *parser)
{
	node_t *node;
	node = parser_bitwise_and(parser);
	if(!node){
		return NULL;
	}

	node_t *right;
	while (node)
	{
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_CARET:
			if(!parser_match(parser, TOKEN_CARET))
			{
				return NULL;
			}
			
			right = parser_bitwise_and(parser);
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
parser_bitwise_or(parser_t *parser)
{
	node_t *node;
	node = parser_bitwise_xor(parser);
	if(!node){
		return NULL;
	}
	
	node_t *right;
	while (node) {
		position_t position = parser->token->position;
		switch (parser->token->type)
		{
		case TOKEN_OR:
			if(!parser_match(parser, TOKEN_OR))
			{
				return NULL;
			}
			
			right = parser_bitwise_xor(parser);
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
parser_logical_and(parser_t *parser)
{
	node_t *node;
	node = parser_bitwise_or(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_AND_AND))
			{
				return NULL;
			}
			
			right = parser_bitwise_or(parser);
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
parser_logical_or(parser_t *parser)
{
	node_t *node;
	node = parser_logical_and(parser);
	if(!node)
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
			if(!parser_match(parser, TOKEN_OR_OR))
			{
				return NULL;
			}
			
			right = parser_logical_and(parser);
			if (!right) {
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
parser_conditional(parser_t *parser)
{
	position_t position = parser->token->position;
	
	node_t *node;
	node = parser_logical_or(parser);
	if(!node)
	{
		return NULL;
	}
	
	node_t *true_expression;
	node_t *false_expression;
	switch (parser->token->type) {
	case TOKEN_QUESTION:
		if(!parser_match(parser, TOKEN_QUESTION))
		{
			return NULL;
		}
		
		true_expression = NULL;
		if (parser->token->type != TOKEN_COLON)
		{
			true_expression = parser_conditional(parser);
			if (!true_expression)
			{
				break;
			}
		}
		if(!parser_match(parser, TOKEN_COLON))
		{
			return NULL;
		}
		
		false_expression = parser_conditional(parser);
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
parser_expression(parser_t *parser)
{
	return parser_conditional(parser);
}



static node_t *
parser_assign_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	node_t *node;
	node = parser_expression(parser);
	if(!node)
	{
		return NULL;
	}
	
	node_t *right;
	switch (parser->token->type)
	{
	case TOKEN_EQ:
		if(!parser_match(parser, TOKEN_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}

		node = node_make_assign(position, node, right);
		break;

	case TOKEN_COLON_EQ:
		if(!parser_match(parser, TOKEN_COLON_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_define(position, node, right);
		break;

	case TOKEN_PLUS_EQ:
		if(!parser_match(parser, TOKEN_PLUS_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_add_assign(position, node, right);
		break;

	case TOKEN_MINUS_EQ:
		if(!parser_match(parser, TOKEN_MINUS_EQ))
		{
			return NULL;
		}	
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_sub_assign(position, node, right);
		break;

	case TOKEN_STAR_EQ:
		if(!parser_match(parser, TOKEN_STAR_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_mul_assign(position, node, right);
		break;

	case TOKEN_SLASH_EQ:
		if(!parser_match(parser, TOKEN_SLASH_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_div_assign(position, node, right);
		break;

	case TOKEN_PERCENT_EQ:
		if(!parser_match(parser, TOKEN_PERCENT_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_mod_assign(position, node, right);
		break;

	case TOKEN_AND_EQ:
		if(!parser_match(parser, TOKEN_AND_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_and_assign(position, node, right);
		break;

	case TOKEN_OR_EQ:
		if(!parser_match(parser, TOKEN_OR_EQ))
		{
			return NULL;
		}	
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_or_assign(position, node, right);
		break;
		
	case TOKEN_LT_LT_EQ:
		if(!parser_match(parser, TOKEN_LT_LT_EQ)){
			return NULL;
		}
		right = parser_expression(parser);
		if (!right)
		{
			return NULL;
		}
		
		node = node_make_shl_assign(position, node, right);
		break;

	case TOKEN_GT_GT_EQ:
		if(!parser_match(parser, TOKEN_GT_GT_EQ))
		{
			return NULL;
		}
		right = parser_expression(parser);
		if (!right) {
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
parser_if_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_IF_KEYWORD))
	{
		return NULL;
	}

	if(!parser_match(parser, TOKEN_LPAREN))
	{
		return NULL;
	}
	
	
	node_t *expr;
	expr = parser_expression(parser);
	if (!expr)
	{
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_RPAREN))
	{
		return NULL;
	}

	node_t *then_block_stmt;
	then_block_stmt = parser_block_stmt(parser);
	if (!then_block_stmt)
	{
		return NULL;
	}

	node_t *else_block_stmt;
	else_block_stmt = NULL;
	if (parser->token->type == TOKEN_ELSE_KEYWORD)
	{
		if(!parser_match(parser, TOKEN_ELSE_KEYWORD))
		{
			return NULL;
		}
	
		if (parser->token->type == TOKEN_IF_KEYWORD)
		{
			else_block_stmt = parser_if_stmt(parser);
		}
		else
		{
			else_block_stmt = parser_block_stmt(parser);
		}

		if (!else_block_stmt)
		{
			return NULL;
		}
	}

	return node_make_if(position, expr, then_block_stmt, else_block_stmt);
}

static node_t *
parser_for_stmt(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_FOR_KEYWORD))
	{
		return NULL;
	}

	int32_t use_brace = 0;
	if(parser->token->type == TOKEN_LBRACE)
	{
		if(!parser_match(parser, TOKEN_LBRACE))
		{
			return NULL;
		}
		use_brace = 1;
	}
	else
	{
		if(!parser_match(parser, TOKEN_LPAREN))
		{
			return NULL;
		}
	}

	if((!use_brace && (parser->token->type == TOKEN_RPAREN)) || use_brace)
	{
		if(!use_brace)
		{
			if(!parser_match(parser, TOKEN_RPAREN))
			{
				return NULL;
			}
		}

		parser->loop_depth += 1;

		node_t *body;
		body = parser_block_stmt(parser);
		if (!body)
		{
			return NULL;
		}

		parser->loop_depth -= 1;

		return node_make_for(position, NULL, NULL, NULL, body);
	}

	node_t *init_expr = NULL;
	int32_t is_trinary = 0;
	int32_t use_init = 0;
	int32_t use_const = 0;
	switch (parser->token->type)
	{
	case TOKEN_CONST_KEYWORD:
	case TOKEN_VAR_KEYWORD:
		if(parser->token->type == TOKEN_CONST_KEYWORD)
		{
			if(!parser_match(parser, TOKEN_CONST_KEYWORD))
			{
				return NULL;
			}
			use_const = 1;
		} 
		else 
		{
			if(!parser_match(parser, TOKEN_VAR_KEYWORD))
			{
				return NULL;
			}
		}
		int32_t is_object = 0;
		int32_t is_array = 0;
		int32_t use_type = 0;
		node_t *name;
		switch(parser->token->type){
		case TOKEN_LBRACE:
			name = parser_object(parser);
			is_object = 1;
			break;

		case TOKEN_LBRACKET:
			name = parser_array(parser);
			is_array = 1;
			break;

		default:
			name = parser_id(parser);
			break;
		}
		if(!name)
		{
			return NULL;
		}
		node_t *type = NULL;
		if(!is_array && !is_object)
		{
			if (parser->token->type == TOKEN_COLON)
			{
				if(!parser_match(parser, TOKEN_COLON))
				{
					return NULL;
				}
				type = parser_expression(parser);
				if (!type)
				{
					return NULL;
				}
				is_trinary = 1;
				use_type = 1;
			}
		}
		
		node_t *expr = NULL;
		if (parser->token->type == TOKEN_EQ || is_object || use_type)
		{
			if(!parser_match(parser, TOKEN_EQ))
			{
				return NULL;
			}

			expr = parser_expression(parser);
			if (!expr)
			{
				return NULL;
			}
			is_trinary = 1;

			if(use_const)
			{
				init_expr = node_make_const(position, name, type, expr);
			}
			else
			{
				init_expr = node_make_var(position, name, type, expr);
			}
			
			if(!init_expr)
			{
				return NULL;
			}

			goto comma_init;
		}

		if(use_const)
		{
			init_expr = node_make_const(position, name, type, expr);
		}
		else
		{
			init_expr = node_make_var(position, name, type, expr);
		}
		
		if(!init_expr)
		{
			return NULL;
		}

		if(parser->token->type == TOKEN_IN_KEYWORD)
		{
			goto forin_loop;
		}
		break;

	default:
		use_init = 1;
		parser_save_state(parser);
		if(parser->token->type != TOKEN_SEMICOLON)
		{
			init_expr = parser_assign_stmt(parser);
			if(!init_expr)
			{
				return NULL;
			}
		}
		break;
	}

	comma_init:
	if(parser->token->type == TOKEN_COMMA)
	{
		is_trinary = 1;

		list_t *init_list;
		init_list = list_create();
		if(!init_list)
		{
			return NULL;
		}

		if(!list_rpush(init_list, (list_value_t)init_expr))
		{
			return NULL;
		}
		while(parser->token->type == TOKEN_COMMA)
		{
			if(!parser_match(parser, TOKEN_COMMA))
			{
				return NULL;
			}

			switch (parser->token->type)
			{
			case TOKEN_VAR_KEYWORD:
				if(!parser_match(parser, TOKEN_VAR_KEYWORD))
				{
					return NULL;
				}
				int32_t is_object = 0;
				int32_t is_array = 0;
				node_t *name;
				switch(parser->token->type)
				{
				case TOKEN_LBRACE:
					name = parser_object(parser);
					is_object = 1;
					break;

				case TOKEN_LBRACKET:
					name = parser_array(parser);
					is_array = 1;
					break;

				default:
					name = parser_id(parser);
					break;
				}

				if(!name)
				{
					return NULL;
				}

				node_t *type = NULL;
				if(!is_array && !is_object)
				{
					if (parser->token->type == TOKEN_COLON)
					{
						if(!parser_match(parser, TOKEN_COLON))
						{
							return NULL;
						}
						type = parser_expression(parser);
						if (!type)
						{
							return NULL;
						}
					}
				}
				
				node_t *expr = NULL;
				if (parser->token->type == TOKEN_EQ || is_object)
				{
					if(!parser_match(parser, TOKEN_EQ))
					{
						return NULL;
					}

					expr = parser_expression(parser);
					if (!expr)
					{
						return NULL;
					}
				}

				init_expr = node_make_var(position, name, type, expr);
				break;

			default:
				init_expr = parser_assign_stmt(parser);
				if(!init_expr)
				{
					return NULL;
				}
				break;
			}

			if(!list_rpush(init_list, (list_value_t)init_expr))
			{
				return NULL;
			}
		}

		init_expr = node_make_for_init_list(position, init_list);
	}

	if(parser->token->type == TOKEN_SEMICOLON || is_trinary)
	{
		if(!parser_match(parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		
		node_t *cond_expr = NULL;
		node_t *step_expr = NULL;
		if(parser->token->type != TOKEN_RPAREN)
		{
			if(parser->token->type != TOKEN_SEMICOLON)
			{
				cond_expr = parser_expression(parser);
				if (!cond_expr)
				{
					return NULL;
				}
			}

			if(parser->token->type != TOKEN_RPAREN)
			{
				if(!parser_match(parser, TOKEN_SEMICOLON))
				{
					return NULL;
				}

				if(parser->token->type != TOKEN_RPAREN)
				{
					step_expr = parser_assign_stmt(parser);
					if(!step_expr)
					{
						return NULL;
					}

					if(parser->token->type == TOKEN_COMMA)
					{
						list_t *step_list;
						step_list = list_create();
						if(!step_list)
						{
							return NULL;
						}

						if(!list_rpush(step_list, (list_value_t)step_expr))
						{
							return NULL;
						}

						while(parser->token->type == TOKEN_COMMA)
						{
							if(!parser_match(parser, TOKEN_COMMA))
							{
								return NULL;
							}

							step_expr = parser_assign_stmt(parser);
							if(!step_expr)
							{
								return NULL;
							}

							if(!list_rpush(step_list, (list_value_t)step_expr))
							{
								return NULL;
							}
						}

						step_expr = node_make_for_step_list(position, step_list);
					}
				}
			}
			
		}

		if(!parser_match(parser, TOKEN_RPAREN))
		{
			return NULL;
		}
		
		parser->loop_depth += 1;

		node_t *body;
		body = parser_block_stmt(parser);
		if (!body)
		{
			return NULL;
		}

		parser->loop_depth -= 1;

		if(use_init)
		{
			parser_release_state(parser);
		}

		return node_make_for(position, init_expr, cond_expr, step_expr, body);
	} 
	else 
	{
		node_t *cond_expr = NULL;
		if(use_init)
		{
			parser_restore_state(parser);
			cond_expr = parser_expression(parser);
			if(!cond_expr)
			{
				return NULL;
			}
		}
		else
		{
			parser_error(parser, position, "variable or constant as the expression used in for loop");
			return NULL;
		}
		
		if(!parser_match(parser, TOKEN_RPAREN))
		{
			return NULL;
		}
		
		parser->loop_depth += 1;

		node_t *body;
		body = parser_block_stmt(parser);
		if (!body)
		{
			return NULL;
		}

		parser->loop_depth -= 1;

		return node_make_for(position, NULL, cond_expr, NULL, body);
	}

	forin_loop:
	if(!parser_match(parser, TOKEN_IN_KEYWORD))
	{
		return NULL;
	}

	node_t *iter;
	iter = parser_expression(parser);
	if(!iter)
	{
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_RPAREN))
	{
		return NULL;
	}
	
	parser->loop_depth += 1;

	node_t *block_stmt;
	block_stmt = parser_block_stmt(parser);
	if (!block_stmt)
	{
		return NULL;
	}
	parser->loop_depth -= 1;

	if(use_init)
	{
		parser_release_state(parser);
	}

	return node_make_forin(position, init_expr, iter, block_stmt);
}

static node_t *
parser_break_stmt(parser_t *parser)
{
	position_t position = parser->token->position;

	if(parser->loop_depth <= 0){
		parser_error(parser, position, "'break' is not within a loop\n");
		return NULL;
	}
	if(!parser_match(parser, TOKEN_BREAK_KEYWORD)){
		return NULL;
	}
	if(!parser_match(parser, TOKEN_SEMICOLON)){
		return NULL;
	}

	return node_make_break(position);
}

static node_t *
parser_continue_stmt(parser_t *parser)
{
	position_t position = parser->token->position;

	if(parser->loop_depth <= 0){
		parser_error(parser, position, "'continue' is not within a loop\n");
		return NULL;
	}
	if(!parser_match(parser, TOKEN_CONTINUE_KEYWORD)){
		return NULL;
	}
	if(!parser_match(parser, TOKEN_SEMICOLON)){
		return NULL;
	}

	return node_make_continue(position);
}

static node_t *
parser_catch_stmt(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_CATCH_KEYWORD)){
		return NULL;
	}

	int32_t use_parenthesis = 0;
	if(parser->token->type == TOKEN_LPAREN){
		if(!parser_match(parser, TOKEN_LPAREN)){
			return NULL;
		}
		use_parenthesis = 1;
	}
	
	node_t *parameter = NULL;
	if(parser->token->type != TOKEN_RPAREN){
		parameter = parser_parameter(parser);
		if (!parameter) {
			return NULL;
		}
	}
	
	if(use_parenthesis){
		if(!parser_match(parser, TOKEN_RPAREN)){
			return NULL;
		}
	}

	node_t *body;
	body = parser_block_stmt(parser);
	if (!body) {
		return NULL;
	}

	return node_make_catch(position, parameter, body);
}

static list_t *
parser_catchs(parser_t *parser)
{
	list_t *list;
	list = list_create();
	if(!list)
	{
		return NULL;
	}

	node_t *node;
	while(parser->token->type == TOKEN_CATCH_KEYWORD)
	{
		node = parser_catch_stmt(parser);
		if (!node) {
			return NULL;
		}

		if(!list_rpush(list, (list_value_t)node))
		{
			return NULL;
		}
	}

	return list;
}

static node_t *
parser_try_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_TRY_KEYWORD)){
		return NULL;
	}

	node_t *body;
	body = parser_block_stmt(parser);
	if (!body) {
		return NULL;
	}

	list_t *catchs = NULL;
	if (parser->token->type == TOKEN_CATCH_KEYWORD) {
		catchs = parser_catchs(parser);
		if (!catchs) {
			return NULL;
		}
	}

	return node_make_try(position, body, catchs);
}

static node_t *
parser_return_stmt(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_RETURN_KEYWORD))
	{
		return NULL;
	}
	
	node_t *expr;
	expr = NULL;
	if (parser->token->type != TOKEN_SEMICOLON) 
	{
		expr = parser_expression(parser);
		if (!expr) {
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_return(position, expr);
}

static node_t *
parser_throw_stmt(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_THROW_KEYWORD))
	{
		return NULL;
	}
	
	node_t *expr = NULL;
	if (parser->token->type != TOKEN_SEMICOLON) 
	{
		expr = parser_expression(parser);
		if (!expr) {
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_throw(position, expr);
}

static node_t *
parser_var_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_VAR_KEYWORD))
	{
		return NULL;
	}

	int32_t is_object = 0;
	int32_t is_array = 0;
	node_t *name;
	switch(parser->token->type)
	{
	case TOKEN_LBRACE:
		name = parser_object(parser);
		is_object = 1;
		break;
	
	case TOKEN_LBRACKET:
		name = parser_array(parser);
		is_array = 1;
		break;

	default:
		name = parser_id(parser);
		break;
	}
	
	if(!name)
	{
		return NULL;
	}

	node_t *type = NULL;
	if(!is_object && !is_array)
	{
		if (parser->token->type == TOKEN_COLON)
		{
			if(!parser_match(parser, TOKEN_COLON))
			{
				return NULL;
			}
			type = parser_expression(parser);
			if (!type)
			{
				return NULL;
			}
		}
	}
	
	node_t *expr = NULL;
	if (parser->token->type == TOKEN_EQ || is_object || is_array || (!is_object && !is_array && !type))
	{
		if(!parser_match(parser, TOKEN_EQ))
		{
			return NULL;
		}

		expr = parser_expression(parser);
		if (!expr)
		{
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_var(position, name, type, expr);
}

static node_t *
parser_const_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_CONST_KEYWORD))
	{
		return NULL;
	}

	int32_t is_object = 0;
	int32_t is_array = 0;
	node_t *name;
	switch(parser->token->type)
	{
	case TOKEN_LBRACE:
		name = parser_object(parser);
		is_object = 1;
		break;
	
	case TOKEN_LBRACKET:
		name = parser_array(parser);
		is_array = 1;
		break;

	default:
		name = parser_id(parser);
		break;
	}
	
	if(!name)
	{
		return NULL;
	}

	node_t *type = NULL;
	if(!is_object && !is_array)
	{
		if (parser->token->type == TOKEN_COLON)
		{
			if(!parser_match(parser, TOKEN_COLON))
			{
				return NULL;
			}
			type = parser_expression(parser);
			if (!type)
			{
				return NULL;
			}
		}
	}
	
	if(!parser_match(parser, TOKEN_EQ))
	{
		return NULL;
	}

	node_t *expr;
	expr = parser_expression(parser);
	if (!expr)
	{
		return NULL;
	}

	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_const(position, name, type, expr);
}

static node_t *
parser_type_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_TYPE_KEYWORD))
	{
		return NULL;
	}

	node_t *name;
	name = parser_id(parser);
	if(!name)
	{
		return NULL;
	}

	list_t *type_parameters = NULL;
	if(parser->token->type == TOKEN_LBRACE)
	{
		if(!parser_match(parser, TOKEN_LBRACE))
		{
			return NULL;
		}

		if(parser->token->type == TOKEN_RBRACE)
		{
			parser_error(parser, parser->token->position, "type parameters is empty");
			return NULL;
		}

		type_parameters = parser_type_parameters(parser);
		if(!type_parameters)
		{
			return NULL;
		}

		if(!parser_match(parser, TOKEN_RBRACE))
		{
			return NULL;
		}
	}

	list_t *heritages = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if(!parser_match(parser, TOKEN_EXTENDS_KEYWORD))
		{
			return NULL;
		}

		heritages = parser_heritages(parser);
		if (!heritages)
		{
			return NULL;
		}	
	}

	if(!parser_match(parser, TOKEN_EQ))
	{
		return NULL;
	}

	node_t *type;
	type = parser_expression(parser);
	if (!type)
	{
		return NULL;
	}

	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_type(position, name, type_parameters, heritages, type);
}

static node_t *
parser_block_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}

	list_t *stmt_list;
	stmt_list = list_create();
	if(!stmt_list){
		return NULL;
	}
	
	while (parser->token->type != TOKEN_RBRACE) {
		/* empty stmt */
		if (parser->token->type == TOKEN_SEMICOLON) {
			if(!parser_match(parser, TOKEN_SEMICOLON)){
				return NULL;
			}

			continue;
		}

		node_t *stmt;
		stmt = parser_statement(parser);
		if (!stmt) {
			return NULL;
		}
		
		if(!list_rpush(stmt_list, (list_value_t)stmt)){
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}

	return node_make_block(position, stmt_list);
}

static node_t *
parser_parameter(parser_t *parser)
{
	position_t position = parser->token->position;
	
	int32_t use_ellipsis = 0;
	if(parser->token->type == TOKEN_DOT_DOT)
	{
		if(!parser_match(parser, TOKEN_DOT_DOT))
		{
			return NULL;
		}
		use_ellipsis = 1;
	}

	node_t *name;
	if(parser->token->type == TOKEN_LBRACE && !use_ellipsis)
	{
		name = parser_object(parser);
	}
	else if(parser->token->type == TOKEN_LBRACKET)
	{
		name = parser_array(parser);
	}
	else
	{
		name = parser_id(parser);
	}
	
	if(!name)
	{
		return NULL;
	}

	if(use_ellipsis)
	{
		name = node_make_ellipsis(position, name);
	}
	if(!name)
	{
		return NULL;
	}
	
	node_t *type = NULL;
	if(!parser_match(parser, TOKEN_COLON))
	{
		return NULL;
	}
	type = parser_expression(parser);
	if (!type)
	{
		return NULL;
	}
	
	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ) {
		if(!parser_match(parser, TOKEN_EQ)){
			return NULL;
		}
		value = parser_expression(parser);
		if (!value) {
			return NULL;
		}
	}

	return node_make_parameter(position, name, type, value);
}

static list_t *
parser_parameters(parser_t *parser)
{
	list_t *parameters;
	parameters = list_create();
	if(!parameters){
		return NULL;
	}

	node_t *node;
	while (true) {
		node = parser_parameter(parser);
		if (!node) {
			return NULL;
		}

		if(!list_rpush(parameters, (list_value_t)node)){
			return NULL;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}

	return parameters;
}

static node_t *
parser_type_parameter(parser_t *parser)
{
	position_t position = parser->token->position;
	
	node_t *name;
	name = parser_id(parser);
	if(!name)
	{
		return NULL;
	}

	node_t *extends = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if(!parser_match(parser, TOKEN_EXTENDS_KEYWORD))
		{
			return NULL;
		}
		extends = parser_expression(parser);
		if (!extends)
		{
			return NULL;
		}
	}
	
	node_t *value = NULL;
	if (parser->token->type == TOKEN_EQ)
	{
		if(!parser_match(parser, TOKEN_EQ))
		{
			return NULL;
		}
		value = parser_expression(parser);
		if (!value)
		{
			return NULL;
		}
	}

	return node_make_type_parameter(position, name, extends, value);
}

static list_t *
parser_type_parameters(parser_t *parser)
{
	list_t *type_parameters;
	type_parameters = list_create();
	if(!type_parameters){
		return NULL;
	}

	node_t *node;
	while (true) {
		node = parser_type_parameter(parser);
		if (!node) {
			return NULL;
		}

		if(!list_rpush(type_parameters, (list_value_t)node)){
			return NULL;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}

	return type_parameters;
}

static node_t *
parser_field(parser_t *parser)
{
	position_t position = parser->token->position;
	
	node_t *name;
	name = parser_id(parser);
	if(!name){
		return NULL;
	}
	
	node_t *type = NULL;
	if(!parser_match(parser, TOKEN_COLON)){
		return NULL;
	}
	type = parser_expression(parser);
	if (!type) {
		return NULL;
	}
	
	return node_make_field(position, name, type);
}

static list_t *
parser_fields(parser_t *parser)
{
	list_t *fields;
	fields = list_create();
	if(!fields){
		return NULL;
	}

	node_t *node;
	while (true) {
		node = parser_field(parser);
		if (!node) {
			return NULL;
		}

		if(!list_rpush(fields, (list_value_t)node)){
			return NULL;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}

	return fields;
}

static node_t *
parser_func_stmt(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_FUNC_KEYWORD))
	{
		return NULL;
	}

	node_t *field = NULL;
	if(parser->token->type == TOKEN_LPAREN)
	{
		if(!parser_match(parser, TOKEN_LPAREN))
		{
			return NULL;
		}		
		if (parser->token->type == TOKEN_RPAREN)
		{
			parser_error(parser, parser->token->position, "fields is empty");
			return NULL;
		}

		field = parser_field(parser);
		if (!field)
		{
			return NULL;
		}

		if(!parser_match(parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	node_t *name;
	name = parser_id(parser);
	if (!name)
	{
		return NULL;
	}
	
	list_t *type_parameters = NULL;
	if (parser->token->type == TOKEN_LBRACE)
	{
		if(!parser_match(parser, TOKEN_LBRACE))
		{
			return NULL;
		}
		
		if(parser->token->type == TOKEN_RBRACE)
		{
			parser_error(parser, parser->token->position, "type parameters is empty");
			return NULL;
		}

		type_parameters = parser_type_parameters(parser);
		if (!type_parameters)
		{
			return NULL;
		}
		
		if(!parser_match(parser, TOKEN_RBRACE))
		{
			return NULL;
		}
	}

	list_t *parameters = NULL;
	if(!parser_match(parser, TOKEN_LPAREN))
	{
		return NULL;
	}
	if (parser->token->type != TOKEN_RPAREN)
	{
		parameters = parser_parameters(parser);
		if (!parameters)
		{
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_RPAREN))
	{
		return NULL;
	}
	
	node_t *return_type;
	return_type = NULL;
	if(parser->token->type == TOKEN_COLON)
	{
		if(!parser_match(parser, TOKEN_COLON))
		{
			return NULL;
		}
		return_type = parser_expression(parser);
		if(!return_type)
		{
			return NULL;
		}
	}
	
	if(!parser_match(parser, TOKEN_MINUS_GT))
	{
		return NULL;
	}

	node_t *body = NULL;
	body = parser_block_stmt(parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_func(position, field, name, type_parameters, parameters, return_type, body);
}

static node_t *
parser_statement(parser_t *parser)
{
	node_t *node = NULL;
	switch (parser->token->type) 
	{
	case TOKEN_SEMICOLON:
		if(!parser_match(parser, TOKEN_SEMICOLON))
		{
			return NULL;
		}
		node = parser_statement(parser);
		break;

	case TOKEN_LBRACE:
		node = parser_block_stmt(parser);
		break;

	case TOKEN_IF_KEYWORD:
		node = parser_if_stmt(parser);
		break;

	case TOKEN_TRY_KEYWORD:
		node = parser_try_stmt(parser);
		break;

	case TOKEN_FOR_KEYWORD:
		node = parser_for_stmt(parser);
		break;

	case TOKEN_VAR_KEYWORD:
		node = parser_var_stmt(parser);
		break;

	case TOKEN_CONST_KEYWORD:
		node = parser_const_stmt(parser);
		break;

	case TOKEN_TYPE_KEYWORD:
		node = parser_type_stmt(parser);
		break;

	case TOKEN_BREAK_KEYWORD:
		node = parser_break_stmt(parser);
		break;

	case TOKEN_CONTINUE_KEYWORD:
		node = parser_continue_stmt(parser);
		break;

	case TOKEN_RETURN_KEYWORD:
		node = parser_return_stmt(parser);
		break;

	case TOKEN_THROW_KEYWORD:
		node = parser_throw_stmt(parser);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_func_stmt(parser);
		break;

	default:
		node = parser_assign_stmt(parser);
		if (!node) {
			break;
		}

		if (node->kind != NODE_KIND_FUNC)
		{
			if(!parser_match(parser, TOKEN_SEMICOLON)){
				return NULL;
			}
		}
		break;
	}

	return node;
}




static node_t *
parser_enum_member(parser_t *parser)
{
	position_t position = parser->token->position;
	node_t *name;
	name = parser_id(parser);
	if (!name) {
		return NULL;
	}
	
	node_t *value = NULL;
	if(parser->token->type == TOKEN_EQ){
		if(!parser_match(parser, TOKEN_EQ)){
			return NULL;
		}

		value = parser_expression(parser);
		if (!value) {
			return NULL;
		}
	}
	
	return node_make_enum_member(position, name, value);
}

static list_t *
parser_enum_block(parser_t *parser)
{
	list_t *member_list;
	member_list = list_create();
	if(!member_list){
		return NULL;
	}
	
	if(parser->token->type != TOKEN_RBRACE){
		while(true){
			node_t *member;
			member = parser_enum_member(parser);
			if(!member){
				return NULL;
			}
			
			if(!list_rpush(member_list, (list_value_t)member)){
				return NULL;
			}
			
			if(parser->token->type != TOKEN_COMMA){
				break;
			}
			
			if(!parser_match(parser, TOKEN_COMMA)){
				return NULL;
			}
		}
	}
	
	return member_list;
}

static node_t *
parser_enum(parser_t *parser, uint64_t flag)
{	
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_ENUM_KEYWORD)){
		return NULL;
	}

	node_t *name;
	name = parser_id(parser);
	if (!name) {
		return NULL;
	}

		
	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}

	list_t *body;
	body = parser_enum_block(parser);
	if(!body){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}

	return node_make_enum(position, flag, name, body);
}

static node_t *
parser_class_func(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_FUNC_KEYWORD))
	{
		return NULL;
	}

	list_t *fields = NULL;
	if((flag & PARSER_MODIFIER_STATIC) != PARSER_MODIFIER_STATIC)
	{
		if(parser->token->type == TOKEN_LPAREN)
		{
			if(!parser_match(parser, TOKEN_LPAREN))
			{
				return NULL;
			}		
			if (parser->token->type != TOKEN_RPAREN)
			{
				fields = parser_fields(parser);
				if (!fields)
				{
					return NULL;
				}
			}
			if(!parser_match(parser, TOKEN_RPAREN))
			{
				return NULL;
			}
		}
	}

	node_t *name;
	name = parser_id(parser);
	if (!name)
	{
		return NULL;
	}
	
	list_t *type_parameters = NULL;
	if (parser->token->type == TOKEN_LBRACE)
	{
		if(!parser_match(parser, TOKEN_LBRACE))
		{
			return NULL;
		}

		if(parser->token->type == TOKEN_RBRACE)
		{
			parser_error(parser, parser->token->position, "type parameters is empty");
			return NULL;
		}
		type_parameters = parser_type_parameters(parser);
		if (!type_parameters)
		{
			return NULL;
		}
		
		if(!parser_match(parser, TOKEN_RBRACE))
		{
			return NULL;
		}
	}
	
	list_t *parameters = NULL;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if(!parser_match(parser, TOKEN_LPAREN))
		{
			return NULL;
		}
		if (parser->token->type != TOKEN_RPAREN)
		{
			parameters = parser_parameters(parser);
			if (!parameters)
			{
				return NULL;
			}
		}
		if(!parser_match(parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}
	
	node_t *return_type = NULL;
	if(parser->token->type == TOKEN_COLON)
	{
		if(!parser_match(parser, TOKEN_COLON))
		{
			return NULL;
		}
		return_type = parser_expression(parser);
		if(!return_type)
		{
			return NULL;
		}
	}
	
	if(!parser_match(parser, TOKEN_MINUS_GT))
	{
		return NULL;
	}

	node_t *body = NULL;
	body = parser_block_stmt(parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_method(position, flag, fields, name, type_parameters, parameters, return_type, body);
}

static node_t *
parser_class_property(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	node_t *name = NULL;
	name = parser_id(parser);
	if (!name)
	{
		return NULL;
	}

	node_t *type = NULL;
	if (parser->token->type == TOKEN_COLON)
	{
		if(!parser_match(parser, TOKEN_COLON))
		{
			return NULL;
		}
		type = parser_expression(parser);
		if (!type)
		{
			return NULL;
		}
	}

	node_t *value = NULL;
	if((flag & PARSER_MODIFIER_STATIC) == PARSER_MODIFIER_STATIC)
	{
		if(!parser_match(parser, TOKEN_EQ))
		{
			return NULL;
		}
		value = parser_expression(parser);
		if (!value)
		{
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_property(position, flag, name, type, value);
}

static node_t *
parser_class_readonly(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_READONLY_KEYWORD)){
		return NULL;
	}

	flag |= PARSER_MODIFIER_READONLY;

	node_t *node = NULL;
	switch(parser->token->type){
	case TOKEN_ID:
		node = parser_class_property(parser, flag);
		break;

	default:
		parser_error(parser, position, "incorrect use of modifier 'readonly'");
		break;
	}

	flag &= ~PARSER_MODIFIER_READONLY;

	if(!node){
		return NULL;
	}

	return node;
}

static node_t *
parser_class_static(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_STATIC_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_STATIC;

	node_t *node = NULL;
	switch(parser->token->type)
	{
	case TOKEN_ID:
		node = parser_class_property(parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(parser, flag);
		break;

	default:
		parser_error(parser, position, "incorrect use of modifier 'static'");
		break;
	}

	flag &= ~PARSER_MODIFIER_STATIC;

	if(!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_public(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_PUBLIC_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_PUBLIC;

	node_t *node = NULL;
	switch(parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(parser, flag);
		break;

	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(parser, flag);
		break;
	
	case TOKEN_ID:
		node = parser_class_property(parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(parser, flag);
		break;

	default:
		parser_error(parser, position, "incorrect use of modifier 'public'");
		break;
	}

	flag &= ~PARSER_MODIFIER_PUBLIC;

	if(!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_private(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_PRIVATE_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_PRIVATE;

	node_t *node = NULL;
	switch(parser->token->type)
	{
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(parser, flag);
		break;

	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(parser, flag);
		break;
	
	case TOKEN_ID:
		node = parser_class_property(parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(parser, flag);
		break;

	default:
		parser_error(parser, position, "incorrect use of modifier 'private'");
		break;
	}

	flag &= ~PARSER_MODIFIER_PRIVATE;

	if(!node)
	{
		return NULL;
	}

	return node;
}

static node_t *
parser_class_protected(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_PROTECTED_KEYWORD))
	{
		return NULL;
	}

	flag |= PARSER_MODIFIER_PROTECTED;

	node_t *node = NULL;
	switch(parser->token->type){
	case TOKEN_READONLY_KEYWORD:
		node = parser_class_readonly(parser, flag);
		break;

	case TOKEN_STATIC_KEYWORD:
		node = parser_class_static(parser, flag);
		break;
	
	case TOKEN_ID:
		node = parser_class_property(parser, flag);
		break;

	case TOKEN_FUNC_KEYWORD:
		node = parser_class_func(parser, flag);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(parser, flag);
		break;

	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(parser, flag);
		break;

	default:
		parser_error(parser, position, "incorrect use of modifier 'protected'");
		break;
	}

	flag &= ~PARSER_MODIFIER_PROTECTED;

	if(!node)
	{
		return NULL;
	}

	return node;
}

static list_t *
parser_class_block(parser_t *parser)
{
	position_t position = parser->token->position;

	list_t *list;
	list = list_create();
	if(!list){
		return NULL;
	}

	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}

	while (parser->token->type != TOKEN_RBRACE) {
		node_t *decl = NULL;
		switch(parser->token->type){
		case TOKEN_READONLY_KEYWORD:
			decl = parser_class_readonly(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_STATIC_KEYWORD:
			decl = parser_class_static(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_PUBLIC_KEYWORD:
			decl = parser_class_public(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_PRIVATE_KEYWORD:
			decl = parser_class_private(parser, PARSER_MODIFIER_NONE);
			break;
		
		case TOKEN_PROTECTED_KEYWORD:
			decl = parser_class_protected(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_ID:
			decl = parser_class_property(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_FUNC_KEYWORD:
			decl = parser_class_func(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_CLASS_KEYWORD:
			decl = parser_class(parser, PARSER_MODIFIER_NONE);
			break;

		case TOKEN_ENUM_KEYWORD:
			decl = parser_enum(parser, PARSER_MODIFIER_NONE);
			break;

		default:
			parser_error(parser, position, "illegal declaration type");
			break;
		}

		if (!decl) {
			return NULL;
		}

		if(!(list_rpush(list, (list_value_t)decl))){
			return NULL;
		}
	}
	
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}

	return list;
}

static list_t *
parser_heritages(parser_t *parser)
{
	position_t position = parser->token->position;

	int32_t use_parenthesis = 0;
	if (parser->token->type == TOKEN_LPAREN)
	{
		if(!parser_match(parser, TOKEN_LPAREN))
		{
			return NULL;
		}
		use_parenthesis = 1;
	}

	list_t *heritage_list;
	heritage_list = list_create();
	if(!heritage_list)
	{
		return NULL;
	}
	
	node_t *node;
	while (true)
	{
		node = parser_postfix(parser);
		if (!node)
		{
			return NULL;
		}

		if(!list_rpush(heritage_list, (list_value_t)node))
		{
			return NULL;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if(!use_parenthesis)
		{
			parser_error(parser, position, "Use parentheses to define multiple inheritance");
			return NULL;
		}

		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	
	if(use_parenthesis)
	{
		if(!parser_match(parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	return heritage_list;
}

static node_t *
parser_class(parser_t *parser, uint64_t flag)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_CLASS_KEYWORD))
	{
		return NULL;
	}
	
	node_t *name;
	name = parser_id(parser);
	if (!name)
	{
		return NULL;
	}
	
	list_t *type_parameters = NULL;
	if (parser->token->type == TOKEN_LBRACE) 
	{
		if(!parser_match(parser, TOKEN_LBRACE))
		{
			return NULL;
		}

		if(parser->token->type == TOKEN_RBRACE)
		{
			parser_error(parser, parser->token->position, "type parameters is empty");
			return NULL;
		}

		type_parameters = parser_type_parameters(parser);
		if (!type_parameters)
		{
			return NULL;
		}

		if(!parser_match(parser, TOKEN_RBRACE))
		{
			return NULL;
		}
	}

	list_t *heritages = NULL;
	if (parser->token->type == TOKEN_EXTENDS_KEYWORD)
	{
		if(!parser_match(parser, TOKEN_EXTENDS_KEYWORD))
		{
			return NULL;
		}
		heritages = parser_heritages(parser);
		if (!heritages)
		{
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_MINUS_GT))
	{
		return NULL;
	}

	list_t *body;
	body = parser_class_block(parser);
	if (!body)
	{
		return NULL;
	}

	return node_make_class(position, flag, name, type_parameters, heritages, body);
}

static list_t *
parser_import_paths(parser_t *parser)
{
	list_t *list;
	list = list_create();
	if(!list)
	{
		return NULL;
	}

	node_t *node;
	while(true)
	{
		node = parser_string(parser);
		if(!node)
		{
			return NULL;
		}

		if(!list_rpush(list, (list_value_t)node))
		{
			return NULL;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA))
		{
			return NULL;
		}
	}

	return list;
}

static node_t *
parser_import(parser_t *parser)
{
	position_t position = parser->token->position;
	
	if(!parser_match(parser, TOKEN_IMPORT_KEYWORD))
	{
		return NULL;
	}
	
	node_t *field;
	switch(parser->token->type)
	{
	case TOKEN_LBRACE:	
		field = parser_object(parser);
		if(!field)
		{
			return NULL;
		}
		break;
	
	default:
		field = parser_id(parser);
		if(!field)
		{
			return NULL;
		}
		break;
	}
	
	if(!parser_match(parser, TOKEN_FROM_KEYWORD))
	{
		return NULL;
	}
	
	int32_t use_parenthesis = 0;
	if(parser->token->type == TOKEN_LPAREN)
	{
		if(!parser_match(parser, TOKEN_LPAREN))
		{
			return NULL;
		}
		use_parenthesis = 1;
	}

	list_t *paths;
	paths = parser_import_paths(parser);
	if (!paths)
	{
		return NULL;
	}

	if(use_parenthesis)
	{
		if(!parser_match(parser, TOKEN_RPAREN))
		{
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_SEMICOLON))
	{
		return NULL;
	}

	return node_make_import(position, field, paths);
}

static list_t *
parser_namespace_block(parser_t *parser)
{
	position_t position = parser->token->position;

	list_t *list;
	list = list_create();
	if(!list)
	{
		return NULL;
	}

	if(!parser_match(parser, TOKEN_LBRACE))
	{
		return NULL;
	}

	node_t *decl;
	while (parser->token->type != TOKEN_RBRACE)
	{
		if (parser->token->type == TOKEN_SEMICOLON)
		{
			if(!parser_match(parser, TOKEN_SEMICOLON))
			{
				return NULL;
			}
			continue;
		}

		switch(parser->token->type)
		{
		case TOKEN_EXPORT_KEYWORD:
			decl = parser_export(parser);
			break;

		case TOKEN_NAMESPACE_KEYWORD:
			decl = parser_namespace(parser);
			break;
			
		case TOKEN_CLASS_KEYWORD:
			decl = parser_class(parser, PARSER_MODIFIER_NONE);
			break;
			
		case TOKEN_ENUM_KEYWORD:
			decl = parser_enum(parser, PARSER_MODIFIER_NONE);
			break;
			
		case TOKEN_FUNC_KEYWORD:
			decl = parser_func_stmt(parser);
			break;
			
		case TOKEN_VAR_KEYWORD:
			decl = parser_var_stmt(parser);
			break;

		case TOKEN_CONST_KEYWORD:
			decl = parser_const_stmt(parser);
			break;

		case TOKEN_TYPE_KEYWORD:
			decl = parser_type_stmt(parser);
			break;

		default:
			parser_error(parser, position, "non-declaration statement outside function body");
			return NULL;
		}

		if (!decl)
		{
			return NULL;
		}

		if(!list_rpush(list, (list_value_t)decl))
		{
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_RBRACE))
	{
		return NULL;
	}

	return list;
}

static node_t *
parser_namespace(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_NAMESPACE_KEYWORD))
	{
		return NULL;
	}

	node_t *name;
	name = parser_id(parser);
	if(!name)
	{
		return NULL;
	}

	list_t *body;
	body = parser_namespace_block(parser);
	if(!body)
	{
		return NULL;
	}

	return node_make_namespace(position, name, body);
}

static node_t *
parser_export(parser_t *parser)
{
	position_t position = parser->token->position;

	if(!parser_match(parser, TOKEN_EXPORT_KEYWORD))
	{
		return NULL;
	}

	node_t *node = NULL;
	switch(parser->token->type)
	{
	case TOKEN_NAMESPACE_KEYWORD:
		node = parser_namespace(parser);
		break;

	case TOKEN_CLASS_KEYWORD:
		node = parser_class(parser, PARSER_MODIFIER_NONE);
		break;
		
	case TOKEN_ENUM_KEYWORD:
		node = parser_enum(parser, PARSER_MODIFIER_NONE);
		break;
		
	case TOKEN_FUNC_KEYWORD:
		node = parser_func_stmt(parser);
		break;
		
	case TOKEN_VAR_KEYWORD:
		node = parser_var_stmt(parser);
		break;

	case TOKEN_CONST_KEYWORD:
		node = parser_const_stmt(parser);
		break;

	case TOKEN_TYPE_KEYWORD:
		node = parser_type_stmt(parser);
		break;

	case TOKEN_LBRACE:
		node = parser_object(parser);
		break;

	default:
		parser_error(parser, position, "illegal declaration");
		break;
	}
	if(!node)
	{
		return NULL;
	}

	return node_make_export(position, node);
}

node_t *
parser_module(parser_t *parser)
{
	position_t position = parser->token->position;
	
	list_t *members;
	members = list_create();
	if(!members)
	{
		return NULL;
	}
	
	node_t *decl;
	while (parser->token->type != TOKEN_EOF)
	{
		if (parser->token->type == TOKEN_SEMICOLON)
		{
			if(!parser_match(parser, TOKEN_SEMICOLON))
			{
				return NULL;
			}
			continue;
		}
		
		switch(parser->token->type)
		{
		case TOKEN_EXPORT_KEYWORD:
			decl = parser_export(parser);
			break;

		case TOKEN_NAMESPACE_KEYWORD:
			decl = parser_namespace(parser);
			break;
			
		case TOKEN_IMPORT_KEYWORD:
			decl = parser_import(parser);
			break;
			
		case TOKEN_CLASS_KEYWORD:
			decl = parser_class(parser, PARSER_MODIFIER_NONE);
			break;
			
		case TOKEN_ENUM_KEYWORD:
			decl = parser_enum(parser, PARSER_MODIFIER_NONE);
			break;
			
		case TOKEN_FUNC_KEYWORD:
			decl = parser_func_stmt(parser);
			break;
			
		case TOKEN_VAR_KEYWORD:
			decl = parser_var_stmt(parser);
			break;

		case TOKEN_CONST_KEYWORD:
			decl = parser_const_stmt(parser);
			break;

		case TOKEN_TYPE_KEYWORD:
			decl = parser_type_stmt(parser);
			break;

		default:
			parser_error(parser, position, "non-declaration statement outside function body");
			return NULL;
		}

		if (!decl) {
			return NULL;
		}

		if(!list_rpush(members, (list_value_t)decl)){
			return NULL;
		}
	}

	if(!parser_match(parser, TOKEN_EOF)){
		return NULL;
	}

	return node_make_module(position, parser->file_source->path, members);
}

parser_t *
parser_create(program_t *program, char *path, list_t *errors)
{
	parser_t *parser;
	parser = (parser_t *)malloc(sizeof(parser_t));
	if(!parser)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(parser_t));
		return NULL;
	}
	memset(parser, 0, sizeof(parser_t));

	file_source_t *file_source;
    file_source = file_create_source(path);
    if(!file_source)
    {
        return NULL;
    }

	scanner_t *scanner;
    scanner = scanner_create(program, file_source, errors);
    if(!scanner)
    {
    	return NULL;
    }

	parser->scanner = scanner;
	parser->errors = errors;
	parser->file_source = file_source;
	parser->loop_depth = 0;
	parser->token = &scanner->token;
	parser->program = scanner->program;
	
	parser->states = list_create();
	if(!parser->states)
	{
		return NULL;
	}

	return parser;
}
