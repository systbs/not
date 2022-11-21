#pragma once

#ifndef __LEXER_H__
#define __LEXER_H__

typedef struct lexer {
	char *path;
	char *source;
	
	uint64_t position;
	uint64_t line;
	uint64_t column;
	
	token_t token;
} lexer_t;

lexer_t *
lexer_create(char *path, char *source);

void
lexer_destroy(lexer_t *lexer);

char *
lexer_load_file(char *path);

token_t *
lexer_get_token(lexer_t *lexer);

char *
lexer_get_path(lexer_t *lexer);

char *
lexer_get_source(lexer_t *lexer);

uint64_t
lexer_get_position(lexer_t *lexer);

uint64_t
lexer_get_line(lexer_t *lexer);

uint64_t
lexer_get_column(lexer_t *lexer);

void 
lexer_set_token(lexer_t *lexer, token_t token);

void
lexer_set_path(lexer_t *lexer, char *path);

void
lexer_set_source(lexer_t *lexer, char *source);

void
lexer_set_position(lexer_t *lexer, uint64_t position);

void
lexer_set_line(lexer_t *lexer, uint64_t line);

void
lexer_set_column(lexer_t *lexer, uint64_t column);

int32_t
lexer_get_next_token(lexer_t *lexer);

#endif // __LEXER_H__
