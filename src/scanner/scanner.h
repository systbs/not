#pragma once

#ifndef __scanner_H__
#define __scanner_H__

typedef struct scanner 
{
	file_source_t *file_source;
	list_t *errors;
	char *source;
	
	int32_t ch;

	uint64_t offset;
	uint64_t reading_offset;
	uint64_t column;
	uint64_t line;

	token_t token;

	program_t *program;
} scanner_t;

scanner_t *
scanner_create(file_source_t *file_source, list_t *errors);

void
scanner_destroy(scanner_t *scanner);

void 
scanner_set_token(scanner_t *scanner, token_t token);

int32_t
scanner_advance(scanner_t *scanner);

int32_t
scanner_next_gt(scanner_t *scanner);

#endif // __scanner_H__
