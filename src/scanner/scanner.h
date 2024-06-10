#pragma once
#ifndef __SCANNER_H__
#define __SCANNER_H__

typedef struct sy_scanner 
{
    char path[MAX_PATH];
	char *source;
	
	int32_t ch;

	uint64_t offset;
	uint64_t reading_offset;
	uint64_t column;
	uint64_t line;

	sy_token_t token;
} sy_scanner_t;

sy_scanner_t *
sy_scanner_create(char *path);

void
sy_scanner_destroy(sy_scanner_t *scanner);

int32_t
sy_scanner_advance(sy_scanner_t *scanner);

int32_t
sy_scanner_gt(sy_scanner_t *scanner);

sy_scanner_t *
sy_scanner_create_from_buffer(char *buf, char *path);

#endif // __scanner_H__
