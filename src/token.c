#include "types.h"
#include "token.h"

// name of every token
const char * const symbols[] = {
  [TOKEN_EOF]       = "EOF",
  [TOKEN_SPACE]     = "SPACE",
  [TOKEN_NOT]       = "!",	
  [TOKEN_QUTA]      = "\"",	
  [TOKEN_HASH]      = "#",	
  [TOKEN_DOLLER]    = "$",
  [TOKEN_PERCENT]   = "%",
  [TOKEN_AND]       = "&", 	
  [TOKEN_PRIME]     = "'",	
  [TOKEN_LPAREN]    = "(",	
  [TOKEN_RPAREN]    = ")",	
  [TOKEN_STAR]      = "*",	 
  [TOKEN_PLUS]      = "+",	 
  [TOKEN_COMMA]     = ",",	
  [TOKEN_MINUS]     = "-",	
  [TOKEN_DOT]       = ".",
  [TOKEN_SLASH]     = "/",
  [TOKEN_COLON]     = ":",	
  [TOKEN_SEMICOLON] = ";",
  [TOKEN_LT]        = "<",
  [TOKEN_EQ]        = "=",	
  [TOKEN_GT]        = ">",	 
  [TOKEN_QUESTION]  = "?", 
  [TOKEN_AT]        = "@",  
  [TOKEN_LBRACKET]  = "[", 
  [TOKEN_BACKSLASH] = "\\",
  [TOKEN_RBRACKET]  = "]", 
  [TOKEN_CARET]     = "^",	 
  [TOKEN_UNDERLINE] = "_",
  [TOKEN_UPRIME]    = "`",	
  [TOKEN_LBRACE]    = "{",	
  [TOKEN_OR]        = "|",	
  [TOKEN_RBRACE]    = "}",	 
  [TOKEN_TILDE]     = "~",

  [TOKEN_CONTINUE]  = "continue",
  [TOKEN_BREAK]     = "break",
  [TOKEN_ELSE]      = "else",
  [TOKEN_IF]        = "if",
  [TOKEN_IMPORT]    = "import",
  [TOKEN_FROM]    	= "from",
  [TOKEN_AS]    	= "as",
  [TOKEN_RETURN]    = "return",
  [TOKEN_SUPER]     = "super",
  [TOKEN_THIS]      = "this",
  [TOKEN_ID]        = "ID",
  [TOKEN_WHILE]     = "while",
  [TOKEN_SIZEOF]    = "sizeof",
  [TOKEN_TYPEOF]    = "typeof",
  [TOKEN_NULL]      = "null",
  [TOKEN_FN]        = "fn",
  [TOKEN_CLASS]     = "class",
  [TOKEN_VAR]     	= "var",
  [TOKEN_ENUM]     	= "enum",
  [TOKEN_EXTERN]    = "extern",

  [TOKEN_NUMBER]    = "number",
  [TOKEN_LETTERS]	= "letters",
  
  [TOKEN_CHAR]      = "char",
  [TOKEN_STRING]    = "string",
  [TOKEN_INT]    	= "int",
  [TOKEN_LONG]	    = "long",
  [TOKEN_FLOAT]	    = "float",
  [TOKEN_DOUBLE]    = "double",
  [TOKEN_ANY]	    = "any"
};

const char *
token_get_name(int32_t tp){
    return symbols[tp];
}

uint64_t
token_get_line(token_t *token){
	return token->line;
}

uint64_t
token_get_column(token_t *token){
	return token->column;
}

uint64_t
token_get_position(token_t *token){
	return token->position;
}

int32_t
token_get_type(token_t *token){
	return token->type;
}

char *
token_get_value(token_t *token){
	return token->value;
}
