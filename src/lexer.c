/*
    Created by YAS - 2021
    
    Lexer:
    This part converts the program text to 
    an understandable form for the parser.
    
    Every token contains:
        long_t identifier;     // --> Token type: TOKEN_EQ mean "="
        long_t value;          // if token from type string or char or number value set to pointer it
        long_t pos;            // posation of token in document
        long_t row;            // line token
        long_t col;            // column token
        long_t fileid;         // file identifier 
        const char * symbol;    // name of symbol, for example "=" -> "EQ"

    Tokens, which are a set of pre-commands, 
    are later stored in a linked list(list_t) and then parsed 

    example: [test.q]

    category4: def {
        w = "simple text 4";
    };

    #first token:
    identifier: TOKEN_ID
    value: (long_t)"category4"
    pos: 1
    row: 1
    col: 1
    fileid: (long_t)"test.q"
    symbol: "ID"
    
    #next token
    identifier: TOKEN_COLON
    value: null
    pos: 9
    row: 1
    col: 9
    fileid: (long_t)"test.q"
    symbol: "COLON"

    #next token
    identifier: TOKEN_DEF
    value: null
    pos: 11
    row: 1
    col: 11
    fileid: (long_t)"test.q"
    symbol: "DEF"

    ...
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "utils.h"
#include "list.h"

#include "lexer.h"

// name of every token
const char * const symbols[] = {
  [TOKEN_EOF]       = "EOF",
  [TOKEN_SPACE]     = "SPACE",
  [TOKEN_NOT]       = "NOT",	
  [TOKEN_QUTA]      = "QUTA",	
  [TOKEN_HASH]      = "HASH",	
  [TOKEN_DOLLER]    = "DOLLOR",
  [TOKEN_PERCENT]   = "PRECENT",
  [TOKEN_AND]       = "AND", 	
  [TOKEN_PRIME]     = "PRIME",	
  [TOKEN_LPAREN]    = "LPAREN",	
  [TOKEN_RPAREN]    = "RPAREN",	
  [TOKEN_STAR]      = "STAR",	 
  [TOKEN_PLUS]      = "PLUS",	 
  [TOKEN_COMMA]     = "COMMA",	
  [TOKEN_MINUS]     = "MINUS",	
  [TOKEN_DOT]       = "DOT",
  [TOKEN_SLASH]     = "SLASH",
  [TOKEN_COLON]     = "COLON",	
  [TOKEN_SEMICOLON] = "SEMICOLON",
  [TOKEN_LT]        = "LT",
  [TOKEN_EQ]        = "EQ",	
  [TOKEN_GT]        = "GT",	 
  [TOKEN_QUESTION]  = "QUESTION", 
  [TOKEN_AT]        = "AT",  
  [TOKEN_LBRACKET]  = "LBRACKET", 
  [TOKEN_BACKSLASH] = "BACKSLASH",
  [TOKEN_RBRACKET]  = "RBRACKET", 
  [TOKEN_CARET]     = "CARET",	 
  [TOKEN_UNDERLINE] = "UNDERLINE",
  [TOKEN_UPRIME]    = "UPRIME",	
  [TOKEN_LBRACE]    = "LBRACE",	
  [TOKEN_OR]        = "OR",	
  [TOKEN_RBRACE]    = "RBRACE",	 
  [TOKEN_TILDE]     = "TILDE",	
        
  [TOKEN_LTLT]      = "LTLT",     
  [TOKEN_GTGT]      = "GTGT",      
  [TOKEN_LOR]       = "LOR",       
  [TOKEN_LAND]      = "LAND",    
  [TOKEN_LTEQ]      = "LTEQ",      
  [TOKEN_GTEQ]      = "GTEQ",   
  [TOKEN_EQEQ]      = "EQEQ",   
  [TOKEN_NEQ]       = "NEQ",   
  [TOKEN_INC]       = "INC",     
  [TOKEN_DEC]       = "DEC",  

  [TOKEN_CONTINUE]  = "CONTINUE",
  [TOKEN_BREAK]     = "BREAK",
  [TOKEN_ELSE]      = "ELSE",
  [TOKEN_IF]        = "IF",
  [TOKEN_IMPORT]    = "IMPORT",
  [TOKEN_RETURN]    = "RETURN",
  [TOKEN_SUPER]     = "SUPER",
  [TOKEN_THIS]      = "THIS",
  [TOKEN_ID]        = "ID",
  [TOKEN_WHILE]     = "WHILE",
  [TOKEN_DELETE]    = "DELETE",
  [TOKEN_INSERT]    = "INSERT",
  [TOKEN_SIZEOF]    = "SIZEOF",
  [TOKEN_TYPEOF]    = "TYPEOF",
  [TOKEN_COUNT]     = "COUNT",
  [TOKEN_NULL]      = "NULL",
  [TOKEN_REF]       = "REF",
  [TOKEN_EVAL]      = "EVAL",
  [TOKEN_FN]        = "FN",
  [TOKEN_ARG]       = "ARG",
  [TOKEN_CLASS]     = "CLASS",

  // system function
  [TOKEN_FORMAT]    = "FORMAT",
  [TOKEN_PRINT]     = "PRINT",

  [TOKEN_CHAR]      = "CHAR",
  [TOKEN_DATA]      = "DATA",
  [TOKEN_NUMBER]    = "NUMBER",

  [TOKEN_LINE]      = "LINE"
};

// A function to facilitate token creation 
token_t *
token_create(long_t identifier, long_t value, long_t pos, long_t row, long_t col)
{
    token_t *token = (token_t *)malloc(sizeof(token_t));
    token->identifier = identifier;
    token->value = value;
    token->pos = pos;
    token->row = row;
    token->col = col;
    token->symbol = symbols[identifier];
    return token;
}

// destroy a token from linked list (list_t) 
long_t
token_destroy(ilist_t *it)
{
    token_t *token = (token_t *)it->value;
    if(token){
        if(token->value){
            qalam_free((void *)token->value);
        }
        qalam_free(token);
    }
    qalam_free(it);
    return 1;
}

/*  Report an error contain 
    ** pos: posation
    ** row: row
    ** col: colomn
    ** str: error message
*/
void
lexer_error(const char *source, long_t pos, long_t row, long_t col, char *str){
    printf("lexer(%ld:%ld): %s!\n", row, col, str);
    char c;
    while((c = source[pos--])){
        if(c == '\n'){
            break;
        }
    }
    while((c = source[pos++])){
        if(c != '\n'){
            printf("%c", c);
        }
    }
    exit(-1);
}

/* Main function for lexer part
    ** ls: linked list for save of created token
    ** source: buffer of program text

*/
void
lexer(list_t *ls, const char *source)
{
    long_t c, a;
    long_t pos = 0;
    long_t row = 1;
    long_t col = 1;

    while ((c = source[pos])) {
        if(c == '\n' || c == '\v'){
            row++;
            pos++;
            col = 1;
            continue;
        }

        if(white_space(c)){
            pos++;
            col++;
            continue;
        }

        if(!valid_alpha(c) && !valid_digit(c) && c != '_'){
            if(c == '"' || c == '\''){
                // parse string literal, currently, the only supported escape
                // character is '\n', store the string literal into data.
                long_t pos2 = pos + 1;
                pos++;
                col++;

                while ((a = source[pos]) && a != c) {
                    pos++;
                    col++;
                }

                char *data = malloc(sizeof(char) * (pos - pos2));
                strncpy(data, source + pos2, pos - pos2 );
                data[pos - pos2] = '\0';

                if(list_rpush(ls, (list_value_t)token_create(TOKEN_DATA,(long_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }

                pos++;
                col++;
                continue;
            }
            else if(c == '('){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_LPAREN,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ')'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_RPAREN,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '['){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_LBRACKET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ']'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_RBRACKET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '{'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_LBRACE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '}'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_RBRACE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '='){
                if ((a = source[pos + 1]) && a == '=') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_EQEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_EQ,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '?'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_QUESTION,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '|'){
                if ((a = source[pos + 1]) && a == '|') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_LOR,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_OR,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '&'){
                if ((a = source[pos + 1]) && a == '&') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_LAND,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_AND,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '^'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_CARET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '!'){
                if ((a = source[pos + 1]) && a == '=') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_NEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_NOT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '<'){
                if ((a = source[pos + 1]) && a == '<') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_LTLT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if ((a = source[pos + 1]) && a == '=') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_LTEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_LT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '>'){
                if ((a = source[pos + 1]) && a == '>') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_GTGT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if ((a = source[pos + 1]) && a == '=') {
                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_GTEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_GT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '+'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_PLUS,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '-'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_MINUS,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '*'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_STAR,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '/'){
                if ((a = source[pos + 1]) && a == '/') {
                    while ((a = source[pos]) && a != '\n') {
                        pos++;
                        col++;
                    }
                    row++;
                    continue;
                }
                else if ((a = source[pos + 1]) && a == '*') {
                    pos += 2;
                    col += 2;
                    long_t i = 0, p = 0;
                    while ((a = source[pos])) {
                        if(a == '*'){
                            if ((a = source[pos + 1]) && a == '/' && i < 1) {
                                break;
                            }
                            i--;
                        }
                        if(a == '\n'){
                            row++;
                        }
                        if(a == '/'){
                            if ((p = source[pos + 1]) && p == '*') {
                                i++;
                            }
                        }
                        pos++;
                        col++;
                    }
                    continue;
                }
                else if(list_rpush(ls, (list_value_t)token_create(TOKEN_SLASH,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '%'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_PERCENT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '.'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_DOT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ','){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_COMMA,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ':'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_COLON,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ';'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_SEMICOLON,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '~'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_TILDE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '#'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_HASH,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '_'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_UNDERLINE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '@'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_AT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '$'){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_DOLLER,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else {
                if(c < 0){
                    break;
                }
                lexer_error(source, pos, row, col, "bad token!");
            }
        }
        else if(valid_digit(c)) {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            long_t token_val = c - '0';
            long_t pos2 = pos;

            if (token_val > 0) {
                pos++;
                col++;

                // dec, starts with [1-9]
                while ((a = source[pos]) && (valid_digit(a) || a == '.') && !white_space(a)) {
                    pos++;
                    col++;
                }

                char *data = malloc(sizeof(char) * (pos - pos2));
                strncpy(data, source + pos2, pos - pos2 );
                data[pos - pos2] = '\0';

                if(list_rpush(ls, (list_value_t)token_create(TOKEN_NUMBER, (long_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }

                continue;
            } else {
                // starts with number 0
                a = source[pos + 1];
                if (a == 'x' || a == 'X') {
                    pos+=2;
                    col+=2;
                    //hex
                    while ((a = source[pos]) && valid_hexadecimal(a)) {
                        token_val = token_val * 16 + (a & 15) + (a >= 'A' ? 9 : 0);
                        pos++;
                        col++;
                    }

                    char *data = malloc(sizeof(char) * (pos - pos2));
                    strncpy(data, source + pos2, pos - pos2 );
                    data[pos - pos2] = '\0';

                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_NUMBER ,(long_t)data ,pos2 ,row ,col-(pos-pos2))) == nullptr){
                        lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                    }
                    continue;
                } else if(valid_digit(a)){
                    // oct
                    while ((a = source[pos]) && valid_octal(a)) {
                        token_val = token_val * 8 + a - '0';
                        pos++;
                        col++;
                    }

                    char *data = malloc(sizeof(char) * (pos - pos2));
                    strncpy(data, source + pos2, pos - pos2 );
                    data[pos - pos2] = '\0';

                    if(list_rpush(ls, (list_value_t)token_create(TOKEN_NUMBER, (long_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                        lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                    }
                } else {
                    pos++;
                    col++;
                }
            }

            char *data = malloc(sizeof(char) * (pos - pos2));
            strncpy(data, source + pos2, pos - pos2 );
            data[pos - pos2] = '\0';

            if(list_rpush(ls, (list_value_t)token_create(TOKEN_NUMBER, (long_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
            }
            continue;
        }
        else {
            // parse identifier
            long_t pos2 = pos;
            long_t hash = 0;
            while ((a = source[pos]) && ( valid_alpha(a) || valid_digit(a) || (a == '_'))) {
                hash = hash * 147 + a;
                pos++;
                col++;
            }

            if(strncmp(source + pos2, "while", 5) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_WHILE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "class", 5) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_CLASS,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "if", 2) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_IF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "else", 4) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_ELSE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "break", 5) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_BREAK,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "continue", 8) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_CONTINUE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "this", 4) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_THIS,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "super", 5) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_SUPER,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "return", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_RETURN,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "import", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_IMPORT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "sizeof", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_SIZEOF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "typeof", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_TYPEOF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "format", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_FORMAT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "print", 5) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_PRINT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "delete", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_DELETE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "null", 4) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_NULL,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "count", 4) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_COUNT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "insert", 6) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_INSERT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "eval", 4) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_EVAL,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "fn", 2) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_FN,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "exit", 4) == 0){
                if(list_rpush(ls, (list_value_t)token_create(TOKEN_EXIT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } 

            char *var = malloc(sizeof(char) * (pos - pos2));
            strncpy(var, source + pos2, pos - pos2);
            var[pos - pos2] = '\0';

            if(list_rpush(ls, (list_value_t)token_create(TOKEN_ID,(long_t)var,pos2,row,col-(pos-pos2))) == nullptr){
                lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
            }
            continue;
        }

        pos++;
        col++;
    }
}
