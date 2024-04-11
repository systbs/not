#pragma once

#ifndef __scanner_H__
#define __scanner_H__

typedef struct scanner 
{
    char path[_MAX_DIR + _MAX_FNAME + _MAX_EXT];
	char *source;
	
	int32_t ch;

	uint64_t offset;
	uint64_t reading_offset;
	uint64_t column;
	uint64_t line;

	token_t token;
} scanner_t;

scanner_t *
scanner_create(program_t *program, char *path);

void
scanner_destroy(program_t *program, scanner_t *scanner);

int32_t
scanner_advance(program_t *program, scanner_t *scanner);

int32_t
scanner_gt (program_t *program, scanner_t *scanner);

#endif // __scanner_H__
