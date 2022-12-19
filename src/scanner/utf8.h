#pragma once
#ifndef __UTF8_H__

void 
utf8_decode(char *p, int32_t *r, int32_t *size);

#define utf8_error 65533

#endif