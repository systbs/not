#include <stdint.h>

#include "../types/types.h"
#include "../token/position.h"
#include "../token/token.h"

// name of every token
const char * const symbols[] = {
  [TOKEN_EOF]       = "EOF",
  [TOKEN_SPACE]     = "SPACE",
  [TOKEN_QUTA]      = "\"",
  [TOKEN_PRIME]     = "'",
  [TOKEN_UPRIME]    = "`",
  [TOKEN_HASH]      = "#",	
  [TOKEN_DOLLER]    = "$",
  [TOKEN_LPAREN]    = "(",	
  [TOKEN_RPAREN]    = ")",
  [TOKEN_LBRACKET]  = "[", 
  [TOKEN_RBRACKET]  = "]",
  [TOKEN_LBRACE]    = "{",
  [TOKEN_RBRACE]    = "}",
  [TOKEN_COMMA]     = ",",	
  [TOKEN_DOT]       = ".",
  [TOKEN_COLON]     = ":",	
  [TOKEN_SEMICOLON] = ";",
  [TOKEN_QUESTION]  = "?", 
  [TOKEN_AT]        = "@",  
  [TOKEN_BACKSLASH] = "\\",
  [TOKEN_UNDERLINE] = "_",


  [TOKEN_PLUS]      = "+",	
  [TOKEN_MINUS]     = "-",
  [TOKEN_STAR]      = "*",
  [TOKEN_SLASH]     = "/",
  [TOKEN_PERCENT]   = "%",

  [TOKEN_AND]       = "&", 	
  [TOKEN_OR]        = "|",
  [TOKEN_CARET]     = "^",
  [TOKEN_TILDE]     = "~",
  [TOKEN_LT_LT]     = "<<",
  [TOKEN_GT_GT]     = ">>",

  [TOKEN_LT]        = "<",
  [TOKEN_GT]        = ">",
  [TOKEN_LT_EQ]     = "<=",
  [TOKEN_GT_EQ]     = ">=",
  [TOKEN_EQ_EQ]     = "==",
  [TOKEN_NOT_EQ]    = "!=",

  [TOKEN_NOT]       = "!",	
  [TOKEN_AND_AND]   = "&&",  
  [TOKEN_OR_OR]     = "||",  
  
  [TOKEN_EQ]        = "=",	
  [TOKEN_PLUS_EQ]   = "+=",	 
  [TOKEN_MINUS_EQ]  = "-=",
  [TOKEN_STAR_EQ]   = "*=",
  [TOKEN_SLASH_EQ]  = "/=",	 
  [TOKEN_PERCENT_EQ]= "%=",
  [TOKEN_AND_EQ]    = "&=",	
  [TOKEN_OR_EQ]     = "|=",	
  [TOKEN_LT_LT_EQ]  = "<<=",	
  [TOKEN_GT_GT_EQ]  = ">>=", 

  [TOKEN_DOT_DOT]   = "..", 
  [TOKEN_COLON_EQ]  = ":=", 
  [TOKEN_MINUS_GT]  = "->", 

  [TOKEN_ID]        = "identifier",
  [TOKEN_NUMBER]    = "number",
  [TOKEN_CHAR]      = "char",
  [TOKEN_STRING]	  = "string",

  [TOKEN_CONTINUE_KEYWORD]  = "continue",
  [TOKEN_BREAK_KEYWORD]     = "break",
  [TOKEN_ELSE_KEYWORD]      = "else",
  [TOKEN_IF_KEYWORD]        = "if",
  [TOKEN_TRY_KEYWORD]       = "try",
  [TOKEN_CATCH_KEYWORD]     = "catch",
  [TOKEN_THROW_KEYWORD]     = "throw",
  [TOKEN_RETURN_KEYWORD]    = "return",
  [TOKEN_TRUE_KEYWORD]      = "true",
  [TOKEN_FALSE_KEYWORD]     = "false",
  [TOKEN_FOR_KEYWORD]       = "for",
  [TOKEN_IN_KEYWORD]        = "in",
  [TOKEN_SIZEOF_KEYWORD]    = "sizeof",
  [TOKEN_TYPEOF_KEYWORD]    = "typeof",
  [TOKEN_NULL_KEYWORD]      = "null",
  [TOKEN_FUNC_KEYWORD]      = "func",
  [TOKEN_CLASS_KEYWORD]     = "class",
  [TOKEN_EXTENDS_KEYWORD]   = "extends",
  [TOKEN_STATIC_KEYWORD]    = "static",
  [TOKEN_READONLY_KEYWORD]  = "readonly",
  [TOKEN_VAR_KEYWORD]     	= "var",
  [TOKEN_CONST_KEYWORD]     = "const",
  [TOKEN_TYPE_KEYWORD]     	= "type",
  [TOKEN_ENUM_KEYWORD]     	= "enum",
  [TOKEN_EXPORT_KEYWORD]    = "export"
};

const char *
token_get_name(int32_t tp){
    return symbols[tp];
}