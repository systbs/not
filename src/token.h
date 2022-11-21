#pragma once

#ifndef __TOKEN_H__
#define __TOKEN_H__

// tokens
enum
{
  TOKEN_EOF = 0,
  TOKEN_SPACE,
  TOKEN_QUTA,	 	//	"
  TOKEN_HASH,	 	//	#
  TOKEN_DOLLER,		//	$
  TOKEN_PRIME,	 	//	'
  TOKEN_LPAREN,		//	(
  TOKEN_RPAREN,		//	)
  TOKEN_COMMA,	 	//	,
  TOKEN_DOT,	   	//	.
  TOKEN_COLON,	 	//	:
  TOKEN_SEMICOLON,	//	;
  TOKEN_QUESTION, 	//	?
  TOKEN_AT,  	 	//	@
  TOKEN_LBRACKET, 	//	[
  TOKEN_BACKSLASH,	//	
  TOKEN_RBRACKET, 	//	]
  TOKEN_UNDERLINE,	//	_
  TOKEN_UPRIME,		//	`
  TOKEN_LBRACE,		//	{
  TOKEN_RBRACE,		//	}
  // Binary Arithmetic 
  TOKEN_PLUS,	 	//	+
  TOKEN_MINUS,	 	//	-
  TOKEN_STAR,	 	//	*
  TOKEN_SLASH,	 	//	/
  TOKEN_PERCENT,	//	%
  // Bit-Wise	&,|,^,~,<<,>>
  TOKEN_AND, 	 	// &
  TOKEN_OR,			// |
  TOKEN_CARET,		// ^
  TOKEN_TILDE,		// ~
  // Relational <,>,<=,>=,==,!=
  TOKEN_LT,	   		//	<
  TOKEN_GT,	   		//	>
  // Logical &&,||,!
  TOKEN_NOT,	   	//	!
  // Assignment =,+=,-=,*=,/=,%=,&=,|=,<<=,>>=
  TOKEN_EQ,	   		//	=

  TOKEN_CONTINUE,
  TOKEN_BREAK,
  TOKEN_ELSE,
  TOKEN_IF,
  TOKEN_IMPORT,
  TOKEN_FROM,
  TOKEN_AS,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_ID, 
  TOKEN_WHILE,
  TOKEN_SIZEOF,
  TOKEN_TYPEOF,
  TOKEN_NULL,
  TOKEN_FN,
  TOKEN_CLASS,
  TOKEN_VAR,
  TOKEN_ENUM,
  TOKEN_EXTERN,

  TOKEN_NUMBER,
  TOKEN_LETTERS,
  
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_INT,
  TOKEN_LONG,
  TOKEN_FLOAT,
  TOKEN_DOUBLE,
  TOKEN_ANY
};

const char *
token_get_name(int32_t tp);

typedef struct token {
	uint64_t position;
	uint64_t line;
	uint64_t column;
	int32_t type;
	char *value;
} token_t;

uint64_t
token_get_line(token_t *token);

uint64_t
token_get_column(token_t *token);

uint64_t
token_get_position(token_t *token);

int32_t
token_get_type(token_t *token);

char *
token_get_value(token_t *token);
#endif //__TOKEN_H__
