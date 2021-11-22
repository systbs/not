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
#include "array.h"
#include "table.h"

#include "lexer.h"

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

  // system function
  [TOKEN_FORMAT]    = "FORMAT",
  [TOKEN_PRINT]     = "PRINT",

  [TOKEN_CHAR]      = "CHAR",
  [TOKEN_DATA]      = "DATA",
  [TOKEN_NUMBER]    = "NUMBER",

  [TOKEN_LINE]      = "LINE"
};


token_t *
token_create(arval_t identifier, arval_t value, arval_t pos, arval_t row, arval_t col)
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

arval_t
token_destroy(itable_t *it)
{
    token_t *token = (token_t *)it->value;
    if(token){
        if(token->value){
            free((void *)token->value);
        }
        free(token);
    }
    free(it);
    return 1;
}

void
lexer_error(const char *source, arval_t pos, arval_t row, arval_t col, char *str){
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


void
lexer(table_t *ls, const char *source)
{
    arval_t c, a;
    arval_t pos = 0;
    arval_t row = 1;
    arval_t col = 1;

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
                arval_t pos2 = pos + 1;
                pos++;
                col++;

                while ((a = source[pos]) && a != c) {
                    pos++;
                    col++;
                }

                char *data = malloc(sizeof(char) * (pos - pos2));
                strncpy(data, source + pos2, pos - pos2 );
                data[pos - pos2] = '\0';

                if(table_rpush(ls, (tbval_t)token_create(TOKEN_DATA,(arval_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }

                pos++;
                col++;
                continue;
            }
            else if(c == '('){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_LPAREN,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ')'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_RPAREN,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '['){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_LBRACKET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ']'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_RBRACKET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '{'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_LBRACE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '}'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_RBRACE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '='){
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_EQEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_EQ,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '?'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_QUESTION,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '|'){
                if ((a = source[pos + 1]) && a == '|') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_LOR,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_OR,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '&'){
                if ((a = source[pos + 1]) && a == '&') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_LAND,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_AND,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '^'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_CARET,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '!'){
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_NEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_NOT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '<'){
                if ((a = source[pos + 1]) && a == '<') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_LTLT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_LTEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_LT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '>'){
                if ((a = source[pos + 1]) && a == '>') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_GTGT,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if ((a = source[pos + 1]) && a == '=') {
                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_GTEQ,0,pos,row,col)) == nullptr){
                        lexer_error(source, pos, row, col, "not append data!");
                    }
                    pos += 2;
                    col += 2;
                    continue;
                }
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_GT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '+'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_PLUS,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '-'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_MINUS,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '*'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_STAR,0,pos,row,col)) == nullptr){
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
                    arval_t i = 0, p = 0;
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
                else if(table_rpush(ls, (tbval_t)token_create(TOKEN_SLASH,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '%'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_PERCENT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '.'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_DOT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ','){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_COMMA,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ':'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_COLON,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == ';'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_SEMICOLON,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '~'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_TILDE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '#'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_HASH,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '_'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_UNDERLINE,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '@'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_AT,0,pos,row,col)) == nullptr){
                    lexer_error(source, pos, row, col, "not append data!");
                }
                pos++;
                col++;
                continue;
            }
            else if(c == '$'){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_DOLLER,0,pos,row,col)) == nullptr){
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
            arval_t token_val = c - '0';
            arval_t pos2 = pos;

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

                if(table_rpush(ls, (tbval_t)token_create(TOKEN_NUMBER, (arval_t)data,pos2,row,col-(pos-pos2))) == nullptr){
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

                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_NUMBER ,(arval_t)data ,pos2 ,row ,col-(pos-pos2))) == nullptr){
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

                    if(table_rpush(ls, (tbval_t)token_create(TOKEN_NUMBER, (arval_t)data,pos2,row,col-(pos-pos2))) == nullptr){
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

            if(table_rpush(ls, (tbval_t)token_create(TOKEN_NUMBER, (arval_t)data,pos2,row,col-(pos-pos2))) == nullptr){
                lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
            }
            continue;
        }
        else {
            // parse identifier
            arval_t pos2 = pos;
            arval_t hash = 0;
            while ((a = source[pos]) && ( valid_alpha(a) || valid_digit(a) || (a == '_'))) {
                hash = hash * 147 + a;
                pos++;
                col++;
            }

            if(strncmp(source + pos2, "while", 5) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_WHILE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "def", 3) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_DEF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "if", 2) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_IF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "else", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_ELSE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "break", 5) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_BREAK,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "continue", 8) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_CONTINUE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "this", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_THIS,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "super", 5) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_SUPER,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "return", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_RETURN,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "import", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_IMPORT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "sizeof", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_SIZEOF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "typeof", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_TYPEOF,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "format", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_FORMAT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "print", 5) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_PRINT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "delete", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_DELETE,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "null", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_NULL,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "count", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_COUNT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "insert", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_INSERT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "eval", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_EVAL,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "fork", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_FORK,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "wait", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_WAIT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "getpid", 6) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_GETPID,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "exit", 4) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_EXIT,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            } else if(strncmp(source + pos2, "sleep", 5) == 0){
                if(table_rpush(ls, (tbval_t)token_create(TOKEN_SLEEP,0,pos2,row,col-(pos-pos2))) == nullptr){
                    lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
                }
                continue;
            }

            char *var = malloc(sizeof(char) * (pos - pos2));
            strncpy(var, source + pos2, pos - pos2);
            var[pos - pos2] = '\0';

            if(table_rpush(ls, (tbval_t)token_create(TOKEN_ID,(arval_t)var,pos2,row,col-(pos-pos2))) == nullptr){
                lexer_error(source, pos, row, col-(pos-pos2), "not append data!");
            }
            continue;
        }

        pos++;
        col++;
    }
}
