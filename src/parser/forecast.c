#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../container/list.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "parser.h"
#include "forecast.h"

static int32_t
forecast_func(parser_t *parser);

static int32_t
forecast_expression(parser_t *parser);

static int32_t
forecast_expression(parser_t *parser);

static int32_t
forecast_prefix(parser_t *parser);

static int32_t
forecast_bitwise_or(parser_t *parser);

static int32_t
forecast_logical_and(parser_t *parser);

static int32_t
forecast_parameters(parser_t *parser);

static int32_t
forecast_type_parameters(parser_t *parser);

static int32_t
forecast_match(parser_t *parser, int32_t type)
{
	if(parser->token->type == type){
		if(!scanner_advance(parser->scanner)){
			return 0;
		} 
	} else {
		return 0;
	}
	return 1;
}



static int32_t
forecast_type_arguments(parser_t *parser)
{
	while (true) {
		if (!forecast_expression(parser)) {
			return 0;
		}

		if (parser->token->type != TOKEN_COMMA) {
			break;
		}
		
		if(!forecast_match(parser, TOKEN_COMMA)){
			return 0;
		}
	}
	
	return 1;
}

static int32_t
forecast_id(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_ID)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_number(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_NUMBER)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_char(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_CHAR)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_string(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_STRING)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_null(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_NULL_KEYWORD)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_true(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_TRUE_KEYWORD)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_false(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_FALSE_KEYWORD)){
		return 0;
	}
	return 1;
}

static int32_t
forecast_array(parser_t *parser)
{	
	if(!forecast_match(parser, TOKEN_LBRACKET))
    {
		return 0;
	}

	while (parser->token->type != TOKEN_RBRACKET) 
    {
		if (!forecast_expression(parser)) {
			return 0;
		}

		if (parser->token->type != TOKEN_COMMA) {
			break;
		}
		if(!forecast_match(parser, TOKEN_COMMA)){
			return 0;
		}
	}

	if(!forecast_match(parser, TOKEN_RBRACKET))
    {
		return 0;
	}

	return 1;
}

static int32_t
forecast_object(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_LBRACE)){
		return 0;
	}
	
	while (parser->token->type != TOKEN_RBRACE) {
		int32_t use_string = 0;
		switch (parser->token->type){	
		case TOKEN_STRING:
			if(!forecast_string(parser)){
				return 0;
			}
			use_string = 1;
			break;

		case TOKEN_LBRACE:
			if(!forecast_object(parser)){
				return 0;
			}
			break;
		
		default:
			if(!forecast_id(parser)){
				return 0;
			}
			break;
		}

		if(parser->token->type == TOKEN_COLON || use_string){
			if(!forecast_match(parser, TOKEN_COLON)){
				return 0;
			}

			if (!forecast_expression(parser)) {
				return 0;
			}
		}
		

		if (parser->token->type != TOKEN_COMMA) {
			break;
		}
		if(!forecast_match(parser, TOKEN_COMMA)){
			return 0;
		}
	}
	if(!forecast_match(parser, TOKEN_RBRACE)){
		return 0;
	}

	return 1;
}

static int32_t
forecast_primary(parser_t *parser)
{
	int32_t test = 1;
	switch (parser->token->type){
	case TOKEN_ID:
		test = forecast_id(parser);
		break;
	
	case TOKEN_NUMBER:
		test = forecast_number(parser);
		break;
	
	case TOKEN_CHAR:
		test = forecast_char(parser);
		break;

	case TOKEN_STRING:
		test = forecast_string(parser);
		break;
	
	case TOKEN_NULL_KEYWORD:
		test = forecast_null(parser);
		break;

	case TOKEN_TRUE_KEYWORD:
		test = forecast_true(parser);
		break;

	case TOKEN_FALSE_KEYWORD:
		test = forecast_false(parser);
		break;
		
	case TOKEN_LBRACKET:
		test = forecast_array(parser);
		break;
		
	case TOKEN_LBRACE:
		test = forecast_object(parser);
		break;

	case TOKEN_FUNC_KEYWORD:
		test = forecast_func(parser);
		break;
		
	case TOKEN_LPAREN:
		if(!forecast_match(parser, TOKEN_LPAREN)){
			return 0;
		}
		test = forecast_expression(parser);
		if(!test){
			return 0;
		}
		if(!forecast_match(parser, TOKEN_RPAREN)){
			return 0;
		}
		break;
		
	default:
		return 0;
	}
	
	return test;
}



static int32_t
forecast_arguments(parser_t *parser)
{

	while(true)
	{
		if(!(forecast_expression(parser)))
		{
			return 0;
		}

		if(parser->token->type != TOKEN_COMMA)
		{
			break;
		}

		if(!forecast_match(parser, TOKEN_COMMA))
		{
			return 0;
		}
	}
	
	return 1;
}

static int32_t
forecast_postfix(parser_t *parser)
{
	int32_t test;
	test = forecast_primary(parser);
	if(!(test)){
		return 0;
	}
	
	int32_t use_composite = 0;
	int32_t slice;
	while(test)
	{
		switch(parser->token->type)
		{
		case TOKEN_LT:
			if(use_composite)
			{
				return 1;
			}
			if(!forecast_is_type_argument(parser))
			{
				return 1;
			}

			if(!forecast_match(parser, TOKEN_LT))
			{
				return 0;
			}
			
			if(parser->token->type != TOKEN_GT)
			{
				if(!(forecast_type_arguments(parser)))
				{
					return 0;
				}
			}

			if(!forecast_match(parser, TOKEN_GT))
			{
				return 0;
			}
			use_composite = 1;
			test = 1;
			break;

		case TOKEN_LPAREN:
			if(!forecast_match(parser, TOKEN_LPAREN))
			{
				return 0;
			}
			
			if(parser->token->type != TOKEN_RPAREN)
			{
				if(!forecast_arguments(parser))
				{
					return 0;
				}
			}
			
			if(!forecast_match(parser, TOKEN_RPAREN))
			{
				return 0;
			}
			test = 1;
			break;
		
		case TOKEN_LBRACKET:
			if(!forecast_match(parser, TOKEN_LBRACKET))
			{
				return 0;
			}

			if (parser->token->type == TOKEN_COLON)
			{
				if(!forecast_match(parser, TOKEN_COLON))
				{
					return 0;
				}
				slice = 1;
			}
			else
			{
				if (!forecast_expression(parser))
				{
					return 0;
				}
				if (parser->token->type == TOKEN_COLON)
				{
					if(!forecast_match(parser, TOKEN_COLON))
					{
						return 0;
					}
					slice = 1;
				}
			}
			
			if (parser->token->type == TOKEN_COLON)
			{
				if(!forecast_match(parser, TOKEN_COLON))
				{
					return 0;
				}
			} else if (slice)
			{
				if (parser->token->type != TOKEN_RBRACKET)
				{
					if (!forecast_expression(parser))
					{
						return 0;
					}
				}
				if (parser->token->type == TOKEN_COLON)
				{
					if(!forecast_match(parser, TOKEN_COLON))
					{
						return 0;
					}
				}
			}
			
			if (slice)
			{
				if (parser->token->type != TOKEN_RBRACKET)
				{
					if (!forecast_expression(parser))
					{
						return 0;
					}
				}
				if(!forecast_match(parser, TOKEN_RBRACKET))
				{
					return 0;
				}

				test = 1;
			}
			else
			{
				if(!forecast_match(parser, TOKEN_RBRACKET))
				{
					return 0;
				}
				test = 1;
			}
			break;
			
		case TOKEN_DOT:
			if(!forecast_match(parser, TOKEN_DOT))
			{
				return 0;
			}
			
			if (!forecast_id(parser))
			{
				return 0;
			}
			
			test = 1;
			break;

		default:
			return test;
			
		}
	}
	
	return test;
}

static int32_t
forecast_prefix(parser_t *parser)
{
	int32_t test = 1;
	switch (parser->token->type)
	{
	case TOKEN_LPAREN:
		if (!forecast_is_casting(parser))
		{
			test = forecast_postfix(parser);
			break;
		}
		if(!forecast_match(parser, TOKEN_LPAREN))
		{
			return 0;
		}
		if (!forecast_expression(parser))
		{
			return 0;
		}
		if(!forecast_match(parser, TOKEN_RPAREN))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_TILDE:
		if(!forecast_match(parser, TOKEN_TILDE))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_NOT:
		if(!forecast_match(parser, TOKEN_NOT))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_MINUS:
		if(!forecast_match(parser, TOKEN_MINUS))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_PLUS:
		if(!forecast_match(parser, TOKEN_PLUS))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;
		
	case TOKEN_STAR:
		if(!forecast_match(parser, TOKEN_STAR))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;
		
	case TOKEN_AND:
		if(!forecast_match(parser, TOKEN_AND))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_TYPEOF_KEYWORD:
		if(!forecast_match(parser, TOKEN_TYPEOF_KEYWORD))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_SIZEOF_KEYWORD:
		if(!forecast_match(parser, TOKEN_SIZEOF_KEYWORD))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_AWAIT_KEYWORD:
		if(!forecast_match(parser, TOKEN_AWAIT_KEYWORD))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	case TOKEN_DOT:
		if(!forecast_match(parser, TOKEN_DOT))
		{
			return 0;
		}
		if(!forecast_match(parser, TOKEN_DOT))
		{
			return 0;
		}
		if(!forecast_match(parser, TOKEN_DOT))
		{
			return 0;
		}
		if (!forecast_prefix(parser))
		{
			return 0;
		}
		test = 1;
		break;

	default:
		test = forecast_postfix(parser);
	}

	return test;
}

static int32_t
forecast_multiplicative(parser_t *parser)
{
	int32_t test;
	if(!(test = forecast_prefix(parser))){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_STAR:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_STAR)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!(forecast_prefix(parser))) {
				return 0;
			}
			
			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		case TOKEN_SLASH:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_SLASH)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!forecast_prefix(parser)) {
				return 0;
			}

			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		case TOKEN_PERCENT:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_PERCENT)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!forecast_prefix(parser)) {
				return 0;
			}
			
			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_addative(parser_t *parser)
{
	int32_t test;
    test = forecast_multiplicative(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_PLUS:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_PLUS)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!forecast_multiplicative(parser)) {
				return 0;
			}
			
			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		case TOKEN_MINUS:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_MINUS)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!forecast_multiplicative(parser)) {
				return 0;
			}
			
			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_shifting(parser_t *parser)
{
	int32_t test;
    test = forecast_addative(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_LT:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_LT)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_LT){
				if(!forecast_match(parser, TOKEN_LT)){
					return 0;
				}
				
				if(parser->token->type == TOKEN_EQ){
					if(!parser_restore_state(parser)){
						return 0;
					}
					return test;
				}
				
				if (!forecast_addative(parser)) {
					return 0;
				}
				
				if(!parser_release_state(parser)){
					return 0;
				}
				test = 1;
				break;
			}
			
			if(!parser_restore_state(parser)){
				return 0;
			}
			return test;

		case TOKEN_GT:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_GT)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_GT){
				if(!forecast_match(parser, TOKEN_GT)){
					return 0;
				}
				
				if(parser->token->type == TOKEN_EQ){
					if(!parser_restore_state(parser)){
						return 0;
					}
					return test;
				}
			
				if (!forecast_addative(parser)) {
					return 0;
				}
				
				if(!parser_release_state(parser)){
					return 0;
				}
				test = 1;
				break;
			}
			
			if(!parser_restore_state(parser)){
				return 0;
			}
			return test;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_relational(parser_t *parser)
{
	int32_t test;
    test = forecast_shifting(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_LT:
			if(!forecast_match(parser, TOKEN_LT)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!forecast_match(parser, TOKEN_EQ)){
					return 0;
				}
				
				if (!forecast_shifting(parser)) {
					return 0;
				}
				
				test = 1;
				break;
			}
			
			if (!forecast_shifting(parser)) {
				return 0;
			}
			
			test = 1;
			break;

		case TOKEN_GT:
			if(!forecast_match(parser, TOKEN_GT)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!forecast_match(parser, TOKEN_EQ)){
					return 0;
				}
				
				if (!forecast_shifting(parser)) {
					return 0;
				}
				
				test = 1;
				break;
			}
			
			if (!forecast_shifting(parser)) {
				return 0;
			}
			
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_equality(parser_t *parser)
{
	int32_t test;
	test = forecast_relational(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_EQ:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_EQ)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!forecast_match(parser, TOKEN_EQ)){
					return 0;
				}
				
				if (!forecast_relational(parser)) {
					return 0;
				}
				
				if(!parser_release_state(parser)){
					return 0;
				}
				test = 1;
				break;
			}
			
			if(!parser_restore_state(parser)){
				return 0;
			}
			return test;

		case TOKEN_NOT:
			if(!forecast_match(parser, TOKEN_NOT)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!forecast_match(parser, TOKEN_EQ)){
					return 0;
				}
			
				if (!forecast_relational(parser)) {
					return 0;
				}
				
				test = 1;
				break;
			}
			
			return 0;

		case TOKEN_IN_KEYWORD:
			if(!forecast_match(parser, TOKEN_IN_KEYWORD)){
				return 0;
			}
			
			if (!forecast_relational(parser)) {
				return 0;
			}
			
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_bitwise_and(parser_t *parser)
{
	int32_t test;
    test = forecast_equality(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_AND:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_AND)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_AND){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!forecast_equality(parser)) {
				return 0;
			}
			
			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_bitwise_xor(parser_t *parser)
{
	int32_t test;
    test = forecast_bitwise_and(parser);
	if(!test){
		return 0;
	}

	while (test) {
		switch (parser->token->type) {
		case TOKEN_CARET:
			if(!forecast_match(parser, TOKEN_CARET)){
				return 0;
			}
			
			if (!forecast_bitwise_and(parser)) {
				return 0;
			}
			
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_bitwise_or(parser_t *parser)
{
	int32_t test;
    test = forecast_bitwise_xor(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_OR:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_OR)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_OR){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if(parser->token->type == TOKEN_EQ){
				if(!parser_restore_state(parser)){
					return 0;
				}
				return test;
			}
			
			if (!forecast_bitwise_xor(parser)) {
				return 0;
			}
			
			if(!parser_release_state(parser)){
				return 0;
			}
			test = 1;
			break;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_logical_and(parser_t *parser)
{
	int32_t test;
    test = forecast_bitwise_or(parser);
	if(!test){
		return 0;
	}

	while (test) {
		switch (parser->token->type) {
		case TOKEN_AND:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_AND)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_AND){
				if(!forecast_match(parser, TOKEN_AND)){
					return 0;
				}
				
				if (!forecast_bitwise_or(parser)) {
					return 0;
				}
				
				if(!parser_release_state(parser)){
					return 0;
				}
				test = 1;
				break;
			}
			
			if(!parser_restore_state(parser)){
				return 0;
			}
			return test;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_logical_or(parser_t *parser)
{
	int32_t test;
    test = forecast_logical_and(parser);
	if(!test){
		return 0;
	}
	
	while (test) {
		switch (parser->token->type) {
		case TOKEN_OR:
			if(!parser_save_state(parser)){
				return 0;
			}
			if(!forecast_match(parser, TOKEN_OR)){
				return 0;
			}
			
			if(parser->token->type == TOKEN_OR){
				if(!forecast_match(parser, TOKEN_OR)){
					return 0;
				}
				
				if (!forecast_logical_and(parser)) {
					return 0;
				}
				
				if(!parser_release_state(parser)){
					return 0;
				}
				test = 1;
				break;
			}
			
			if(!parser_restore_state(parser)){
				return 0;
			}
			return test;

		default:
			return test;
		}
	}
	
	return test;
}

static int32_t
forecast_conditional(parser_t *parser)
{
	int32_t test;
    test = forecast_logical_or(parser);
	if(!test){
		return 0;
	}
	
	if (test) {
		switch (parser->token->type) {
		case TOKEN_QUESTION:
			if(!forecast_match(parser, TOKEN_QUESTION)){
				return 0;
			}
			
			if (parser->token->type != TOKEN_COLON) {
				if (!forecast_logical_or(parser)) {
					break;
				}
			}
			if(!forecast_match(parser, TOKEN_COLON)){
				return 0;
			}
			
			if (! forecast_expression(parser)) {
				break;
			}

			test = 1;
			break;

		default:
			return test;
		}
	}

	return test;
}

static int32_t
forecast_expression(parser_t *parser)
{
	return forecast_conditional(parser);
}



static int32_t
forecast_parameter(parser_t *parser)
{
	if(!forecast_id(parser)){
        return 0;
    }
	
	if (parser->token->type == TOKEN_COLON) {
		if(!forecast_match(parser, TOKEN_COLON)){
			return 0;
		}
		if (!forecast_expression(parser)) {
			return 0;
		}
	}
	
	if (parser->token->type == TOKEN_EQ) {
		if(!forecast_match(parser, TOKEN_EQ)){
			return 0;
		}
		if (!forecast_expression(parser)) {
			return 0;
		}
	}

	return 1;
}

static int32_t
forecast_parameters(parser_t *parser)
{
	while (parser->token->type != TOKEN_RPAREN) {
		if (!forecast_parameter(parser)) {
			return 0;
		}
		if (parser->token->type != TOKEN_COMMA) {
			break;
		}
		if(!forecast_match(parser, TOKEN_COMMA)){
			return 0;
		}
	}

	return 1;
}

static int32_t
forecast_type_parameter(parser_t *parser)
{
    if(!forecast_id(parser)){
		return 0;
	}
	
	if (parser->token->type == TOKEN_EQ) {
		if(!forecast_match(parser, TOKEN_EQ)){
			return 0;
		}
		if (!forecast_expression(parser)) {
			return 0;
		}
	}

	return 1;
}

static int32_t
forecast_type_parameters(parser_t *parser)
{
	while (parser->token->type != TOKEN_GT) {
		if (!forecast_type_parameter(parser)) {
			return 0;
		}
		if (parser->token->type != TOKEN_COMMA) {
			break;
		}
		if(!forecast_match(parser, TOKEN_COMMA)){
			return 0;
		}
	}

	return 1;
}

static int32_t
forecast_func(parser_t *parser)
{
	if(!forecast_match(parser, TOKEN_FUNC_KEYWORD)){
		return 0;
	}

	if(parser->token->type == TOKEN_ID){
		if (!forecast_id(parser)) {
			return 0;
		}
	}
	
	if (parser->token->type == TOKEN_LT) {
		if(!forecast_match(parser, TOKEN_LT)){
			return 0;
		}
		if (parser->token->type != TOKEN_GT) {
			if (!forecast_type_parameters(parser)) {
				return 0;
			}
		}
		if(!forecast_match(parser, TOKEN_GT)){
			return 0;
		}
	}

	if (parser->token->type == TOKEN_LPAREN) {
		if(!forecast_match(parser, TOKEN_LPAREN)){
			return 0;
		}		
		if (parser->token->type != TOKEN_RPAREN) {
			if (!forecast_parameters(parser)) {
				return 0;
			}
		}
		if(!forecast_match(parser, TOKEN_RPAREN)){
			return 0;
		}
	}
	
	if(parser->token->type == TOKEN_COLON){
		if(!forecast_match(parser, TOKEN_COLON)){
			return 0;
		}
		if(!forecast_expression(parser)){
			return 0;
		}
	}
	
	if (parser->token->type == TOKEN_LBRACE) {
        if(!forecast_match(parser, TOKEN_LBRACE)){
            return 0;
        }
        int32_t depth = 0;
		while(parser->token->type != TOKEN_RBRACE){
            if(parser->token->type == TOKEN_LBRACE){
                depth++;
            }
            if(parser->token->type == TOKEN_RBRACE){
                depth--;
                if(depth <= 0){
                    break;
                }
            }
            if(!forecast_match(parser, parser->token->type)){
                return 0;
            }
        }
        if(!forecast_match(parser, TOKEN_RBRACE)){
            return 0;
        }
	} else {
		if(parser->token->type == TOKEN_SEMICOLON){
			if(!forecast_match(parser, TOKEN_SEMICOLON)){
				return 0;
			}
		}
	}
	
	return 1;
}

int32_t
forecast_is_type_argument(parser_t *parser)
{
    if(!parser_save_state(parser)){
		return 0;
	}

    int32_t test = 1;
    if(!forecast_match(parser, TOKEN_LT)){
		test = 0;
        goto end_test;
	}

    if(parser->token->type != TOKEN_GT){
        if(!forecast_type_arguments(parser)){
            test = 0;
            goto end_test;
        }
    }

    if(!forecast_match(parser, TOKEN_GT)){
		test = 0;
        goto end_test;
	}

    end_test:
    if(!parser_restore_state(parser)){
		return 0;
	}

    return test;
}

int32_t
forecast_is_object(parser_t *parser)
{
    if(!parser_save_state(parser)){
		return 0;
	}

    int32_t test = 1;

    if(!forecast_object(parser)){
		test = 0;
		goto end_test;
	}

    end_test:
    if(!parser_restore_state(parser)){
		return 0;
	}

    return test;
}

int32_t
forecast_is_casting(parser_t *parser)
{
    if(!parser_save_state(parser)){
		return 0;
	}

    int32_t test = 1;

	if(!forecast_match(parser, TOKEN_LPAREN)){
		return 0;
	}
    if(!forecast_expression(parser)){
		test = 0;
		goto end_test;
	}
	if(!forecast_match(parser, TOKEN_RPAREN)){
		return 0;
	}

	if(!forecast_prefix(parser)){
		test = 0;
		goto end_test;
	}

    end_test:
    if(!parser_restore_state(parser)){
		return 0;
	}

    return test;
}