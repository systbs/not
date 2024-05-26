#pragma once
#ifndef __SCANNER_H__
#define __SCANNER_H__

typedef struct SyScanner 
{
    char path[_MAX_DIR + _MAX_FNAME + _MAX_EXT];
	char *source;
	
	int32_t ch;

	uint64_t offset;
	uint64_t reading_offset;
	uint64_t column;
	uint64_t line;

	SyToken_t token;
} SyScanner_t;

SyScanner_t *
SyScanner_Create(char *path);

void
SyScanner_Destroy(SyScanner_t *scanner);

int32_t
SyScanner_Advance(SyScanner_t *scanner);

int32_t
SyScanner_Gt(SyScanner_t *scanner);

#endif // __scanner_H__
