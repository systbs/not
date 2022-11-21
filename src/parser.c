#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "types.h"
#include "token.h"
#include "lexer.h"
#include "list.h"
#include "node.h"
#include "parser.h"
#include "debug.h"

static node_t *
parser_expression(parser_t *parser);

static node_t *
parser_statement(parser_t *parser);

static node_t *
parser_block_stmt(parser_t *parser);

static node_t *
parser_class_stmt(parser_t *parser);

static node_t *
parser_fn_stmt(parser_t *parser);

static node_t *
parser_typing(parser_t *parser);

static int32_t
parser_typing_test(parser_t *parser);

static node_t *
parser_bitwise_or(parser_t *parser);

static node_t *
parser_logical_and(parser_t *parser);

static void
parser_error(parser_t *parser, const char *format, ...)
{
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	char message[1024];
    va_list arg;
    if (format) {
        va_start (arg, format);
        vsnprintf(message, sizeof(message), format, arg);
        va_end (arg);
    }
    
	fprintf(stderr, "%s:%ld:%ld: error: %s",
		lexer_get_path(lexer),
		token_get_line(token),
		token_get_column(token),
		message);
}

static void
parser_expected(parser_t *parser, int32_t type)
{
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	fprintf(stderr,
		"%s:%ld:%ld: error: "
		"expected '%s' current is '%s'\n",
		lexer_get_path(lexer),
		token_get_line(token),
		token_get_column(token),
		token_get_name(type),
		token_get_name(token_get_type(token)));
}

static int32_t
parser_match(parser_t *parser, int32_t type){
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if(token_get_type(token) == type){
		lexer_get_next_token(lexer); 
	} else {
		parser_expected(parser, type);
		return 0;
	}
	return 1;
}

static int32_t
parser_match_test(parser_t *parser, int32_t type){
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if(token_get_type(token) == type){
		lexer_get_next_token(lexer); 
	} else {
		return 0;
	}
	return 1;
}

static node_t *
parser_id(parser_t *parser){
	node_t *node;
	
	lexer_t *lexer = parser->lexer;
	token_t *token = lexer_get_token(lexer);
	
	node = node_make_id(token_get_value(token));
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_ID)){
		return NULL;
	}
	return node;
}

static int32_t
parser_id_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_ID)){
		return 0;
	}
	return 1;
}

static node_t *
parser_number(parser_t *parser){
	node_t *node;
	
	lexer_t *lexer = parser->lexer;
	token_t *token = lexer_get_token(lexer);
	
	node = node_make_number(token_get_value(token));
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_NUMBER)){
		return NULL;
	}
	return node;
}
 
static node_t *
parser_letters(parser_t *parser){
	node_t *node;
	
	lexer_t *lexer = parser->lexer;
	token_t *token = lexer_get_token(lexer);
	
	node = node_make_letters(token_get_value(token));
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_LETTERS)){
		return NULL;
	}
	return node;
}

static node_t *
parser_null(parser_t *parser){
	node_t *node;
	
	node = node_make_null();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_NULL)){
		return NULL;
	}
	return node;
}

static node_t *
parser_this(parser_t *parser){
	node_t *node;
	
	node = node_make_this();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_THIS)){
		return NULL;
	}
	return node;
}

static node_t *
parser_super(parser_t *parser){
	node_t *node;
	
	node = node_make_super();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_SUPER)){
		return NULL;
	}
	return node;
}

static node_t *
parser_list(parser_t *parser)
{
	node_t *expr;
	list_t *expr_list;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_LBRACKET)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	expr_list = list_create();
	if(!expr_list){
		return NULL;
	}
	
	expr = NULL;
	while (token_get_type(token) != TOKEN_RBRACKET) {
		expr = parser_expression(parser);
		if (!expr) {
			return NULL;
		}

		if(!list_rpush(expr_list, (list_value_t)expr)){
			return NULL;
		}

		if (token_get_type(token) != TOKEN_COMMA) {
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_RBRACKET)){
		return NULL;
	}

	return node_make_list(expr_list);
}

static node_t *
parser_map(parser_t *parser)
{
	node_t *key;
	node_t *value;
	node_t *element;
	list_t *element_list;

	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}
	
	lexer_t *lexer = parser->lexer;
	token_t *token = lexer_get_token(lexer);
	
	element_list = list_create();
	if(!element_list){
		return NULL;
	}
	
	element = NULL;
	while (token_get_type(token) != TOKEN_RBRACE) {
		key = parser_expression(parser);
		if (!key) {
			return NULL;
		}
		if(!parser_match(parser, TOKEN_COLON)){
			return NULL;
		}

		value = parser_expression(parser);
		if (!value) {
			return NULL;
		}

		element = node_make_element(key, value);
		if (!element) {
			return NULL;
		}

		if(!list_rpush(element_list, (list_value_t)element)){
			return NULL;
		}

		if (token_get_type(token) != TOKEN_COMMA) {
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}

	return node_make_map(element_list);
}

static node_t *
parser_primary(parser_t *parser){
	node_t *node;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	switch (token_get_type(token)){
	case TOKEN_ID:
		node = parser_id(parser);
		break;
	
	case TOKEN_NUMBER:
		node = parser_number(parser);
		break;
	
	case TOKEN_LETTERS:
		node = parser_letters(parser);
		break;
	
	case TOKEN_NULL:
		node = parser_null(parser);
		break;
	
	case TOKEN_THIS:
		node = parser_this(parser);
		break;
	
	case TOKEN_SUPER:
		node = parser_super(parser);
		break;
		
	case TOKEN_LBRACKET:
		node = parser_list(parser);
		break;
		
	case TOKEN_LBRACE:
		node = parser_map(parser);
		break;
		
	case TOKEN_LPAREN:
		if(!parser_match(parser, TOKEN_LPAREN)){
			return NULL;
		}
		node = parser_expression(parser);
		if(!node){
			return NULL;
		}
		if(!parser_match(parser, TOKEN_RPAREN)){
			return NULL;
		}
		break;
		
	default:
		parser_error(parser, "unknown token\n");
		return NULL;
	}
	
	return node;
}

static node_t *
parser_char(parser_t *parser){
	node_t *node;
	
	node = node_make_char();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_CHAR)){
		return NULL;
	}
	return node;
}

static int32_t
parser_char_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_CHAR)){
		return 0;
	}
	return 1;
}

static node_t *
parser_string(parser_t *parser){
	node_t *node;
	
	node = node_make_string();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_STRING)){
		return NULL;
	}
	return node;
}

static int32_t
parser_string_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_STRING)){
		return 0;
	}
	return 1;
}

static node_t *
parser_int(parser_t *parser){
	node_t *node;

	node = node_make_int();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_INT)){
		return NULL;
	}
	return node;
}

static int32_t
parser_int_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_INT)){
		return 0;
	}
	return 1;
}

static node_t *
parser_long(parser_t *parser){
	node_t *node;

	node = node_make_long();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_LONG)){
		return NULL;
	}
	return node;
}

static int32_t
parser_long_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_LONG)){
		return 0;
	}
	return 1;
}

static node_t *
parser_float(parser_t *parser){
	node_t *node;

	node = node_make_float();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_FLOAT)){
		return NULL;
	}
	return node;
}

static int32_t
parser_float_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_FLOAT)){
		return 0;
	}
	return 1;
}

static node_t *
parser_double(parser_t *parser){
	node_t *node;

	node = node_make_double();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_DOUBLE)){
		return NULL;
	}
	return node;
}

static int32_t
parser_double_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_DOUBLE)){
		return 0;
	}
	return 1;
}

static node_t *
parser_any(parser_t *parser){
	node_t *node;
	
	node = node_make_any();
	if(!(node)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_ANY)){
		return NULL;
	}
	return node;
}

static int32_t
parser_any_test(parser_t *parser){
	if(!parser_match_test(parser, TOKEN_ANY)){
		return 0;
	}
	return 1;
}

static node_t *
parser_tuple(parser_t *parser){
	node_t *type;
	list_t *type_list;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if(!parser_match(parser, TOKEN_LBRACKET)){
		return NULL;
	}
	
	type_list = list_create();
	if(!type_list){
		return NULL;
	}
	
	type = NULL;
	while (token_get_type(token) != TOKEN_RBRACKET) {
		type = parser_typing(parser);
		if (!type) {
			return NULL;
		}

		if(!list_rpush(type_list, (list_value_t)type)){
			return NULL;
		}

		if (token_get_type(token) != TOKEN_COMMA) {
			break;
		}
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_RBRACKET)){
		return NULL;
	}
	
	return node_make_tuple(type_list);
}

static int32_t
parser_tuple_test(parser_t *parser){
	int32_t type;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if(!parser_match_test(parser, TOKEN_LBRACKET)){
		return 0;
	}
	
	while (token_get_type(token) != TOKEN_RBRACKET) {
		type = parser_typing_test(parser);
		if (!type) {
			return 0;
		}

		if (token_get_type(token) != TOKEN_COMMA) {
			break;
		}
		if(!parser_match_test(parser, TOKEN_COMMA)){
			return 0;
		}
	}
	if(!parser_match_test(parser, TOKEN_RBRACKET)){
		return 0;
	}
	
	return 1;
}

static node_t *
parser_tag_list(parser_t *parser){
	node_t *type;
	list_t *type_list;
	
	lexer_t *lexer;
	token_t *token;
	
	type_list = list_create();
	if(!type_list){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	type = NULL;
	while (token_get_type(token) != TOKEN_GT) {
		type = parser_typing(parser);
		if (!type) {
			return NULL;
		}

		if(!list_rpush(type_list, (list_value_t)type)){
			return NULL;
		}

		if (token_get_type(token) != TOKEN_COMMA) {
			break;
		}
		
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	
	return node_make_tag_list(type_list);
}

static int32_t
parser_tag_list_test(parser_t *parser){
	int32_t type;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (token_get_type(token) != TOKEN_GT) {
		type = parser_typing_test(parser);
		if (!type) {
			return 0;
		}

		if (token_get_type(token) != TOKEN_COMMA) {
			break;
		}
		
		if(!parser_match_test(parser, TOKEN_COMMA)){
			return 0;
		}
	}
	
	return 1;
}

static node_t *
parser_type(parser_t *parser){
	node_t *node;
	node_t *type_list;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	switch (token_get_type(token)){
	case TOKEN_ID:
		node = parser_id(parser);
		if(!node){
			return NULL;
		}
		if(token_get_type(token) == TOKEN_LT){
			if(!parser_match(parser, TOKEN_LT)){
				return NULL;
			}
			type_list = parser_tag_list(parser);
			if(!type_list){
				return NULL;
			}
			if(!parser_match(parser, TOKEN_GT)){
				return NULL;
			}
			node = node_make_tag(node, type_list);
		}
		break;
		
	case TOKEN_CHAR:
		node = parser_char(parser);
		break;
		
	case TOKEN_STRING:
		node = parser_string(parser);
		break;
	
	case TOKEN_INT:
		node = parser_int(parser);
		break;
	
	case TOKEN_LONG:
		node = parser_long(parser);
		break;
		
	case TOKEN_FLOAT:
		node = parser_float(parser);
		break;
		
	case TOKEN_DOUBLE:
		node = parser_double(parser);
		break;
		
	case TOKEN_ANY:
		node = parser_any(parser);
		break;
		
	case TOKEN_LBRACKET:
		node = parser_tuple(parser);
		break;
		
	default:
		parser_error(parser, "unknown type\n");
		return NULL;
	}
	
	return node;
}

static int32_t
parser_type_test(parser_t *parser){
	int32_t node;
	int32_t type_list;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = 1;
	switch (token_get_type(token)){
	case TOKEN_ID:
		node = parser_id_test(parser);
		if(!node){
			return 0;
		}
		if(token_get_type(token) == TOKEN_LT){
			if(!parser_match_test(parser, TOKEN_LT)){
				return 0;
			}
			type_list = parser_tag_list_test(parser);
			if(!type_list){
				return 0;
			}
			if(!parser_match_test(parser, TOKEN_GT)){
				return 0;
			}
			node = 1;
		}
		break;
		
	case TOKEN_CHAR:
		node = parser_char_test(parser);
		break;
		
	case TOKEN_STRING:
		node = parser_string_test(parser);
		break;
	
	case TOKEN_INT:
		node = parser_int_test(parser);
		break;
	
	case TOKEN_LONG:
		node = parser_long_test(parser);
		break;
		
	case TOKEN_FLOAT:
		node = parser_float_test(parser);
		break;
		
	case TOKEN_DOUBLE:
		node = parser_double_test(parser);
		break;
		
	case TOKEN_ANY:
		node = parser_any_test(parser);
		break;
		
	case TOKEN_LBRACKET:
		node = parser_tuple_test(parser);
		break;
		
	default:
		return 0;
	}
	
	return node;
}

static node_t *
parser_pointer(parser_t *parser)
{
	node_t *node;

	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = parser_type(parser);
	if(!node){
		return node;
	}
	
	while(node){
		switch (token_get_type(token)) {
		case TOKEN_STAR:
			if(!parser_match(parser, TOKEN_STAR)){
				return NULL;
			}
			
			node = node_make_pointer(node);
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static int32_t
parser_pointer_test(parser_t *parser)
{
	int32_t node;

	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = parser_type_test(parser);
	if(!node){
		return node;
	}
	
	while(node){
		switch (token_get_type(token)) {
		case TOKEN_STAR:
			if(!parser_match_test(parser, TOKEN_STAR)){
				return 0;
			}
			
			node = 1;
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static node_t *
parser_array(parser_t *parser)
{
	node_t *node;

	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = parser_pointer(parser);
	if(!node){
		return node;
	}
	
	while(node){
		switch (token_get_type(token)) {
		case TOKEN_LBRACKET:
			if(!parser_match(parser, TOKEN_LBRACKET)){
				return NULL;
			}
			if(!parser_match(parser, TOKEN_RBRACKET)){
				return NULL;
			}
			
			node = node_make_array(node);
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static int32_t
parser_array_test(parser_t *parser)
{
	int32_t node;

	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = parser_pointer_test(parser);
	if(!node){
		return 0;
	}
	
	while(node){
		switch (token_get_type(token)) {
		case TOKEN_LBRACKET:
			if(!parser_match_test(parser, TOKEN_LBRACKET)){
				return 0;
			}
			if(!parser_match_test(parser, TOKEN_RBRACKET)){
				return 0;
			}
			
			node = 1;
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static node_t *
parser_typing(parser_t *parser){
	return parser_array(parser);
}

static int32_t
parser_typing_test(parser_t *parser){
	return parser_array_test(parser);
}

static node_t *
parser_argument(parser_t *parser)
{
	node_t *node;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
		
	switch (token_get_type(token)) {
	case TOKEN_FN:
		node = parser_fn_stmt(parser);
		break;
		
	default:
		node = parser_expression(parser);
		break;
	}
	
	return node;
}

static node_t *
parser_argument_list(parser_t *parser){
	node_t *node;
	node_t *argument;
	list_t *list;
	
	lexer_t *lexer;
	token_t *token;
	
	list = list_create();
	if(!(list)){
		return NULL;
	}
	
	argument = parser_argument(parser);
	if(!(argument)){
		return NULL;
	}
	
	if(!list_rpush(list, (list_value_t)argument)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if(token_get_type(token) == TOKEN_COMMA){
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
		
		while(token_get_type(token) != TOKEN_RPAREN){
			argument = parser_argument(parser);
			if(!(argument)){
				return NULL;
			}
		
			if(!list_rpush(list, (list_value_t)argument)){
				return NULL;
			}
			
			if(token_get_type(token) != TOKEN_COMMA){
				break;
			}
			
			if(!parser_match(parser, TOKEN_COMMA)){
				return NULL;
			}
		}
	}
	
	if(!(node = node_make_argument(list))){
		return NULL;
	}
	
	return node;
}

static int32_t
parser_is_tag_list_test(parser_t *parser){
	int32_t is_tag_list;
	
	lexer_t *lexer;
	token_t *token;
	
	uint64_t position;
	uint64_t line;
	uint64_t column;
	
	token_t token_store;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	position = lexer_get_position(lexer);
	line = lexer_get_line(lexer);
	column = lexer_get_column(lexer);
	
	token_store = *token;
	
	if(!parser_match_test(parser, TOKEN_LT)){
		is_tag_list = 0;
		goto end_unit_test;
	}
	
	if(token_get_type(token) != TOKEN_GT){
		is_tag_list = parser_tag_list_test(parser);
		if(!is_tag_list){
			goto end_unit_test;
		}
	}
				
	if(!parser_match_test(parser, TOKEN_GT)){
		is_tag_list = 0;
		goto end_unit_test;
	}
	
end_unit_test:	
	lexer_set_token(lexer, token_store);
	
	lexer_set_position(lexer, position);
	lexer_set_line(lexer, line);
	lexer_set_column(lexer, column);
	
	return is_tag_list;
}

static node_t *
parser_postfix(parser_t *parser){
	node_t *node;
	node_t *start;
	node_t *step;
	node_t *stop;
	node_t *right;
	node_t *argument_list;
	node_t *tag_list;
	
	int32_t slice;
	
	lexer_t *lexer;
	token_t *token;
	
	node = parser_primary(parser);
	if(!(node)){
		return NULL;
	}
	
	while(node){
		lexer = parser->lexer;
		token = lexer_get_token(lexer);
		
		switch(token_get_type(token)){
		case TOKEN_LPAREN:
		case TOKEN_LT:
			tag_list = NULL;
			if(token_get_type(token) == TOKEN_LT){
				if(!parser_is_tag_list_test(parser)){
					return node;
				}
				if(!parser_match(parser, TOKEN_LT)){
					return NULL;
				}
				
				tag_list = NULL;
				if(token_get_type(token) != TOKEN_GT){
					tag_list = parser_tag_list(parser);
					if(!(tag_list)){
						return NULL;
					}
				}
				
				if(!parser_match(parser, TOKEN_GT)){
					return NULL;
				}
			}
			
			if(!parser_match(parser, TOKEN_LPAREN)){
				return NULL;
			}
			
			argument_list = NULL;
			if(token_get_type(token) != TOKEN_RPAREN){
				argument_list = parser_argument_list(parser);
				if(!(argument_list)){
					return NULL;
				}
			}
			
			if(!parser_match(parser, TOKEN_RPAREN)){
				return NULL;
			}
			node = node_make_call(node, tag_list, argument_list);
			break;
		
		case TOKEN_LBRACKET:
			if(!parser_match(parser, TOKEN_LBRACKET)){
				return NULL;
			}
			if (token_get_type(token) == TOKEN_COLON) {
				if(!parser_match(parser, TOKEN_COLON)){
					return NULL;
				}
				start = NULL;
				slice = 1;
			} else {
				start = parser_expression(parser);
				if (!start) {
					return NULL;
				}
				if (token_get_type(token) == TOKEN_COLON) {
					if(!parser_match(parser, TOKEN_COLON)){
						return NULL;
					}
					slice = 1;
				}
			}
			
			stop = NULL;
			if (token_get_type(token) == TOKEN_COLON) {
				if(!parser_match(parser, TOKEN_COLON)){
					return NULL;
				}
				stop = NULL;
			} else if (slice) {
				stop = NULL;
				if (token_get_type(token) != TOKEN_RBRACKET) {
					stop = parser_expression(parser);
					if (!stop) {
						return NULL;
					}
				}
				if (token_get_type(token) == TOKEN_COLON) {
					if(!parser_match(parser, TOKEN_COLON)){
						return NULL;
					}
				}
			}
			
			if (slice) {
				step = NULL;
				if (token_get_type(token) != TOKEN_RBRACKET) {
					step = parser_expression(parser);
					if (!step) {
						return NULL;
					}
				}
				if(!parser_match(parser, TOKEN_RBRACKET)){
					return NULL;
				}

				node = node_make_get_slice(node, start, stop, step);
			} else {
				if(!parser_match(parser, TOKEN_RBRACKET)){
					return NULL;
				}
				if (!start) {
					return NULL;
				}
				node = node_make_get_item(node, start);
			}
			break;
			
		case TOKEN_DOT:
			if(!parser_match(parser, TOKEN_DOT)){
				return NULL;
			}
			
			right = parser_id(parser);
			if (!(right)) {
				return NULL;
			}
			
			if(!(node = node_make_get_attr(node, right))){
				return NULL;
			}
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
	node_t *node;
	node_t *left;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
		
	switch (token_get_type(token)) {
	case TOKEN_TILDE:
		if(!parser_match(parser, TOKEN_TILDE)){
			return NULL;
		}
		left = parser_prefix(parser);
		if (!left) {
			return NULL;
		}
		node = node_make_tilde(left);
		break;

	case TOKEN_NOT:
		if(!parser_match(parser, TOKEN_NOT)){
			return NULL;
		}
		left = parser_prefix(parser);
		if (!left) {
			return NULL;
		}
		node = node_make_not(left);
		break;

	case TOKEN_MINUS:
		if(!parser_match(parser, TOKEN_MINUS)){
			return NULL;
		}
		left = parser_prefix(parser);
		if (!left) {
			return NULL;
		}
		node = node_make_neg(left);
		break;

	case TOKEN_PLUS:
		if(!parser_match(parser, TOKEN_PLUS)){
			return NULL;
		}
		left = parser_prefix(parser);
		if (!left) {
			return NULL;
		}
		node = node_make_pos(left);
		break;
		
	case TOKEN_STAR:
		if(!parser_match(parser, TOKEN_STAR)){
			return NULL;
		}
		left = parser_prefix(parser);
		if (!left) {
			return NULL;
		}
		node = node_make_get_value(left);
		break;
		
	case TOKEN_AND:
		if(!parser_match(parser, TOKEN_AND)){
			return NULL;
		}
		left = parser_prefix(parser);
		if (!left) {
			return NULL;
		}
		node = node_make_get_address(left);
		break;

	default:
		node = parser_postfix(parser);
	}

	return node;
}

static node_t *
parser_as(parser_t *parser)
{
	node_t *node;
	node_t *right;
	
	lexer_t *lexer;
	token_t *token;

	node = parser_prefix(parser);
	if(!node){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_AS:
			if(!parser_match(parser, TOKEN_AS)){
				return NULL;
			}
			
			right = parser_typing(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_as(node, right);
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static node_t *
parser_multiplicative(parser_t *parser)
{
	node_t *node;
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_as(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_STAR:
			if(!parser_match(parser, TOKEN_STAR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_mul_assign(node, right);
				break;
			}
			
			right = parser_as(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_mul(node, right);
			break;

		case TOKEN_SLASH:
			if(!parser_match(parser, TOKEN_SLASH)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_div_assign(node, right);
				break;
			}
			
			right = parser_as(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_div(node, right);
			break;

		case TOKEN_PERCENT:
			if(!parser_match(parser, TOKEN_PERCENT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_mod_assign(node, right);
				break;
			}
			
			right = parser_as(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_mod(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_multiplicative(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_PLUS:
			if(!parser_match(parser, TOKEN_PLUS)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_add_assign(node, right);
				break;
			}
			
			right = parser_multiplicative(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_plus(node, right);
			break;

		case TOKEN_MINUS:
			if(!parser_match(parser, TOKEN_MINUS)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_sub_assign(node, right);
				break;
			}
			
			right = parser_multiplicative(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_minus(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_addative(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_LT:
			if(!parser_match(parser, TOKEN_LT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_LT){
				if(!parser_match(parser, TOKEN_LT)){
					return NULL;
				}
				
				
				if(token_get_type(token) == TOKEN_EQ){
					if(!parser_match(parser, TOKEN_EQ)){
						return NULL;
					}
					
					right = parser_expression(parser);
					if (!(right)) {
						return NULL;
					}
					
					node = node_make_shl_assign(node, right);
					break;
				}
				
				right = parser_addative(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_shl(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_shifting(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_le(node, right);
				break;
			}
			
			right = parser_shifting(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_lt(node, right);
			break;

		case TOKEN_GT:
			if(!parser_match(parser, TOKEN_GT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_GT){
				if(!parser_match(parser, TOKEN_GT)){
					return NULL;
				}
				
				if(token_get_type(token) == TOKEN_EQ){
					if(!parser_match(parser, TOKEN_EQ)){
						return NULL;
					}
					
					right = parser_expression(parser);
					if (!(right)) {
						return NULL;
					}
					
					node = node_make_shr_assign(node, right);
					break;
				}
			
				right = parser_addative(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_shr(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_shifting(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_ge(node, right);
				break;
			}
			
			right = parser_shifting(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_gt(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_shifting(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_LT:
			if(!parser_match(parser, TOKEN_LT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_shifting(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_le(node, right);
				break;
			}
			
			right = parser_shifting(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_lt(node, right);
			break;

		case TOKEN_GT:
			if(!parser_match(parser, TOKEN_GT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_shifting(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_ge(node, right);
				break;
			}
			
			right = parser_shifting(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_gt(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	node = parser_relational(parser);
	if(!node){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_EQ:
			if(!parser_match(parser, TOKEN_EQ)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_relational(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_eq(node, right);
				break;
			}
			
			right = parser_expression(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_assign(node, right);
			break;

		case TOKEN_NOT:
			if(!parser_match(parser, TOKEN_NOT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
			
				right = parser_relational(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_neq(node, right);
				break;
			}
			
			parser_error(parser, "'!' operator is not a binary operator\n");
			return NULL;

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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_equality(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_AND:
			if(!parser_match(parser, TOKEN_AND)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_AND){
				if(!parser_match(parser, TOKEN_AND)){
					return NULL;
				}
				
				right = parser_bitwise_or(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_land(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_and_assign(node, right);
				break;
			}
			
			right = parser_equality(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_and(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;

	if(!(node = parser_bitwise_and(parser))){
		return NULL;
	}

	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_CARET:
			if(!parser_match(parser, TOKEN_CARET)){
				return NULL;
			}
			
			right = parser_bitwise_and(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_xor(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_bitwise_xor(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_OR:
			if(!parser_match(parser, TOKEN_OR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_OR){
				if(!parser_match(parser, TOKEN_OR)){
					return NULL;
				}
				
				right = parser_logical_and(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_lor(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_or_assign(node, right);
				break;
			}
			
			right = parser_bitwise_xor(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_or(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_bitwise_or(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_AND:
			if(!parser_match(parser, TOKEN_AND)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_AND){
				if(!parser_match(parser, TOKEN_AND)){
					return NULL;
				}
				
				right = parser_bitwise_or(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_land(node, right);
				break;
			}
			
			right = parser_equality(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_and(node, right);
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
	node_t *right;
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_logical_and(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_OR:
			if(!parser_match(parser, TOKEN_OR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_OR){
				if(!parser_match(parser, TOKEN_OR)){
					return NULL;
				}
				
				right = parser_logical_and(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_lor(node, right);
				break;
			}
			
			right = parser_bitwise_xor(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_or(node, right);
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
	node_t *node;
	node_t *true_expr;
	node_t *false_expr;
	lexer_t *lexer;
	token_t *token;
		
	if(!(node = parser_logical_or(parser))){
		return NULL;
	}
	
	if (node) {
		lexer = parser->lexer;
		token = lexer_get_token(lexer);
	
		switch (token_get_type(token)) {
		case TOKEN_QUESTION:
			if(!parser_match(parser, TOKEN_QUESTION)){
				return NULL;
			}
			true_expr = NULL;
			if (token_get_type(token) != TOKEN_COLON) {
				true_expr = parser_logical_or(parser);
				if (!true_expr) {
					break;
				}
			}
			if(!parser_match(parser, TOKEN_COLON)){
				return NULL;
			}

			false_expr = parser_expression(parser);
			if (!false_expr) {
				break;
			}

			node = node_make_conditional(node, true_expr, false_expr);
			break;

		default:
			return node;
		}
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
	node_t *node;
	node_t *right;
	list_t *list;
	lexer_t *lexer;
	token_t *token;
		
	node = parser_expression(parser);
	if(!(node)){
		return NULL;
	}
	
	if (node) {
		lexer = parser->lexer;
		token = lexer_get_token(lexer);
		
		switch (token_get_type(token)) {
		case TOKEN_COMMA:
			list = list_create();
			if(!list){
				return NULL;
			}
			
			if(!(list_rpush(list, (list_value_t)node))){
				return NULL;
			}
			
			while (token_get_type(token) == TOKEN_COMMA) {
				if(!parser_match(parser, TOKEN_COMMA)){
					return NULL;
				}
				
				node = parser_expression(parser);
				if (!(node)) {
					break;
				}
				
				if(!(list_rpush(list, (list_value_t)node))){
					return NULL;
				}
			}
			node = node_make_unpack(list);
			break;

		case TOKEN_EQ:
			if(!parser_match(parser, TOKEN_EQ)){
				return NULL;
			}
			
			right = parser_expression(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_assign(node, right);
			break;

		case TOKEN_PLUS:
			if(!parser_match(parser, TOKEN_PLUS)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_add_assign(node, right);
				break;
			}
			
			right = parser_multiplicative(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_plus(node, right);
			break;

		case TOKEN_MINUS:
			if(!parser_match(parser, TOKEN_MINUS)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_sub_assign(node, right);
				break;
			}
			
			right = parser_multiplicative(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_minus(node, right);
			break;

		case TOKEN_STAR:
			if(!parser_match(parser, TOKEN_STAR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_mul_assign(node, right);
				break;
			}
			
			right = parser_as(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_mul(node, right);
			break;

		case TOKEN_SLASH:
			if(!parser_match(parser, TOKEN_SLASH)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_div_assign(node, right);
				break;
			}
			
			right = parser_as(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_div(node, right);
			break;

		case TOKEN_PERCENT:
			if(!parser_match(parser, TOKEN_PERCENT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_mod_assign(node, right);
				break;
			}
			
			right = parser_as(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_mod(node, right);
			break;

		case TOKEN_AND:
			if(!parser_match(parser, TOKEN_AND)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_AND){
				if(!parser_match(parser, TOKEN_AND)){
					return NULL;
				}
				
				right = parser_bitwise_or(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_land(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_and_assign(node, right);
				break;
			}
			
			right = parser_equality(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_and(node, right);
			break;

		case TOKEN_OR:
			if(!parser_match(parser, TOKEN_OR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_OR){
				if(!parser_match(parser, TOKEN_OR)){
					return NULL;
				}
				
				right = parser_logical_and(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_lor(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_expression(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_or_assign(node, right);
				break;
			}
			
			right = parser_bitwise_xor(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_or(node, right);
			break;
			
		case TOKEN_LT:
			if(!parser_match(parser, TOKEN_LT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_LT){
				if(!parser_match(parser, TOKEN_LT)){
					return NULL;
				}
				
				
				if(token_get_type(token) == TOKEN_EQ){
					if(!parser_match(parser, TOKEN_EQ)){
						return NULL;
					}
					
					right = parser_expression(parser);
					if (!(right)) {
						return NULL;
					}
					
					node = node_make_shl_assign(node, right);
					break;
				}
				
				right = parser_addative(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_shl(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_shifting(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_le(node, right);
				break;
			}
			
			right = parser_shifting(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_lt(node, right);
			break;

		case TOKEN_GT:
			if(!parser_match(parser, TOKEN_GT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_GT){
				if(!parser_match(parser, TOKEN_GT)){
					return NULL;
				}
				
				if(token_get_type(token) == TOKEN_EQ){
					if(!parser_match(parser, TOKEN_EQ)){
						return NULL;
					}
					
					right = parser_expression(parser);
					if (!(right)) {
						return NULL;
					}
					
					node = node_make_shr_assign(node, right);
					break;
				}
			
				right = parser_addative(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_shr(node, right);
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				
				right = parser_shifting(parser);
				if (!(right)) {
					return NULL;
				}
				
				node = node_make_ge(node, right);
				break;
			}
			
			right = parser_shifting(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_gt(node, right);
			break;

		default:
			return node;
		}
	}

	return node;
}

static node_t *
parser_if_stmt(parser_t *parser)
{
	node_t *expr;
	node_t *then_block_stmt;
	node_t *else_block_stmt;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_IF)){
		return NULL;
	}

	if(!parser_match(parser, TOKEN_LPAREN)){
		return NULL;
	}
	
	expr = parser_expression(parser);
	if (!expr) {
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_RPAREN)){
		return NULL;
	}

	list_rpush(parser->scope_list, parser->scope_type);
	parser->scope_type = PARSER_SCOPE_IF;
	
	then_block_stmt = parser_block_stmt(parser);
	if (!then_block_stmt) {
		return NULL;
	}
	
	parser->scope_type = list_content(list_rpop(parser->scope_list));

	lexer = parser->lexer;
	token = lexer_get_token(lexer);
		
	else_block_stmt = NULL;
	if (token_get_type(token) == TOKEN_ELSE) {
		if(!parser_match(parser, TOKEN_ELSE)){
			return NULL;
		}
	
		if (token_get_type(token) == TOKEN_IF) {
			else_block_stmt = parser_if_stmt(parser);
		} else {
			list_rpush(parser->scope_list, parser->scope_type);
			parser->scope_type = PARSER_SCOPE_IF;
		
			else_block_stmt = parser_block_stmt(parser);
			
			parser->scope_type = list_content(list_rpop(parser->scope_list));
		}

		if (!else_block_stmt) {
			return NULL;
		}
	}

	return node_make_if(expr, then_block_stmt, else_block_stmt);
}

static node_t *
parser_while_stmt(parser_t *parser)
{
	node_t *expr;
	node_t *block_stmt;

	if(!parser_match(parser, TOKEN_WHILE)){
		return NULL;
	}

	if(!parser_match(parser, TOKEN_LPAREN)){
		return NULL;
	}
	
	expr = parser_expression(parser);
	if (!expr) {
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_RPAREN)){
		return NULL;
	}
	
	list_rpush(parser->scope_list, parser->scope_type);
	parser->scope_type = PARSER_SCOPE_WHILE;
	
	parser->loop_depth += 1;

	block_stmt = parser_block_stmt(parser);
	if (!block_stmt) {
		return NULL;
	}
	
	parser->loop_depth -= 1;
	
	parser->scope_type = list_content(list_rpop(parser->scope_list));

	return node_make_while(expr, block_stmt);
}

static node_t *
parser_break_stmt(parser_t *parser)
{
	if(parser->loop_depth <= 0){
		parser_error(parser, "'break' is not within a loop\n");
		return NULL;
	}
	if(!parser_match(parser, TOKEN_BREAK)){
		return NULL;
	}
	if(!parser_match(parser, TOKEN_SEMICOLON)){
		return NULL;
	}

	return node_make_break();
}

static node_t *
parser_continue_stmt(parser_t *parser)
{
	if(parser->loop_depth <= 0){
		parser_error(parser, "'continue' is not within a loop\n");
		return NULL;
	}
	if(!parser_match(parser, TOKEN_CONTINUE)){
		return NULL;
	}
	if(!parser_match(parser, TOKEN_SEMICOLON)){
		return NULL;
	}

	return node_make_continue();
}

static node_t *
parser_return_stmt(parser_t *parser)
{
	node_t *expr;
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_RETURN)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	expr = NULL;
	if (token_get_type(token) != TOKEN_SEMICOLON) {
		expr = parser_expression(parser);
		if (!expr) {
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_SEMICOLON)){
		return NULL;
	}

	return node_make_return(expr);
}

static node_t *
parser_prototype(parser_t *parser)
{
	node_t *node;
	node_t *right;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!(node = parser_id(parser))){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while (node) {
		switch (token_get_type(token)) {
		case TOKEN_AS:
			if(!parser_match(parser, TOKEN_AS)){
				return NULL;
			}
			
			right = parser_id(parser);
			if (!(right)) {
				return NULL;
			}
			
			node = node_make_as(node, right);
			break;

		default:
			return node;
		}
	}
	
	return node;
}

static node_t *
parser_directory(parser_t *parser)
{
	node_t *node;
	list_t *wise_list;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}
		
	wise_list = list_create();
	if(!wise_list){
		return NULL;
	}
		
	node = parser_prototype(parser);
	if(!node){
		return NULL;
	}
		
	if(!list_rpush(wise_list, (list_value_t)node)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
		
	while(token_get_type(token) == TOKEN_COMMA){
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
		node = parser_prototype(parser);
		if(!node){
			return NULL;
		}
			
		if(!list_rpush(wise_list, (list_value_t)node)){
			return NULL;
		}
	}
		
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}
	
	return node_make_directory(wise_list);
}

static node_t *
parser_var_stmt(parser_t *parser)
{
	node_t *name;
	node_t *label;
	node_t *expr;
	node_t *node;
	int32_t is_directory;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	is_directory = 0;
	if (token_get_type(token) == TOKEN_LBRACE) {
		name = parser_directory(parser);
		is_directory = 1;
	} else {
		name = parser_id(parser);
	}
	
	if(!name){
		return NULL;
	}

	label = NULL;
	if(!is_directory){
		if (token_get_type(token) == TOKEN_COLON) {
			if(!parser_match(parser, TOKEN_COLON)){
				return NULL;
			}
			label = parser_typing(parser);
			if (!label) {
				return NULL;
			}
		}
	}
		
	expr = NULL;
	if (token_get_type(token) == TOKEN_EQ || is_directory) {
		if(!parser_match(parser, TOKEN_EQ)){
			return NULL;
		}
		expr = parser_expression(parser);
		if (!expr) {
			return NULL;
		}
	}

	node = node_make_var(name, label, expr);
		
	return node;
}

static node_t *
parser_var_list_stmt(parser_t *parser)
{
	node_t *node;
	list_t *list;
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_VAR)){
		return NULL;
	}
	
	node = parser_var_stmt(parser);
	if(!node){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if(token_get_type(token) == TOKEN_COMMA){
		list = list_create();
		if(!list){
			return NULL;
		}
		
		if(!list_rpush(list, (list_value_t)node)){
			return NULL;
		}
		
		while(token_get_type(token) == TOKEN_COMMA){
			if(!parser_match(parser, TOKEN_COMMA)){
				return NULL;
			}
			node = parser_var_stmt(parser);
			if(!node){
				return NULL;
			}
			
			if(!list_rpush(list, (list_value_t)node)){
				return NULL;
			}
		}
		
		node = node_make_var_list(list);
	}
	
	if(!parser_match(parser, TOKEN_SEMICOLON)){
		return NULL;
	}
	
	return node;
}

static node_t *
parser_parameter(parser_t *parser)
{
	node_t *name;
	node_t *label;
	node_t *expr;
	node_t *node;
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	if (token_get_type(token) == TOKEN_VAR) {
		if(!parser_match(parser, TOKEN_VAR)){
			return NULL;
		}
	}
	
	name = parser_id(parser);
	
	label = NULL;
	if (token_get_type(token) == TOKEN_COLON) {
		if(!parser_match(parser, TOKEN_COLON)){
			return NULL;
		}
		label = parser_typing(parser);
		if (!label) {
			return NULL;
		}
	}
	
	expr = NULL;
	if (token_get_type(token) == TOKEN_EQ) {
		if(!parser_match(parser, TOKEN_EQ)){
			return NULL;
		}
		expr = parser_expression(parser);
		if (!expr) {
			return NULL;
		}
	}

	node = node_make_parameter(name, label, expr);

	return node;
}

static node_t *
parser_parameter_list(parser_t *parser)
{
	node_t *node;
	node_t *parameter;
	list_t *list;
	lexer_t *lexer;
	token_t *token;
	
	parameter = parser_parameter(parser);
	if (!parameter) {
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = parameter;
	if (token_get_type(token) == TOKEN_COMMA) {
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
		
		list = list_create();
		while (token_get_type(token) != TOKEN_RPAREN) {
			parameter = parser_parameter(parser);
			if (!parameter) {
				return NULL;
			}

			if(!list_rpush(list, (list_value_t)parameter)){
				return NULL;
			}

			if (token_get_type(token) != TOKEN_COMMA) {
				break;
			}
			if(!parser_match(parser, TOKEN_COMMA)){
				return NULL;
			}
		}
		node = node_make_parameter_list(list);
	}

	return node;
}

static node_t *
parser_generic(parser_t *parser)
{
	node_t *name;
	node_t *label;
	node_t *default_value;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	name = parser_id(parser);
	
	label = NULL;
	if (token_get_type(token) == TOKEN_COLON) {
		if(!parser_match(parser, TOKEN_COLON)){
			return NULL;
		}
		label = parser_typing(parser);
		if (!label) {
			return NULL;
		}
	}
	
	default_value = NULL;
	if (token_get_type(token) == TOKEN_EQ) {
		if(!parser_match(parser, TOKEN_EQ)){
			return NULL;
		}
		default_value = parser_typing(parser);
		if (!default_value) {
			return NULL;
		}
	}

	return node_make_generic(name, label, default_value);
}

static node_t *
parser_generic_list(parser_t *parser)
{
	node_t *node;
	node_t *generic;
	list_t *generic_list;
	
	lexer_t *lexer;
	token_t *token;
	
	generic = parser_generic(parser);
	if (!generic) {
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	node = generic;
	if (token_get_type(token) == TOKEN_COMMA) {
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
		
		generic_list = list_create();
		if(!generic_list){
			return NULL;
		}
		
		while (token_get_type(token) != TOKEN_GT) {
			generic = parser_generic(parser);
			if (!generic) {
				return NULL;
			}

			if(!list_rpush(generic_list, (list_value_t)generic)){
				return NULL;
			}

			if (token_get_type(token) != TOKEN_COMMA) {
				break;
			}
			if(!parser_match(parser, TOKEN_COMMA)){
				return NULL;
			}
		}
		node = node_make_generic_list(generic_list);
	}

	return node;
}

static node_t *
parser_fn_stmt(parser_t *parser)
{
	node_t *name;
	node_t *parameter_list;
	node_t *generic_list;
	node_t *return_type;
	node_t *block_stmt;
	
	int32_t is_overloading = 0;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_FN)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);

	name = NULL;
	
	if(parser->scope_type == PARSER_SCOPE_CLASS){
	
		switch(token_get_type(token)){
		case TOKEN_PLUS:
			if(!parser_match(parser, TOKEN_PLUS)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("+=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("+");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
		
		case TOKEN_MINUS:
			if(!parser_match(parser, TOKEN_MINUS)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("-=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("-");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_STAR:
			if(!parser_match(parser, TOKEN_STAR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("*=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("*");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_SLASH:
			if(!parser_match(parser, TOKEN_SLASH)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("/=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("/");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_PERCENT:
			if(!parser_match(parser, TOKEN_PERCENT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("%=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("%");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_AND:
			if(!parser_match(parser, TOKEN_AND)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_AND){
				if(!parser_match(parser, TOKEN_AND)){
					return NULL;
				}
				name = node_make_id("&&");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("&=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("&");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_OR:
			if(!parser_match(parser, TOKEN_OR)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_OR){
				if(!parser_match(parser, TOKEN_OR)){
					return NULL;
				}
				name = node_make_id("||");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("|=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("|");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_CARET:
			if(!parser_match(parser, TOKEN_CARET)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("^=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("^");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_TILDE:
			if(!parser_match(parser, TOKEN_TILDE)){
				return NULL;
			}
			name = node_make_id("~");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_LT:
			if(!parser_match(parser, TOKEN_LT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_LT){
				if(!parser_match(parser, TOKEN_LT)){
					return NULL;
				}
				
				if(token_get_type(token) == TOKEN_EQ){
					if(!parser_match(parser, TOKEN_EQ)){
						return NULL;
					}
					name = node_make_id("<<=");
					if(!(name)){
						return NULL;
					}
					is_overloading = 1;
					break;
				}
			
				name = node_make_id("<<");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("<=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("<");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_GT:
			if(!parser_match(parser, TOKEN_GT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_GT){
				if(!parser_match(parser, TOKEN_GT)){
					return NULL;
				}
				
				if(token_get_type(token) == TOKEN_EQ){
					if(!parser_match(parser, TOKEN_EQ)){
						return NULL;
					}
					name = node_make_id(">>=");
					if(!(name)){
						return NULL;
					}
					is_overloading = 1;
					break;
				}
			
				name = node_make_id(">>");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id(">=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id(">");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_NOT:
			if(!parser_match(parser, TOKEN_NOT)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("!=");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("!");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_EQ:
			if(!parser_match(parser, TOKEN_EQ)){
				return NULL;
			}
			
			if(token_get_type(token) == TOKEN_EQ){
				if(!parser_match(parser, TOKEN_EQ)){
					return NULL;
				}
				name = node_make_id("==");
				if(!(name)){
					return NULL;
				}
				is_overloading = 1;
				break;
			}
			
			name = node_make_id("=");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_LBRACKET:
			if(!parser_match(parser, TOKEN_LBRACKET)){
				return NULL;
			}
			if(!parser_match(parser, TOKEN_RBRACKET)){
				return NULL;
			}
			name = node_make_id("[]");
			if(!(name)){
				return NULL;
			}
			is_overloading = 1;
			break;
			
		case TOKEN_ID:
			name = parser_id(parser);
			if (!name) {
				return NULL;
			}
			break;
			
		default:
			name = NULL;
			break;
		}
	} else {
		if(token_get_type(token) == TOKEN_ID){
			name = parser_id(parser);
			if (!name) {
				return NULL;
			}
		}
	}
	
	generic_list = NULL;
	if(!is_overloading){
		if (token_get_type(token) == TOKEN_LT) {
			if(!parser_match(parser, TOKEN_LT)){
				return NULL;
			}
			if (token_get_type(token) != TOKEN_GT) {
				generic_list = parser_generic_list(parser);
				if (!generic_list) {
					return NULL;
				}
			}
			if(!parser_match(parser, TOKEN_GT)){
				return NULL;
			}
		}
	}

	parameter_list = NULL;
	if (token_get_type(token) == TOKEN_LPAREN) {
		if(!parser_match(parser, TOKEN_LPAREN)){
			return NULL;
		}		
		if (token_get_type(token) != TOKEN_RPAREN) {
			parameter_list = parser_parameter_list(parser);
			if (!parameter_list) {
				return NULL;
			}
		}
		if(!parser_match(parser, TOKEN_RPAREN)){
			return NULL;
		}
	}
	
	return_type = NULL;
	if(token_get_type(token) == TOKEN_COLON){
		if(!parser_match(parser, TOKEN_COLON)){
			return NULL;
		}
		return_type = parser_typing(parser);
		if(!return_type){
			return NULL;
		}
	}
	
	block_stmt = NULL;
	if (parser->scope_type == PARSER_SCOPE_MODULE){
		if (token_get_type(token) == TOKEN_LBRACE) {
			list_rpush(parser->scope_list, parser->scope_type);
			parser->scope_type = PARSER_SCOPE_FN;
			
			block_stmt = parser_block_stmt(parser);
			if (!block_stmt) {
				return NULL;
			}
			
			parser->scope_type = list_content(list_rpop(parser->scope_list));
		} else {
			if(!parser_match(parser, TOKEN_SEMICOLON)){
				return NULL;
			}
		}
	} else {
		list_rpush(parser->scope_list, parser->scope_type);
		parser->scope_type = PARSER_SCOPE_FN;
			
		block_stmt = parser_block_stmt(parser);
		if (!block_stmt) {
			return NULL;
		}
			
		parser->scope_type = list_content(list_rpop(parser->scope_list));
	}
	return node_make_fn(name, generic_list, parameter_list, return_type, block_stmt);
}

static node_t *
parser_enum_block_stmt(parser_t *parser)
{
	node_t *key;
	node_t *value;
	node_t *element;
	list_t *enum_list;
	
	lexer_t *lexer;
	token_t *token;
	
	enum_list = list_create();
	if(!enum_list){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	while(token_get_type(token) != TOKEN_RBRACE){
		key = parser_id(parser);
		if (!key) {
			return NULL;
		}
		
		value = NULL;
		if(token_get_type(token) == TOKEN_EQ){
			if(!parser_match(parser, TOKEN_EQ)){
				return NULL;
			}
	
			if(token_get_type(token) == TOKEN_LETTERS){
				value = parser_letters(parser);
				if (!value) {
					return NULL;
				}
			} else {
				value = parser_number(parser);
				if (!value) {
					return NULL;
				}
			}
		}
		
		element = node_make_element(key, value);
		if(!element){
			return NULL;
		}
		
		if(!list_rpush(enum_list, (list_value_t)element)){
			return NULL;
		}
		
		if(token_get_type(token) != TOKEN_COMMA){
			break;
		}
		
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}
	}
	
	return node_make_enum_block(enum_list);
}

static node_t *
parser_enum_stmt(parser_t *parser)
{
	node_t *name;
	node_t *block_stmt;

	if(!parser_match(parser, TOKEN_ENUM)){
		return NULL;
	}

	name = parser_id(parser);
	if (!name) {
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}
	
	list_rpush(parser->scope_list, parser->scope_type);
	parser->scope_type = PARSER_SCOPE_ENUM;
	
	block_stmt = parser_enum_block_stmt(parser);
	if(!block_stmt){
		return NULL;
	}
	
	parser->scope_type = list_content(list_rpop(parser->scope_list));
	
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}
		
	return node_make_enum(name, block_stmt);
}

static node_t *
parser_class_super_list(parser_t *parser)
{
	node_t *name;
	list_t *super_list;
	lexer_t *lexer;
	token_t *token;
	
	name = parser_postfix(parser);
	if (!name) {
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	super_list = list_create();
	if(!super_list){
		return NULL;
	}
	
	if(!list_rpush(super_list, (list_value_t)name)){
		return NULL;
	}
	
	while (token_get_type(token) != TOKEN_RPAREN) {
		if(!parser_match(parser, TOKEN_COMMA)){
			return NULL;
		}

		name = parser_postfix(parser);
		if (!name) {
			return NULL;
		}

		if(!list_rpush(super_list, (list_value_t)name)){
			return NULL;
		}
	}

	return node_make_super_list(super_list);
}

static node_t *
parser_class_block_stmt(parser_t *parser)
{
	node_t *stmt;
	list_t *block_list;
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	block_list = list_create();
	if(!block_list){
		return NULL;
	}
	
	while (token_get_type(token) != TOKEN_RBRACE) {
		if (token_get_type(token) == TOKEN_VAR) {
			stmt = parser_var_list_stmt(parser);
		} else if (token_get_type(token) == TOKEN_CLASS) {
			stmt = parser_class_stmt(parser);
		} else if (token_get_type(token) == TOKEN_ENUM) {
			stmt = parser_enum_stmt(parser);
		} else if (token_get_type(token) == TOKEN_FN) {
			stmt = parser_fn_stmt(parser);
		} else {
			parser_error(parser,
			             "class accepts only var, enum, function "
			             "and class declarations\n");

			return NULL;
		}

		if (!stmt) {
			return NULL;
		}

		if(!(list_rpush(block_list, (list_value_t)stmt))){
			return NULL;
		}
	}
	
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}
	
	return node_make_class_block(block_list);
}

static node_t *
parser_class_stmt(parser_t *parser)
{
	node_t *name;
	node_t *super_list;
	node_t *block_stmt;
	node_t *generic_list;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_CLASS)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);

	name = parser_id(parser);
	if (!name) {
		return NULL;
	}
	
	generic_list = NULL;
	if (token_get_type(token) == TOKEN_LT) {
		if(!parser_match(parser, TOKEN_LT)){
			return NULL;
		}
		if (token_get_type(token) != TOKEN_GT) {
			generic_list = parser_generic_list(parser);
			if (!generic_list) {
				return NULL;
			}
		}
		if(!parser_match(parser, TOKEN_GT)){
			return NULL;
		}
	}

	super_list = NULL;
	if (token_get_type(token) == TOKEN_LPAREN) {
		if(!parser_match(parser, TOKEN_LPAREN)){
			return NULL;
		}

		if (token_get_type(token) == TOKEN_ID) {
			super_list = parser_class_super_list(parser);
			if (!super_list) {
				return NULL;
			}
		}
		
		if(!parser_match(parser, TOKEN_RPAREN)){
			return NULL;
		}
	}
	
	list_rpush(parser->scope_list, parser->scope_type);
	parser->scope_type = PARSER_SCOPE_CLASS;

	block_stmt = parser_class_block_stmt(parser);
	if (!block_stmt) {
		return NULL;
	}
	
	parser->scope_type = list_content(list_rpop(parser->scope_list));

	return node_make_class(name, generic_list, super_list, block_stmt);
}

static node_t *
parser_block_stmt(parser_t *parser)
{
	node_t *stmt;
	list_t *list;
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_LBRACE)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	list = list_create();
	if(!list){
		return NULL;
	}
	
	while (token_get_type(token) != TOKEN_RBRACE) {
		/* empty stmt */
		if (token_get_type(token) == TOKEN_SEMICOLON) {
			if(!parser_match(parser, TOKEN_SEMICOLON)){
				return NULL;
			}

			continue;
		}

		stmt = parser_statement(parser);
		if (!stmt) {
			return NULL;
		}
		
		if(!list_rpush(list, (list_value_t)stmt)){
			return NULL;
		}
	}
	if(!parser_match(parser, TOKEN_RBRACE)){
		return NULL;
	}

	return node_make_block(list);
}

static node_t *
parser_statement(parser_t *parser)
{
	node_t *node;

	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	switch (token_get_type(token)) {
	case TOKEN_LBRACE:
		node = parser_block_stmt(parser);
		break;

	case TOKEN_IF:
		node = parser_if_stmt(parser);
		break;

	case TOKEN_WHILE:
		node = parser_while_stmt(parser);
		break;

	case TOKEN_VAR:
		node = parser_var_list_stmt(parser);
		break;

	case TOKEN_BREAK:
		node = parser_break_stmt(parser);
		break;

	case TOKEN_CONTINUE:
		node = parser_continue_stmt(parser);
		break;

	case TOKEN_RETURN:
		node = parser_return_stmt(parser);
		break;

	default:
		node = parser_assign_stmt(parser);
		if (!node) {
			break;
		}

		if (node->kind != NODE_KIND_CLASS && node->kind != NODE_KIND_FN)
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
parser_import_stmt(parser_t *parser)
{
	node_t *from;
	node_t *node;

	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_IMPORT)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	switch(token_get_type(token)){
	case TOKEN_LBRACE:	
		node = parser_directory(parser);
		if(!node){
			return NULL;
		}
		break;
	
	default:
		node = parser_id(parser);
		if(!node){
			return NULL;
		}
		break;
	}
	
	if(!parser_match(parser, TOKEN_FROM)){
		return NULL;
	}
	
	from = parser_letters(parser);
	if (!from) {
		return NULL;
	}

	return node_make_import(node, from);
}

static node_t *
parser_extern_stmt(parser_t *parser)
{
	node_t *stmt;
	
	lexer_t *lexer;
	token_t *token;
	
	if(!parser_match(parser, TOKEN_EXTERN)){
		return NULL;
	}
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	switch(token_get_type(token)){
	case TOKEN_VAR:
		stmt = parser_var_list_stmt(parser);
		break;
	
	case TOKEN_FN:
		stmt = parser_fn_stmt(parser);
		break;
		
	default:
		parser_error(parser,
			"extern accepts only var and function "
			"declarations\n");
		return NULL;
	}
	
	if (!stmt) {
		return NULL;
	}

	return node_make_extern(stmt);
}

node_t *
parser_module(parser_t *parser)
{
	node_t *stmt;
	list_t *stmt_list;
	
	lexer_t *lexer;
	token_t *token;
	
	lexer = parser->lexer;
	token = lexer_get_token(lexer);
	
	stmt_list = list_create();
	if(!(stmt_list)){
		return NULL;
	}
	
	if(!parser_match(parser, TOKEN_EOF)){
		return NULL;
	}
	
	list_rpush(parser->scope_list, parser->scope_type);
	parser->scope_type = PARSER_SCOPE_MODULE;
	
	while (token_get_type(token) != TOKEN_EOF) {
		/* empty stmt */
		if (token_get_type(token) == TOKEN_SEMICOLON) {
			if(!parser_match(parser, TOKEN_SEMICOLON)){
				return NULL;
			}

			continue;
		}
		
		switch(token_get_type(token)){
		case TOKEN_EXTERN:
			stmt = parser_extern_stmt(parser);
			break;
			
		case TOKEN_IMPORT:
			stmt = parser_import_stmt(parser);
			break;
			
		case TOKEN_CLASS:
			stmt = parser_class_stmt(parser);
			break;
			
		case TOKEN_ENUM:
			stmt = parser_enum_stmt(parser);
			break;
			
		case TOKEN_FN:
			stmt = parser_fn_stmt(parser);
			break;
			
		case TOKEN_VAR:
			stmt = parser_var_list_stmt(parser);
			break;
			
		default:
			stmt = parser_assign_stmt(parser);
			if (!stmt) {
				return NULL;
			}
			if (stmt->kind != NODE_KIND_CLASS && stmt->kind != NODE_KIND_FN && stmt->kind != NODE_KIND_ENUM)
			{
				if(!parser_match(parser, TOKEN_SEMICOLON)){
					return NULL;
				}
			}
			break;
		}

		if (!stmt) {
			return NULL;
		}

		if(!list_rpush(stmt_list, (list_value_t)stmt)){
			return NULL;
		}
	}
	
	parser->scope_type = list_content(list_rpop(parser->scope_list));
	
	if(!parser_match(parser, TOKEN_EOF)){
		return NULL;
	}

	return node_make_module(stmt_list);
}

parser_t *
parser_create(lexer_t *lexer){
	parser_t *parser;
	if(!(parser = (parser_t *)malloc(sizeof(parser_t)))){
		fprintf(stderr, "unable to allocted a block of %zu bytes", sizeof(parser_t));
		return NULL;
	}
	memset(parser, 0, sizeof(parser_t));
	parser->lexer = lexer;
	parser->loop_depth = 0;
	
	parser->scope_list = list_create();
	if(!parser->scope_list){
		return NULL;
	}
	
	parser->scope_type = 0;
	
	return parser;
}
