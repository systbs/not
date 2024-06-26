#pragma once
#ifndef __SCANNER_H__
#define __SCANNER_H__

typedef struct not_scanner
{
	char path[MAX_PATH];
	char *source;

	int32_t ch;

	uint64_t offset;
	uint64_t reading_offset;
	uint64_t column;
	uint64_t line;

	not_token_t token;
} not_scanner_t;

not_scanner_t *
not_scanner_create(char *path);

void not_scanner_destroy(not_scanner_t *scanner);

int32_t
not_scanner_advance(not_scanner_t *scanner);

int32_t
not_scanner_gt(not_scanner_t *scanner);

#endif // __scanner_H__
