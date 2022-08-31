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
#include "list.h"
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
    assert_format(!token_done(prs), "[PARSER] call next token in end of list");\
    prs->c = prs->c->next;\
    prs->c;\
})

#define token_prev(prs) ({\
    assert_format(!token_begin(prs), "[PARSER] call next token in end of list");\
    prs->c = prs->c->previous;\
    prs->c;\
})

int
next_n_is(parser_t *prs, long_t n, long_t identifier){
    ilist_t *b = prs->c;
    long_t i = 0;
    for(i = 0; i < n; i++){
        b = b->next;
        if((b == prs->tokens->end)){
            return 0;
        }
    }
    token_t *token = (token_t *)list_content(b);
    return token->identifier == identifier;
}

int
next_is(parser_t *prs, long_t identifier){
    ilist_t *b = prs->c;
    token_t *token = (token_t *)list_content(b->next);
    return token->identifier == identifier;
}

#define next_each(prs, ids)({\
    ilist_t *b = prs->c;\
    token_t *token = (token_t *)list_content(b->next);\
    long_t i = 0;\
    int res = 0; \
    long_t len = sizeof(ids) / sizeof(ids[0]);\
    for(i = 0; i < len; i++){\
        res = res || (token->identifier == ids[i]);\
    }\
    res;\
})

int
prev_is(parser_t *prs, long_t identifier){
    ilist_t *b = prs->c;
    token_t *token = (token_t *)list_content(b->previous);
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
statement(parser_t *prs, list_t *code);


void
expression_comma(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_COMMA), 
        "[COMMA] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, COMMA);
}

void
expression_continue(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_CONTINUE), 
        "[CONTINUE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, JMP);
    ilist_t *b;
    for(b = list_last(code); b != code->begin; b = b->previous){
        if((b->value == BLP) && (b->next->value == ELP)){
            list_rpush(code, (long_t)b);
            return;
        }
    }
    assert_format(!!(b != code->begin), 
        "[CONTINUE] bad expression, can not find begin scope [row:%ld col:%ld]\n", 
            token->row, token->col);
}

void
expression_break(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_BREAK), 
        "[BREAK] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, JMP);
    ilist_t *b;
    for(b = list_last(code); b != code->begin; b = b->previous){
        if(b->value == ELP && b->previous->value == BLP){
            list_rpush(code, (long_t)b);
            return;
        }
    }
    assert_format(!!(b != code->begin), 
        "[BREAK] bad expression [row:%ld col:%ld]\n", token->row, token->col);
}

void
expression_id(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);

    assert_format((token->identifier == TOKEN_ID), 
        "[ID] bad expression [row:%ld col:%ld]\n", token->row, token->col);
	
	list_rpush(code, IMM);
    list_rpush(code, token->value);
    list_rpush(code, TP_VAR);

    if(next_is(prs, TOKEN_LPAREN)){
        list_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RPAREN)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        list_rpush(code, CALL);
    }
    else if(next_is(prs, TOKEN_LBRACKET)){
        list_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RBRACKET)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        list_rpush(code, CELL);
    }

    if(prs->ub == 1){
        return;
    }

    int ids_moreexp[] = {
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
        TOKEN_GTGT,
        TOKEN_DOT
    };
    if(next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_super(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);

    assert_format((token->identifier == TOKEN_SUPER), 
        "[SUPER] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, SUPER);
    
    if(next_is(prs, TOKEN_LPAREN)){
        list_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RPAREN)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        list_rpush(code, CALL);
    }
    else if(next_is(prs, TOKEN_LBRACKET)){
        list_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RBRACKET)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        list_rpush(code, CELL);
    }

    if(prs->ub == 1){
        return;
    }

    int ids_moreexp[] = {
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
        TOKEN_GTGT,
        TOKEN_DOT
    };
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_this(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_THIS), 
        "[THIS] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, THIS);

    if(next_is(prs, TOKEN_LPAREN)){
        list_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RPAREN)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        list_rpush(code, CALL);
    }
    else if(next_is(prs, TOKEN_LBRACKET)){
        list_rpush(code, PUSH);
        token_next(prs);
        if(!next_is(prs, TOKEN_RBRACKET)){
            expression(prs, code);
        }else{
            token_next(prs);
        }
        list_rpush(code, CELL);
    }

    if(prs->ub == 1){
        return;
    }

    int ids_moreexp[] = {
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
        TOKEN_GTGT,
        TOKEN_DOT
    };
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_data(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);

    assert_format((token->identifier == TOKEN_DATA), 
        "[DATA] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(code, IMM);
    list_rpush(code, token->value);
    list_rpush(code, TP_ARRAY);

    if(prs->ub == 1){
        return;
    }

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_number(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);

    assert_format((token->identifier == TOKEN_NUMBER), 
        "[NUMBER] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(code, IMM);
    list_rpush(code, token->value);
    list_rpush(code, TP_NUMBER);

    if(prs->ub == 1){
        return;
    }

    int ids_moreexp[] = {
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

    if(next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_null(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);

    assert_format((token->identifier == TOKEN_NULL), 
        "[NULL] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(code, IMM);
    list_rpush(code, token->value);
    list_rpush(code, TP_NULL);

    if(prs->ub == 1){
        return;
    }

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}



void
expression_eq(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_EQ), 
        "[EQ] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, SD);
}

void
expression_not(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_NOT), 
        "[NOT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, PUSH);
    list_rpush(code, IMM);
    list_rpush(code, 0);
    list_rpush(code, TP_IMM);
    list_rpush(code, EQ);
}

void
expression_tilde(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_TILDE), 
        "[TILDE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, PUSH);
    list_rpush(code, IMM);
    list_rpush(code, -1);
    list_rpush(code, TP_IMM);
    list_rpush(code, XOR);
}

void
expression_question(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_QUESTION), 
        "[QUESTION] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(code, JZ);
    ilist_t *a = list_rpush(code, 0);
    token_next(prs);
    expression(prs, code);
    token_next(prs);

    token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_COLON), 
        "[QUESTION] missing colon in conditional [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);

    list_rpush(code, JMP);
    ilist_t *b = list_rpush(code, 0);

    expression(prs, code);

    a->value = (long_t) b->next;
    b->value = (long_t) list_rpush(code, NUL);
}

void
expression_caret(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_CARET), 
        "[CARET] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"^");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, XOR);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_mul(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_STAR), 
        "[STAR] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    
    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"*");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
        expression(prs, code);
    }
    list_rpush(code, MUL);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_div(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_SLASH), 
        "[SLASH] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"/");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, DIV);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_episode(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_BACKSLASH), 
        "[BACKSLASH] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"\\");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, EPISODE);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_mod(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_PERCENT), 
        "[PERCENT] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"%");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, MOD);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_plus(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_PLUS), 
        "[PLUS] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    
    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"+");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    prs->ub = 1;
    expression(prs, code);
    int ids_heighexp[] = {
        TOKEN_CARET,
        TOKEN_LAND,
        TOKEN_LOR,
        TOKEN_STAR, 
        TOKEN_SLASH, 
        TOKEN_PERCENT, 
        TOKEN_BACKSLASH
    };
    if(next_each(prs, ids_heighexp)){
        token_next(prs);
        expression(prs, code);
    }
    list_rpush(code, ADD);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_minus(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_MINUS), 
        "[MINUS] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"-");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, SUB);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_ee(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_EQEQ), 
        "[EQEQ] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, EQ);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_ne(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_NEQ), 
        "[NE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    
    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"!=");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, NE);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_ge(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_GTEQ), 
        "[GE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    
    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)">=");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, GE);
}

void
expression_le(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LTEQ), 
        "[LE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, LE);
}

void
expression_land(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LAND), 
        "[LAND] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    
    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"&");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, LAND);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_lor(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LOR), 
        "[LOR] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"|");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
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
    list_rpush(code, LOR);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_and(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_AND), 
        "[AND] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    
    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"&&");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, AND);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_or(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_OR), 
        "[OR] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"||");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, OR);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_lt(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LT), 
        "[LT] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"<");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, LT);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_gt(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_GT), 
        "[GT] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)">");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, GT);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_lshift(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LTLT), 
        "[LSHIFT] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)"<<");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, LSHIFT);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_rshift(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_GTGT), 
        "[RSHIFT] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    if(next_is(prs, TOKEN_COLON)){
        list_rpush(code, IMM);
        list_rpush(code, (long_t)">>");
        list_rpush(code, TP_VAR);
        return;
    }

    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, RSHIFT);

    prs->ub = 0;

    int ids_moreexp[] = {
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
    if(!!next_each(prs, ids_moreexp)){
        token_next(prs);
        expression(prs, code);
    }
}

void
expression_if(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_IF), 
        "[IF] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    /* if expr expr else expr */
    token_next(prs);
    expression(prs, code);

    list_rpush(code, JZ);
    ilist_t *a = list_rpush(code, 0);

    if(next_is(prs, TOKEN_LBRACE)){
        token_next(prs);
        token_next(prs);
        do {
            expression(prs, code);
            token_next(prs);
            token = (token_t *)list_content(prs->c);
        } while(token->identifier != TOKEN_RBRACE);
    }
    else {
        expression(prs, code);
    }

    if(next_is(prs, TOKEN_ELSE)){
        token_next(prs);

        list_rpush(code, JMP);
        ilist_t *b = list_rpush(code, NUL);

        if(next_is(prs, TOKEN_IF)){
            a->value = (long_t) list_rpush(code, NUL);
            b->value = a->value;
            return;
        }

        if(next_is(prs, TOKEN_LBRACE)){
            token_next(prs);
            token_next(prs);
            do {
                expression(prs, code);
                token_next(prs);
                token = (token_t *)list_content(prs->c);
            } while(token->identifier != TOKEN_RBRACE);
        }
        else {
            expression(prs, code);
        }

        a->value = (long_t) b->next;
        a = b;
    }

    a->value = (long_t) list_rpush(code, NUL);
}

void
expression_while(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_WHILE), 
        "[WHILE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    /* while expr expr */
    list_rpush(code, BSCP);

    ilist_t *a = list_rpush(code, BLP);
    ilist_t *c = list_rpush(code, ELP);

    token_next(prs);
    expression(prs, code);

    list_rpush(code, JZ);
    ilist_t *b = list_rpush(code, 0);

    if(next_is(prs, TOKEN_LBRACE)){
        token_next(prs);
        token_next(prs);
        do {
            expression(prs, code);
            token_next(prs);
            token = (token_t *)list_content(prs->c);
        } while (token->identifier != TOKEN_RBRACE);
    } else {
        expression(prs, code);
    }

    list_unlink(code, c);
    list_rpush(code, JMP);
    list_rpush(code, (long_t)a->next);
    list_link(code, code->end, c);

    b->value = (long_t) c;
    list_rpush(code, ESCP);
}

void
expression_return(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_RETURN), 
        "[RETURN] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, RET);
    list_rpush(code, LEV);
}

void
expression_print(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_PRINT), 
        "[PRINT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, PRTF);
}

void
expression_lparen(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LPAREN), 
        "[LPAREN] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(prs->stack, prs->ub);
    prs->ub = 0;
    token_next(prs);
    do {
        expression(prs, code);
        token_next(prs);
        token = (token_t *)list_content(prs->c);
    } while (token->identifier != TOKEN_RPAREN);
    prs->ub = list_content(list_rpop(prs->stack));
}

void
expression_lbracket(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LBRACKET), 
        "[LBRACKET] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    do {
        expression(prs, code);
        token_next(prs);
        token = (token_t *)list_content(prs->c);
    } while (token->identifier != TOKEN_RBRACKET);
    list_rpush(code, ARRAY);
}

void
expression_lbrace(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LBRACE), 
        "[LBRACE] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    do {
        expression(prs, code);
        token_next(prs);
        token = (token_t *)list_content(prs->c);
    } while (token->identifier != TOKEN_RBRACE);
    list_rpush(code, TUPLE);
}

void
expression_fn(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_FN), 
        "[FN] bad expression [row:%ld col:%ld]\n", token->row, token->col);
	
	/*
		fn (param1, param2, ...){
		
		}
	*/
	
    list_rpush(code, JMP);
    ilist_t *region = list_rpush(code, 0);

    schema_t *schema = schema_create(prs->schema);
    list_rpush(prs->schema->branches, (list_value_t)schema);

    schema->start = list_rpush(code, ENT);
    
    if(next_is(prs, TOKEN_LPAREN)){
        token_next(prs);
        // if next token is ')' forward to next token
        if(next_is(prs, TOKEN_RPAREN)){
            token_next(prs);
        }else{
            expression(prs, code);
            list_rpush(code, LD);
        }
    }

    token_next(prs);
    token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LBRACE), 
        "[DEF] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(prs->schemas, (list_value_t)prs->schema);
    prs->schema = schema;
    
    if(!next_is(prs, TOKEN_RBRACE)){
        token_next(prs);
		do {
		    expression(prs, code);
		    token_next(prs);
		    token = (token_t *)list_content(prs->c);
		} while (token->identifier != TOKEN_RBRACE);
    }

    prs->schema = (schema_t *)list_content(list_rpop(prs->schemas));
    
    list_rpush(code, NUL);
    list_rpush(code, RET);

    schema->end = list_rpush(code, LEV);

    region->value = (long_t)list_rpush(code, IMM);
    list_rpush(code, (long_t)schema);
    list_rpush(code, TP_SCHEMA);
}

void
expression_class(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_CLASS), 
        "[DEF] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(code, JMP);
    ilist_t *region = list_rpush(code, 0);

    schema_t *schema = schema_create(prs->schema);
    list_rpush(prs->schema->branches, (list_value_t)schema);

    schema->start = list_rpush(code, ENT);
    list_rpush(code, EXD);
    
    token_next(prs);
    token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_LBRACE), 
        "[DEF] bad expression [row:%ld col:%ld]\n", token->row, token->col);

    list_rpush(prs->schemas, (list_value_t)prs->schema);
    prs->schema = schema;
    
    if(!next_is(prs, TOKEN_RBRACE)){
        token_next(prs);
		do {
		    expression(prs, code);
		    token_next(prs);
		    token = (token_t *)list_content(prs->c);
		} while (token->identifier != TOKEN_RBRACE);
    }

    prs->schema = (schema_t *)list_content(list_rpop(prs->schemas));
    
    list_rpush(code, THIS);
    list_rpush(code, RET);

    schema->end = list_rpush(code, LEV);

    region->value = (long_t)list_rpush(code, IMM);
    list_rpush(code, (long_t)schema);
    list_rpush(code, TP_SCHEMA);
}

void
expression_dot(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_DOT), 
        "[DOT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, SIM);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, REL);
}

void
expression_eval(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_EVAL), 
        "[EVAL] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, EVAL);

    int ids[] = {
        TOKEN_COMMA, 
        TOKEN_SEMICOLON, 
        TOKEN_RBRACE, 
        TOKEN_RBRACKET, 
        TOKEN_RPAREN,
        TOKEN_COLON
    };
    if(next_each(prs, ids) || (prs->ub == 1)){
        return;
    }

    token_next(prs);
    expression(prs, code);
}


/*
    expr : expr;
*/
void
expression_colon(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_COLON), 
        "[COLON] bad statement [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, PUSH);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, CLS);
}

/*
    import expr;
*/
void
expression_import(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_IMPORT), 
        "[IMPORT] bad statement [row:%ld col:%ld]\n", token->row, token->col);
    token_next(prs);
    expression(prs, code);
    list_rpush(code, IMPORT);
}

void
expression_exit(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);
    assert_format((token->identifier == TOKEN_EXIT), 
        "[EXIT] bad expression [row:%ld col:%ld]\n", token->row, token->col);
    list_rpush(code, EXIT);
}

void
expression(parser_t *prs, list_t *code){
    token_t *token = (token_t *)list_content(prs->c);

    //printf(" %s ", (char *)token->symbol);

    if(token->identifier == TOKEN_COLON){
        expression_colon(prs, code);
        return;
    }
    else if (token->identifier == TOKEN_IMPORT){
        expression_import(prs, code);
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
    } else if(token->identifier == TOKEN_FN){
        expression_fn(prs, code);
        return;
    } else if(token->identifier == TOKEN_CLASS){
        expression_class(prs, code);
        return;
    } else if(token->identifier == TOKEN_EVAL){
        expression_eval(prs, code);
        return;
    } else if(token->identifier == TOKEN_EXIT){
        expression_exit(prs, code);
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
    
    assert_format(token_done(prs),
        "bad expression [row:%ld col:%ld] %ld\n", token->row, token->col, token->identifier);

    return;
}


void 
statement(parser_t *prs, list_t *code) {
    do {
        //token_t *token = (token_t *)list_content(prs->c);
        //printf(" %s ", (char *)token->symbol);
        prs->ub = 0;
        expression(prs, code);
        token_next(prs);
    } while(!token_done(prs));
}

const char * 
const STRCODE[] = {
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
	[EXD] = "EXD",
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
parse(list_t *tokens, list_t *code){
    parser_t *prs;

    assert_format(!!(prs = (parser_t *)qalam_malloc(sizeof(parser_t))),
        "bad expression defination, alloc memory");

    prs->schema = schema_create(nullptr);

    prs->schema->start = list_rpush(code, ENT);

    prs->tokens = tokens;

    prs->schemas = list_create();
    prs->frame = list_create();
    prs->stack = list_create();
 
    prs->c = tokens->begin;
    statement(prs, code);
    list_rpush(code, THIS);
    list_rpush(code, RET);
    prs->schema->end = list_rpush(code, LEV);

    return prs;
}
