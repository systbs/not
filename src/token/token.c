//#include <stdint.h>

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
  [TOKEN_BACKSLASH_EQ]  = "\\=",	
  [TOKEN_PERCENT_EQ]= "%=",
  [TOKEN_AND_EQ]    = "&=",	
  [TOKEN_OR_EQ]     = "|=",	
  [TOKEN_LT_LT_EQ]  = "<<=",	
  [TOKEN_GT_GT_EQ]  = ">>=", 

  [TOKEN_MINUS_GT]  = "->", 

  [TOKEN_ID]                = "identifier",

  [TOKEN_NUMBER]            = "number",
  [TOKEN_CHAR]              = "char",
  [TOKEN_STRING]	          = "string",

  [TOKEN_NULL_KEYWORD]      = "null keyword",

  [TOKEN_INT8_KEYWORD]      = "int8 keyword",
  [TOKEN_INT16_KEYWORD]     = "int16 keyword",
  [TOKEN_INT32_KEYWORD]     = "int32 keyword",
  [TOKEN_INT64_KEYWORD]     = "int64 keyword",
  [TOKEN_UINT8_KEYWORD]     = "uint8 keyword",
  [TOKEN_UINT16_KEYWORD]    = "uint16 keyword",
  [TOKEN_UINT32_KEYWORD]    = "uint32 keyword",
  [TOKEN_UINT64_KEYWORD]    = "uint64 keyword",
  [TOKEN_BIGINT_KEYWORD]    = "bigint keyword",

  [TOKEN_FLOAT32_KEYWORD]   = "float32 keyword",
  [TOKEN_FLOAT64_KEYWORD]   = "float64 keyword",
  [TOKEN_BIGFLOAT_KEYWORD]  = "bigfloat keyword",

  [TOKEN_CHAR_KEYWORD]      = "char keyword",
  [TOKEN_STRING_KEYWORD]    = "string keyword",

  [TOKEN_CONTINUE_KEYWORD]  = "continue keyword",
  [TOKEN_BREAK_KEYWORD]     = "break keyword",
  [TOKEN_ELSE_KEYWORD]      = "else keyword",
  [TOKEN_IF_KEYWORD]        = "if keyword",
  [TOKEN_TRY_KEYWORD]       = "try keyword",
  [TOKEN_CATCH_KEYWORD]     = "catch keyword",
  [TOKEN_THROW_KEYWORD]     = "throw keyword",
  [TOKEN_RETURN_KEYWORD]    = "return keyword",
  [TOKEN_FOR_KEYWORD]       = "for keyword",
  [TOKEN_SIZEOF_KEYWORD]    = "sizeof keyword",
  [TOKEN_TYPEOF_KEYWORD]    = "typeof keyword",
  [TOKEN_FUN_KEYWORD]       = "fun keyword",
  [TOKEN_CLASS_KEYWORD]     = "class keyword",
  [TOKEN_EXTENDS_KEYWORD]   = "extends keyword",
  [TOKEN_STATIC_KEYWORD]    = "static keyword",
  [TOKEN_READONLY_KEYWORD]  = "readonly keyword",
  [TOKEN_REFERENCE_KEYWORD] = "reference keyword",
  [TOKEN_VAR_KEYWORD]     	= "var keyword",
  [TOKEN_EXPORT_KEYWORD]    = "export keyword",
  [TOKEN_USING_KEYWORD]     = "using keyword",
  [TOKEN_FROM_KEYWORD]      = "from keyword"
};


const char *
sy_token_get_name(int32_t tp){
    return symbols[tp];
}