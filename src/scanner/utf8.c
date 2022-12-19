#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../types/types.h"
#include "utf8.h"

#define t1  0
#define tx  128
#define t2  192
#define t3  224
#define t4  240
#define t5  248

#define maskx  63
#define mask2  31
#define mask3  15
#define mask4  7

// The default lowest and highest continuation byte.
#define locb 128
#define hicb 191

/*  These names of these constants are chosen to give nice alignment in the
    table below. The first nibble is an index into acceptRanges or F for
    special one-byte cases. The second nibble is the Rune length or the
    Status for the special one-byte case. */

#define xx 0xF1 // invalid: size 1
#define as 0xF0 // ASCII: size 1
#define s1 0x02 // accept 0, size 2
#define s2 0x13 // accept 1, size 3
#define s3 0x03 // accept 0, size 3
#define s4 0x23 // accept 2, size 3
#define s5 0x34 // accept 3, size 4
#define s6 0x04 // accept 0, size 4
#define s7 0x44 // accept 4, size 4


// first is information about the first byte in a UTF-8 sequence.
int32_t first[256] = {
	//   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x00-0x0F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x10-0x1F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x20-0x2F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x30-0x3F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x40-0x4F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x50-0x5F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x60-0x6F
	as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, // 0x70-0x7F
	//   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, // 0x80-0x8F
	xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, // 0x90-0x9F
	xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, // 0xA0-0xAF
	xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, // 0xB0-0xBF
	xx, xx, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, // 0xC0-0xCF
	s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, // 0xD0-0xDF
	s2, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s4, s3, s3, // 0xE0-0xEF
	s5, s6, s6, s6, s7, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, // 0xF0-0xFF
};

// acceptRange gives the range of valid values for the second byte in a UTF-8
// sequence.
typedef struct acceptRange {
	uint8_t lo; // lowest value for second byte.
	uint8_t hi; // highest value for second byte.
} acceptRange_t;

// acceptRanges has size 16 to avoid bounds checks in the code that uses it.
const acceptRange_t acceptRanges[5] = {
	[0]= {locb, hicb},
	[1]= {0xA0, hicb},
	[2]= {locb, 0x9F},
	[3]= {0x90, hicb},
	[4]= {locb, 0x8F},
};

void 
utf8_decode(char *p, int32_t *r, int32_t *w) {
	size_t n = strlen(p);
	if (n < 1) {
        *r = utf8_error;
        *w = 0;
		return;
	}
	int32_t p0 = (int32_t)p[0];
	int32_t x = first[p0];
	if (x >= as) {
		// The following code simulates an additional check for x == xx and
		// handling the ASCII and invalid cases accordingly. This mask-and-or
		// approach prevents an additional branch.
		int32_t mask = (int32_t)(x) << 31 >> 31; // Create 0x0000 or 0xFFFF.
        *r = (int32_t)(((p[0]) & ~mask) | (utf8_error & mask));
        *w = 1;
		return;
	}
	int32_t sz = (int32_t)(x & 7);
	acceptRange_t accept = acceptRanges[x>>4];
	if (n < (size_t)sz) {
        *r = utf8_error;
        *w = 1;
		return;
	}
	int32_t b1 = (int32_t)p[1];
	if (b1 < accept.lo || accept.hi < b1) {
		*r = utf8_error;
        *w = 1;
		return;
	}
	if (sz <= 2) {
        *r = (int32_t)(p0&mask2)<<6 | (int32_t)(b1&maskx);
        *w = 2;
		return;
	}
	int32_t b2 = (int32_t)p[2];
	if (b2 < locb || hicb < b2) {
		*r = utf8_error;
        *w = 1;
		return;
	}
	if (sz <= 3) {
        *r = (int32_t)(p0&mask3)<<12 | (int32_t)(b1&maskx)<<6 | (int32_t)(b2&maskx);
        *w = 3;
		return;
	}
	int32_t b3 = (int32_t)p[3];
	if (b3 < locb || hicb < b3) {
		*r = utf8_error;
        *w = 1;
		return;
	}
    *r = (int32_t)(p0&mask4)<<18 | (int32_t)(b1&maskx)<<12 | (int32_t)(b2&maskx)<<6 | (int32_t)(b3&maskx);
    *w = 4;
	return;
}