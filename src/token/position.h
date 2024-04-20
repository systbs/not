#pragma once

#ifndef __POSITION_H__
#define __POSITION_H__

typedef struct position
{
	char *path;
	uint64_t offset;
	uint64_t line;
	uint64_t column;
	uint64_t length;
} position_t;

#endif