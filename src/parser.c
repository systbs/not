#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>


#include "types.h"
#include "utils.h"
#include "array.h"
#include "table.h"
#include "lexer.h"
#include "memory.h"
#include "object.h"
#include "variable.h"
#include "schema.h"
#include "parser.h"
#include "data.h"

#define token_done(prs) (prs->c == prs->tokens->end)
#define token_begin(prs) (prs->c == prs->tokens->begin)

#define token_next(prs) ({\
    validate_format(!token_done(prs), "[PARSER] call next token in end of list");\
    prs->c = prs->c->next;\
    prs->c;\
})

#define token_prev(prs) ({\
    validate_format(!token_begin(prs), "[PARSER] call next token in end of list");\
    prs->c = prs->c->previous;\
    prs->c;\
})

int array_sizeof(int *arr) {
    int count=0;
    for(;*arr++;)
        count++;
    return count;
}

int
next_n_is(parser_t *prs, long_t n, arval_t identifier){
    itable_t *b = prs->c;
    long_t i = 0;
    for(i = 0; i < n; i++){
        b = b->next;
        if((b == prs->tokens->end)){
            return 0;
        }
    }
    token_t *token = (token_t *)table_content(b);
    return token->identifier == identifier;
}

int
next_is(parser_t *prs, arval_t identifier){
    itable_t *b = prs->c;
    token_t *token = (token_t *)table_content(b->next);
    return token->identifier == identifier;
}

int
next_each(parser_t *prs, int *ids){
    itable_t *b = prs->c;
    token_t *token = (token_t *)table_content(b->next);
    long_t i = 0;
    int res = 0; 
    long_t len = array_sizeof(ids);
    for(i = 0; i < len; i++){
        res = res || (token->identifier == ids[i]);
    }
    return res;
}

int
prev_is(parser_t *prs, arval_t identifier){
    itable_t *b = prs->c;
    token_t *token = (token_t *)table_content(b->previous);
    return token->identifier == identifier;
}

int
next_isop(parser_t *prs){
    int idsc[] = {
        TOKEN_PLUS,
        TOKEN_MINUS,
        TOKEN_STAR,
        TOKEN_SLASH,
        TOKEN_CARET,
        TOKEN_PERCENT,
        TOKEN_EQEQ,
        TOKEN_NEQ,
        TOKEN_GTEQ,
        TOKEN_LTEQ,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_AND,
        TOKEN_OR,
        TOKEN_LT,
        TOKEN_GT,
        TOKEN_LTLT,
        TOKEN_GTGT
    };
    return next_each(prs, idsc);
}




void
expression_colon(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_COLON), 
        "[COLON] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, CLS);
}

void
expression_comma(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_COMMA), 
        "[COMMA] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, COMMA);
}

void
expression_continue(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_CONTINUE), 
        "[CONTINUE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, JMP);
    iarray_t *b;
    for(b = array_last(code); b != code->begin; b = b->previous){
        if((b->value == BLP) && (b->next->value == ELP)){
            array_rpush(code, (arval_t)b);
            return;
        }
    }
    validate_format(!!(b != code->begin), 
        "[CONTINUE] bad expression, can not find begin scope [row:%ld col:%ld]\n", 
            token->row, token->col);
}

void
expression_break(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_BREAK), 
        "[BREAK] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, JMP);
    iarray_t *b;
    for(b = array_last(code); b != code->begin; b = b->previous){
        if(b->value == ELP && b->previous->value == BLP){
            array_rpush(code, (arval_t)b);
            return;
        }
    }
    validate_format(!!(b != code->begin), 
        "[BREAK] bad expression [row:%ld col:%ld]\n", token->row, token->col);
}




void
expression_id(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);

    validate_format((token->identifier == TOKEN_ID), 
        "[ID] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    array_rpush(code, IMM);
    array_rpush(code, token->value);
    array_rpush(code, TP_VAR);

    if(next_is(prs, TOKEN_LPAREN)){
        array_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RPAREN)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        array_rpush(code, CALL);
    }
    else if(next_is(prs, TOKEN_LBRACKET)){
        array_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RBRACKET)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        array_rpush(code, CELL);
    }

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}

void
expression_super(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);

    validate_format((token->identifier == TOKEN_SUPER), 
        "[SUPER] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, SUPER);
    
    if(next_is(prs, TOKEN_LPAREN)){
        array_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RPAREN)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        array_rpush(code, CALL);
    }
    else if(next_is(prs, TOKEN_LBRACKET)){
        array_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RBRACKET)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        array_rpush(code, CELL);
    }

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}

void
expression_this(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_THIS), 
        "[THIS] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, THIS);

    if(next_is(prs, TOKEN_LPAREN)){
        array_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RPAREN)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        array_rpush(code, CALL);
    }
    else if(next_is(prs, TOKEN_LBRACKET)){
        array_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RBRACKET)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        array_rpush(code, CELL);
    }

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}

void
expression_data(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);

    validate_format((token->identifier == TOKEN_DATA), 
        "[DATA] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    array_rpush(code, IMM);
    array_rpush(code, token->value);
    array_rpush(code, TP_ARRAY);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}

void
expression_number(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);

    validate_format((token->identifier == TOKEN_NUMBER), 
        "[NUMBER] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    array_rpush(code, IMM);
    array_rpush(code, token->value);
    array_rpush(code, TP_NUMBER);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}

void
expression_null(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);

    validate_format((token->identifier == TOKEN_NULL), 
        "[NULL] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    array_rpush(code, IMM);
    array_rpush(code, token->value);
    array_rpush(code, TP_NULL);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}




void
expression_eq(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_EQ), 
        "[EQ] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, SD);
}

void
expression_not(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_NOT), 
        "[NOT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, PUSH);
    array_rpush(code, IMM);
    array_rpush(code, 0);
    array_rpush(code, TP_IMM);
    array_rpush(code, EQ);
}

void
expression_tilde(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_TILDE), 
        "[TILDE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, PUSH);
    array_rpush(code, IMM);
    array_rpush(code, -1);
    array_rpush(code, TP_IMM);
    array_rpush(code, XOR);
}

void
expression_question(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_QUESTION), 
        "[QUESTION] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    array_rpush(code, JZ);
    iarray_t *a = array_rpush(code, 0);
    token_next(prs);
    expression(prs, code);
    token_next(prs);

    token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_COLON), 
        "[QUESTION] missing colon in conditional [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);

    array_rpush(code, JMP);
    iarray_t *b = array_rpush(code, 0);

    expression(prs, code);

    a->value = (arval_t) b->next;
    b->value = (arval_t) array_rpush(code, NUL);
}

void
expression_caret(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_CARET), 
        "[CARET] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR
    };
    if(next_each(prs, ids1)){
        prs->ub = 1;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, XOR);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    token_next(prs);
    prs->ub = 0;
    expression(prs, code);
}

void
expression_mul(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_STAR), 
        "[STAR] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids1)){
        token_next(prs);
        prs->ub = 1;
        expression(prs, code);
    }
    array_rpush(code, MUL);

    prs->ub = 0;
    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    if(next_isop(prs)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_div(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_SLASH), 
        "[SLASH] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids1)){
        prs->ub = 1;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, DIV);

    prs->ub = 0;
    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    if(next_isop(prs)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_episode(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_BACKSLASH), 
        "[BACKSLASH] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids1)){
        prs->ub = 1;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, EPISODE);

    prs->ub = 0;
    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    if(next_isop(prs)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_mod(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_PERCENT), 
        "[PERCENT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids1)){
        prs->ub = 0;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, MOD);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    token_next(prs);
    prs->ub = 0;
    expression(prs, code);
}

void
expression_plus(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_PLUS), 
        "[PLUS] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids1)){
        token_next(prs);
        prs->ub = 1;
        expression(prs, code);
    }
    array_rpush(code, ADD);

    prs->ub = 0;
    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    if(next_isop(prs)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_minus(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_MINUS), 
        "[MINUS] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids1)){
        prs->ub = 1;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, SUB);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    token_next(prs);
    prs->ub = 0;
    expression(prs, code);
}

void
expression_ee(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_EQ), 
        "[EQ] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, EQ);
}

void
expression_ne(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_NEQ), 
        "[NE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, NE);
}

void
expression_ge(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_GTEQ), 
        "[GE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, GE);
}

void
expression_le(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LTEQ), 
        "[LE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, LE);
}

void
expression_land(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LAND), 
        "[LAND] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR
    };
    if(next_each(prs, ids1)){
        prs->ub = 1;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, LAND);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    token_next(prs);
    prs->ub = 0;
    expression(prs, code);
}

void
expression_lor(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LOR), 
        "[LOR] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids1[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR
    };
    if(next_each(prs, ids1)){
        prs->ub = 1;
        token_next(prs);
        expression(prs, code);
    }
    array_rpush(code, LOR);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids)){
        return;
    }

    token_next(prs);
    prs->ub = 0;
    expression(prs, code);
}

void
expression_and(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_AND), 
        "[AND] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, AND);
}

void
expression_or(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_OR), 
        "[OR] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, OR);
}

void
expression_lt(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LT), 
        "[LT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, LT);
}

void
expression_gt(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_GT), 
        "[GT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, GT);
}

void
expression_lshift(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LTLT), 
        "[LSHIFT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, LSHIFT);
}

void
expression_rshift(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_GTGT), 
        "[RSHIFT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, RSHIFT);
}

void
expression_if(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_IF), 
        "[IF] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    /* if expr expr else expr */
    token_next(prs);
    expression(prs, code);

    array_rpush(code, JZ);
    iarray_t *a = array_rpush(code, 0);

    if(next_is(prs, TOKEN_LBRACE)){
        token_next(prs);
        token_next(prs);
        do {
            expression(prs, code);
            token_next(prs);
            token = (token_t *)table_content(prs->c);
        } while(token->identifier != TOKEN_RBRACE);
    }
    else {
        expression(prs, code);
    }

    if(next_is(prs, TOKEN_ELSE)){
        token_next(prs);

        array_rpush(code, JMP);
        iarray_t *b = array_rpush(code, NUL);

        if(next_is(prs, TOKEN_IF)){
            a->value = (arval_t) array_rpush(code, NUL);
            b->value = a->value;
            return;
        }

        if(next_is(prs, TOKEN_LBRACE)){
            token_next(prs);
            token_next(prs);
            do {
                expression(prs, code);
                token_next(prs);
                token = (token_t *)table_content(prs->c);
            } while(token->identifier != TOKEN_RBRACE);
        }
        else {
            expression(prs, code);
        }

        a->value = (arval_t) b->next;
        a = b;
    }

    a->value = (arval_t) array_rpush(code, NUL);
}

void
expression_while(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_WHILE), 
        "[WHILE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    /* while expr expr */
    array_rpush(code, BSCP);

    iarray_t *a = array_rpush(code, BLP);
    iarray_t *c = array_rpush(code, ELP);

    token_next(prs);
    expression(prs, code);

    array_rpush(code, JZ);
    iarray_t *b = array_rpush(code, 0);

    if(next_is(prs, TOKEN_LBRACE)){
        token_next(prs);
        token_next(prs);
        do {
            expression(prs, code);
            token_next(prs);
            token = (token_t *)table_content(prs->c);
        } while (token->identifier != TOKEN_RBRACE);
    } else {
        expression(prs, code);
    }

    array_unlink(code, c);
    array_rpush(code, JMP);
    array_rpush(code, (arval_t)a->next);
    array_link(code, code->end, c);

    b->value = (arval_t) c;
    array_rpush(code, ESCP);
}

void
expression_return(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_RETURN), 
        "[RETURN] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, RET);
    array_rpush(code, LEV);
}

void
expression_print(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_PRINT), 
        "[PRINT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, PRTF);
}

void
expression_lparen(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LPAREN), 
        "[LPAREN] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    do {
        expression(prs, code);
        token_next(prs);
        token = (token_t *)table_content(prs->c);
    } while (token->identifier != TOKEN_RPAREN);
    
    if(next_is(prs, TOKEN_CGT)){
        token_next(prs);
        expression(prs, code);
        return;
    }
}

void
expression_lbracket(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LBRACKET), 
        "[LBRACKET] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    do {
        expression(prs, code);
        token_next(prs);
        token = (token_t *)table_content(prs->c);
    } while (token->identifier != TOKEN_RBRACKET);
    array_rpush(code, ARRAY);
}

void
expression_lbrace(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LBRACE), 
        "[LBRACE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    do {
        expression(prs, code);
        token_next(prs);
        token = (token_t *)table_content(prs->c);
    } while (token->identifier != TOKEN_RBRACE);
}

void
expression_def(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_DEF), 
        "[DEF] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    token_next(prs);
    token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_LBRACE), 
        "[DEF] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    array_rpush(code, JMP);
    iarray_t *region = array_rpush(code, 0);

    schema_t *schema = schema_create(prs->schema);
    table_rpush(prs->schema->branches, (tbval_t)schema);

    schema->start = array_rpush(code, ENT);
    array_rpush(code, EXTND);

    table_rpush(prs->schemas, (tbval_t)prs->schema);
    prs->schema = schema;

    expression(prs, code);

    prs->schema = (schema_t *)table_content(table_rpop(prs->schemas));
    
    array_rpush(code, THIS);
    array_rpush(code, RET);

    schema->end = array_rpush(code, LEV);

    region->value = (arval_t)array_rpush(code, IMM);
    array_rpush(code, (arval_t)schema);
    array_rpush(code, TP_SCHEMA);
}

void
expression_dot(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_DOT), 
        "[DOT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    array_rpush(code, SIM);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, REL);
}

void
expression_eval(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);
    validate_format((token->identifier == TOKEN_EVAL), 
        "[EVAL] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    array_rpush(code, EVAL);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_CGT,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}


void
expression(parser_t *prs, array_t *code){
    token_t *token = (token_t *)table_content(prs->c);

    //printf(" %s ", (char *)token->symbol);

    if(token->identifier == TOKEN_COLON) {
        expression_colon(prs, code);
        return;
    } else if(token->identifier == TOKEN_COMMA){
        expression_comma(prs, code);
        return;
    } else if(token->identifier == TOKEN_CONTINUE){
        expression_continue(prs, code);
        return;
    } else if(token->identifier == TOKEN_BREAK){
        expression_break(prs, code);
        return;
    } else if(token->identifier == TOKEN_ID){
        expression_id(prs, code);
        return;
    } else if(token->identifier == TOKEN_SUPER){
        expression_super(prs, code);
        return;
    } else if(token->identifier == TOKEN_THIS){
        expression_this(prs, code);
        return;
    } else if(token->identifier == TOKEN_NUMBER){
        expression_number(prs, code);
        return;
    } else if(token->identifier == TOKEN_DATA){
        expression_data(prs, code);
        return;
    } else if(token->identifier == TOKEN_NULL){
        expression_null(prs, code);
        return;
    } else if(token->identifier == TOKEN_EQ){
        expression_eq(prs, code);
        return;
    } else if(token->identifier == TOKEN_NOT){
        expression_not(prs, code);
        return;
    } else if(token->identifier == TOKEN_TILDE){
        expression_tilde(prs, code);
        return;
    } else if(token->identifier == TOKEN_QUESTION){
        expression_question(prs, code);
        return;
    } else if(token->identifier == TOKEN_CARET){
        expression_caret(prs, code);
        return;
    } else if(token->identifier == TOKEN_STAR){
        expression_mul(prs, code);
        return;
    } else if(token->identifier == TOKEN_SLASH){
        expression_div(prs, code);
        return;
    } else if(token->identifier == TOKEN_BACKSLASH){
        expression_episode(prs, code);
        return;
    } else if(token->identifier == TOKEN_PERCENT){
        expression_mod(prs, code);
        return;
    } else if(token->identifier == TOKEN_PLUS){
        expression_plus(prs, code);
        return;
    } else if(token->identifier == TOKEN_MINUS){
        expression_minus(prs, code);
        return;
    } else if(token->identifier == TOKEN_EQEQ){
        expression_ee(prs, code);
        return;
    } else if(token->identifier == TOKEN_NEQ){
        expression_ne(prs, code);
        return;
    } else if(token->identifier == TOKEN_GTEQ){
        expression_ge(prs, code);
        return;
    } else if(token->identifier == TOKEN_LTEQ){
        expression_le(prs, code);
        return;
    } else if(token->identifier == TOKEN_LAND){
        expression_land(prs, code);
        return;
    } else if(token->identifier == TOKEN_LOR){
        expression_lor(prs, code);
        return;
    } else if(token->identifier == TOKEN_AND){
        expression_and(prs, code);
        return;
    } else if(token->identifier == TOKEN_OR){
        expression_or(prs, code);
        return;
    } else if(token->identifier == TOKEN_LT){
        expression_lt(prs, code);
        return;
    } else if(token->identifier == TOKEN_GT){
        expression_gt(prs, code);
        return;
    } else if(token->identifier == TOKEN_LTLT){
        expression_lshift(prs, code);
        return;
    } else if(token->identifier == TOKEN_GTGT){
        expression_rshift(prs, code);
        return;
    } else if(token->identifier == TOKEN_IF){
        expression_if(prs, code);
        return;
    } else if(token->identifier == TOKEN_WHILE){
        expression_while(prs, code);
        return;
    } else if(token->identifier == TOKEN_RETURN){
        expression_return(prs, code);
        return;
    } else if(token->identifier == TOKEN_PRINT){
        expression_print(prs, code);
        return;
    } else if(token->identifier == TOKEN_LPAREN){
        expression_lparen(prs, code);
        return;
    } else if(token->identifier == TOKEN_LBRACKET){
        expression_lbracket(prs, code);
        return;
    } else if(token->identifier == TOKEN_LBRACE){
        expression_lbrace(prs, code);
        return;
    } else if(token->identifier == TOKEN_DEF){
        expression_def(prs, code);
        return;
    } else if(token->identifier == TOKEN_EVAL){
        expression_eval(prs, code);
        return;
    } else if(token->identifier == TOKEN_DOT){
        expression_dot(prs, code);
        return;
    } else if(token->identifier == TOKEN_RPAREN){
        return;
    } else if(token->identifier == TOKEN_RBRACKET){
        return;
    } else if(token->identifier == TOKEN_RBRACE){
        return;
    } else if(token->identifier == TOKEN_SEMICOLON){
        return;
    }
    
    validate_format(token_done(prs),
        "bad expression [row:%ld col:%ld]\n", token->row, token->col);

    return;
}

void 
statement(parser_t *prs, array_t *code) {
    do {
        prs->ub = 0;
        expression(prs, code);
        token_next(prs);
        //printf("\n");
    } while(!token_done(prs));
}




const char * const STRCODE[] = {
	[NUL] = "NUL", 
	[IMM] = "IMM",   
	[VAR] = "VAR",
	[DATA] = "DATA",
	[SUPER] = "SUPER", 
	[THIS] = "THIS", 

	[ENT] = "ENT", 
	[LEV] = "LEV",  
	[PUSH] = "PUSH", 
	[POP] = "POP", 
	[CONTINUE] = "CONTINUE", 
	[BREAK] = "BREAK",
	[DOT] = "DOT", 
	[CALL] = "CALL", 
	[CELL] = "CELL",

	[JMP] = "JMP",  
	[JZ] = "JZ",  	
	[JNZ] = "JNZ",  
	[LD] = "LD",  
	[SD] = "SD",  
	[HASH] = "HASH",
	[RET] = "RET",
    [SIM] = "SIM",
    [REL] = "REL",
	[EXTND] = "EXTND",
	[FN] = "FN",
	[AT] = "AT", 
	[CLS] = "CLS", 
	[COMMA] = "COMMA",
	[BLP] = "BLP",
	[ELP] = "ELP",

	[OR] = "OR",
	[LOR] = "LOR",
	[XOR] = "XOR",
	[AND] = "AND",
	[LAND] = "LAND",
	[EQ] = "EQ",
	[NE] = "NE",
	[LT] = "LT",
	[GT] = "GT",
	[LE] = "LE",
	[GE] = "GE",
	[LSHIFT] = "LSHIFT",
	[RSHIFT] = "RSHIFT",

	[ADD] = "ADD",
	[SUB] = "SUB",
	[MUL] = "MUL",
	[DIV] = "DIV",
	[MOD] = "MOD",
	[EPISODE] = "EPISODE",

	[PRTF] = "PRTF",  
	[SIZEOF] = "SIZEOF",
	[TYPEOF] = "TYPEOF",

	[EXIT] = "EXIT"
};

const char *
code_typeAsString(int tp){
    return STRCODE[tp];
}

parser_t *
parse(table_t *tokens, array_t *code){
    parser_t *prs;

    validate_format(!!(prs = (parser_t *)qalam_malloc(sizeof(parser_t))),
        "bad expression defination, alloc memory");

    prs->schema = schema_create(nullptr);

    prs->schema->start = array_rpush(code, ENT);

    prs->tokens = tokens;

    prs->schemas = table_create();

    prs->c = tokens->begin;

    statement(prs, code);

    prs->schema->end = array_rpush(code, LEV);

    return prs;
}
