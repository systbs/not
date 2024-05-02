#pragma once

#ifndef __TOKEN_H__
#define __TOKEN_H__

// tokens
enum
{
  TOKEN_EOF = 0,
  TOKEN_SPACE,
  TOKEN_QUTA,	 	    //	"
  TOKEN_PRIME,	 	  //	'
  TOKEN_UPRIME,		  //	`
  TOKEN_HASH,	 	    //	#
  TOKEN_DOLLER,		  //	$
  TOKEN_LPAREN,		  //	(
  TOKEN_RPAREN,		  //	)
  TOKEN_LBRACKET, 	//	[
  TOKEN_RBRACKET, 	//	]
  TOKEN_LBRACE,		  //	{
  TOKEN_RBRACE,		  //	}
  TOKEN_COMMA,	 	  //	,
  TOKEN_DOT,	   	  //	.
  TOKEN_COLON,	 	  //	:
  TOKEN_SEMICOLON,	//	;
  TOKEN_QUESTION, 	//	?
  TOKEN_AT,  	 	    //	@
  TOKEN_UNDERLINE,	//	_

  // Binary Arithmetic 
  TOKEN_PLUS,	 	  //	+
  TOKEN_MINUS,	 	//	-
  TOKEN_STAR,	 	  //	*
  TOKEN_POWER,	 	//	**
  TOKEN_SLASH,	 	//	/
  TOKEN_BACKSLASH, 
  TOKEN_PERCENT,	//	%

  // Bit-Wise
  TOKEN_AND, 	 	  // &
  TOKEN_OR,			  // |
  TOKEN_CARET,		// ^
  TOKEN_TILDE,		// ~
  TOKEN_LT_LT,    // <<
  TOKEN_GT_GT,    // >>

  // Relational
  TOKEN_LT,	   		//	<
  TOKEN_GT,	   		//	>
  TOKEN_LT_EQ,    // <=
  TOKEN_GT_EQ,    // >=
  TOKEN_EQ_EQ,    // ==
  TOKEN_NOT_EQ,   // !=

  // Logical
  TOKEN_NOT,	   	//	!
  TOKEN_AND_AND,  // &&
  TOKEN_OR_OR,    // ||

  // Assignment
  TOKEN_EQ,	   		    //	=
  TOKEN_PLUS_EQ,      // +=
  TOKEN_MINUS_EQ,     // -=
  TOKEN_STAR_EQ,      // *=
  TOKEN_SLASH_EQ,     // /=
  TOKEN_BACKSLASH_EQ, // \=
  TOKEN_PERCENT_EQ,   // %=
  TOKEN_AND_EQ,       // &=
  TOKEN_OR_EQ,        // |=
  TOKEN_LT_LT_EQ,     // <<=
  TOKEN_GT_GT_EQ,     // >>=

  TOKEN_MINUS_GT,     // ->

  TOKEN_ID,
  TOKEN_NUMBER,
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_FSTRING,

  TOKEN_CONTINUE_KEYWORD,
  TOKEN_BREAK_KEYWORD,
  TOKEN_ELSE_KEYWORD,
  TOKEN_IF_KEYWORD,
  TOKEN_TRY_KEYWORD,
  TOKEN_CATCH_KEYWORD,
  TOKEN_THROW_KEYWORD,
  TOKEN_RETURN_KEYWORD,
  TOKEN_FOR_KEYWORD,
  TOKEN_IN_KEYWORD,
  TOKEN_SIZEOF_KEYWORD,
  TOKEN_TYPEOF_KEYWORD,
  TOKEN_NULL_KEYWORD,
  TOKEN_FUN_KEYWORD,
  TOKEN_CLASS_KEYWORD,
  TOKEN_EXTENDS_KEYWORD,
  TOKEN_STATIC_KEYWORD,
  TOKEN_READONLY_KEYWORD,
  TOKEN_REFERENCE_KEYWORD,
  TOKEN_OVERRIDE_KEYWORD,
  TOKEN_THIS_KEYWORD,
  TOKEN_SELF_KEYWORD,
  TOKEN_VAR_KEYWORD,
  TOKEN_EXPORT_KEYWORD,
  TOKEN_USING_KEYWORD,
  TOKEN_FROM_KEYWORD
};


typedef struct token {
	position_t position;
	int32_t type;
	char *value;
} token_t;

const char *
token_get_name(int32_t tp);

#endif //__TOKEN_H__
