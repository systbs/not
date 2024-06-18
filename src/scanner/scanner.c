#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <float.h>

#include "../types/types.h"
#include "../utils/utils.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../error.h"
#include "../memory.h"
#include "../mutex.h"
#include "../config.h"
#include "scanner.h"
#include "utf8.h"

#if defined(_WIN32) || defined(_WIN64)
#else
#define max(a, b) a > b ? a : b
#endif

#define isoctal(c) (c >= '0' && c <= '7')
#define isbinary(c) (c == '0' || c == '1')
#define bom 0xFEFF
#define eof -1

int32_t
not_scanner_digit_value(int32_t ch)
{
	if ('0' <= ch && ch <= '9')
	{
		return (int32_t)(ch - '0');
	}
	else if ('a' <= tolower(ch) && tolower(ch) <= 'f')
	{
		return (int32_t)(tolower(ch) - 'a' + 10);
	}
	return 16;
}

void not_scanner_set_token(not_scanner_t *scanner, not_token_t token)
{
	// printf("%s-%lld:%lld  %s\n", token.position.path, token.position.line, token.position.column, not_token_get_name(token.type));
	scanner->token = token;
}

not_scanner_t *
not_scanner_create(char *path)
{
	not_scanner_t *scanner = (not_scanner_t *)not_memory_calloc(1, sizeof(not_scanner_t));
	if (scanner == NOT_PTR_NULL)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	strcpy(scanner->path, path);

	FILE *fd = fopen(scanner->path, "rb");
	if (fd == NOT_PTR_NULL)
	{
		not_error_system("could not open(%s)\n", scanner->path);
		return NOT_PTR_ERROR;
	}

	int64_t pos = ftell(fd);
	fseek(fd, 0, SEEK_END);
	int64_t chunk = ftell(fd);
	fseek(fd, pos, SEEK_SET);

	char *buf = not_memory_calloc(1, chunk + 1);
	if (buf == NOT_PTR_NULL)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	int64_t i;
	if ((i = fread(buf, 1, chunk, fd)) < chunk)
	{
		not_error_system("read returned %ld\n", i);
		return NOT_PTR_ERROR;
	}

	buf[i] = '\0';

	fclose(fd);

	scanner->source = buf;
	scanner->offset = 0;
	scanner->reading_offset = 0;
	scanner->line = 1;
	scanner->column = 1;
	scanner->ch = ' ';

	not_scanner_advance(scanner);

	return scanner;
}

not_scanner_t *
not_scanner_create_from_buffer(char *buf, char *path)
{
	not_scanner_t *scanner = (not_scanner_t *)not_memory_calloc(1, sizeof(not_scanner_t));
	if (scanner == NOT_PTR_NULL)
	{
		not_error_no_memory();
		return NOT_PTR_NULL;
	}

	strcpy(scanner->path, path);

	scanner->source = buf;
	scanner->offset = 0;
	scanner->reading_offset = 0;
	scanner->line = 1;
	scanner->column = 1;
	scanner->ch = ' ';

	not_scanner_advance(scanner);

	return scanner;
}

void not_scanner_destroy(not_scanner_t *scanner)
{
	free(scanner);
}

static int32_t
not_scanner_next(not_scanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		scanner->column += scanner->reading_offset - scanner->offset;
		scanner->offset = scanner->reading_offset;
		int32_t r = (int32_t)(scanner->source[scanner->reading_offset]);
		int32_t w = 1;

		scanner->column += ((r == '\t') ? 3 : 0);

		if (r == 0)
		{
			not_error_lexer_by_position((not_position_t){
											.path = scanner->path,
											.offset = scanner->offset,
											.column = scanner->column,
											.line = scanner->line,
											.length = 1},
										"illegal character NUL");
			return -1;
		}
		if (r >= 0x80)
		{
			not_utf8_decode(scanner->source + scanner->reading_offset, &r, &w);
			if (r == utf8_error && w == 1)
			{
				not_error_lexer_by_position((not_position_t){
												.path = scanner->path,
												.offset = scanner->offset,
												.column = scanner->column,
												.line = scanner->line,
												.length = 1},
											"illegal UTF-8 encoding");
				return -1;
			}
			else if (r == bom && scanner->offset > 0)
			{
				not_error_lexer_by_position((not_position_t){
												.path = scanner->path,
												.offset = scanner->offset,
												.column = scanner->column,
												.line = scanner->line,
												.length = 1},
											"illegal byte order mark");
				return -1;
			}
		}
		scanner->reading_offset += w;
		scanner->ch = r;
		return 1;
	}
	else
	{
		scanner->offset = strlen(scanner->source);
		scanner->ch = eof;
		return 1;
	}
}

static char
not_scanner_peek(not_scanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		return scanner->source[scanner->reading_offset];
	}
	return 0;
}

static int32_t
not_scanner_skip_trivial(not_scanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if (scanner->ch == '\n' || scanner->ch == '\v' || scanner->ch == '\r')
		{
			scanner->column = 0;
			scanner->line++;

			int32_t ch = scanner->ch;
			if (not_scanner_next(scanner) == -1)
			{
				return -1;
			}
			if (ch == '\r' && scanner->ch == '\n')
			{
				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
			}
			continue;
		}

		if (scanner->ch == '\t' || isspace(scanner->ch))
		{
			if (not_scanner_next(scanner) == -1)
			{
				return -1;
			}
			continue;
		}
		break;
	}
	return 1;
}

int32_t
not_scanner_advance(not_scanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if (not_scanner_skip_trivial(scanner) == -1)
		{
			return -1;
		}

		if (scanner->ch == eof)
		{
			break;
		}

		if (!isalpha(scanner->ch) && !isdigit(scanner->ch) && scanner->ch != '_')
		{
			if (scanner->ch == '\'')
			{
				uint64_t start_offset = scanner->offset + 1;
				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				int32_t n = 0;
				while (true)
				{
					char ch = scanner->ch;
					if (ch == '\n' || ch < 0)
					{
						not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "rune literal not terminated");
						return -1;
					}
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					if (ch == '\'')
					{
						if (n == 0)
						{
							not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "empty rune literal or unescaped '");
							return -1;
						}
						break;
					}
					n++;
					if (ch == '\\')
					{
						int32_t m;
						int32_t base = 8, max = 255;

						switch (scanner->ch)
						{
						case 'a':
						case 'b':
						case 'f':
						case 'n':
						case 'r':
						case 't':
						case 'v':
						case '\\':
						case '\'':
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							break;

						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
							m = 3;
							base = 8;
							max = 255;
							break;

						case 'x':
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							m = 2;
							base = 16;
							max = 255;
							break;

						case 'u':
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							m = 4;
							base = 16;
							max = 1114111; // unicode max rune
							break;

						case 'U':
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							m = 8;
							base = 16;
							max = 1114111;
							break;

						default:
							if (scanner->ch < 0)
							{
								not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
								return -1;
							}
							not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "unknown escape sequence");
							return -1;
						}

						uint32_t x = 0;
						while (m > 0)
						{
							int32_t d = (int32_t)(not_scanner_digit_value(scanner->ch));
							if (d >= base)
							{
								if (scanner->ch < 0)
								{
									not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
									return -1;
								}
								not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "illegal character %c in escape sequence", scanner->ch);
								return -1;
							}
							x = x * base + d;
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							m--;
						}
						if ((x > (uint32_t)max) || ((0xD800 <= x) && (x < 0xE000)))
						{
							not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence is invalid Unicode code point");
							return -1;
						}
					}
				}
				if (n != 1)
				{
					not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "more than one character in rune literal");
					return -1;
				}

				char *data = not_memory_calloc(1, (scanner->offset - start_offset) + 1);
				if (data == NOT_PTR_NULL)
				{
					not_error_no_memory();
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_CHAR,
												   .value = data,
												   .position = {
													   .path = scanner->path,
													   .offset = start_offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = scanner->offset - start_offset}});

				return 1;
			}
			else if (scanner->ch == '"')
			{
				uint64_t start_offset = scanner->offset + 1;
				char c = scanner->ch;
				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				while (scanner->ch != c)
				{
					if ((scanner->ch == '\n' || scanner->ch == '\r') && c != '"')
					{
						not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset - 1, .column = scanner->column - (scanner->offset - start_offset) - 1, .line = scanner->line}, "newline in string");
						return -1;
					}
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
				}

				char *data = not_memory_calloc(1, (scanner->offset - start_offset) + 1);
				if (data == NOT_PTR_NULL)
				{
					not_error_no_memory();
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_STRING,
												   .value = data,
												   .position = {
													   .path = scanner->path,
													   .offset = start_offset - 1,
													   .column = scanner->column - (scanner->offset - start_offset) - 1,
													   .line = scanner->line,
													   .length = scanner->offset - start_offset}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '`')
			{
				uint64_t start_offset = scanner->offset + 1;
				char c = scanner->ch;
				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				while (scanner->ch != c)
				{
					if ((scanner->ch == '\n' || scanner->ch == '\r') && c != '`')
					{
						not_error_lexer_by_position((not_position_t){.path = scanner->path, .offset = scanner->offset - 1, .column = scanner->column - (scanner->offset - start_offset) - 1, .line = scanner->line}, "newline in string");
						return -1;
					}
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
				}

				char *data = not_memory_calloc(1, (scanner->offset - start_offset) + 1);
				if (data == NOT_PTR_NULL)
				{
					not_error_no_memory();
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_STRING,
												   .value = data,
												   .position = {
													   .path = scanner->path,
													   .offset = start_offset - 1,
													   .column = scanner->column - (scanner->offset - start_offset) - 1,
													   .line = scanner->line,
													   .length = scanner->offset - start_offset}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '#')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_HASH,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '$')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_DOLLER,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '(')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_LPAREN,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ')')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_RPAREN,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '[')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_LBRACKET,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ']')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_RBRACKET,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '{')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_LBRACE,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '}')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_RBRACE,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ',')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_COMMA,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '.')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_DOT,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				return 1;
			}
			else if (scanner->ch == ':')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_COLON,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				return 1;
			}
			else if (scanner->ch == ';')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_SEMICOLON,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '?')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_QUESTION,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '@')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_AT,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '\\')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_BACKSLASH,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_BACKSLASH_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '_')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_UNDERLINE,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '+')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_PLUS,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_PLUS_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '-')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_MINUS,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_MINUS_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '>')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_MINUS_GT,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '*')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_STAR,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}

				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_STAR_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '*')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_POWER,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					if (not_scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}

					if (scanner->ch == eof)
					{
						break;
					}
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_POWER_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '/')
			{
				char c;
				if ((c = not_scanner_peek(scanner)) && c == '/')
				{
					while (scanner->ch != '\n')
					{
						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
					}
					continue;
				}
				else if ((c = not_scanner_peek(scanner)) && c == '*')
				{
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					int64_t depth = 0;
					while (true)
					{
						if (scanner->ch == '*')
						{
							if ((c = not_scanner_peek(scanner)) && c == '/' && depth < 1)
							{
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
								break;
							}
							depth--;
						}
						if (scanner->ch == '\n')
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '\r')
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '/')
						{
							if ((c = not_scanner_peek(scanner)) && c == '*')
							{
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
								depth++;
								continue;
							}
						}
						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
					}
					continue;
				}
				else
				{
					uint64_t offset = scanner->offset;
					uint64_t column = scanner->column;
					uint64_t line = scanner->line;

					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_SLASH,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = scanner->offset,
														   .column = scanner->column,
														   .line = scanner->line,
														   .length = 1}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					if (not_scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						not_scanner_set_token(scanner, (not_token_t){
														   .type = TOKEN_SLASH_EQ,
														   .value = NOT_PTR_NULL,
														   .position = {
															   .path = scanner->path,
															   .offset = offset,
															   .column = column,
															   .line = line,
															   .length = 2}});

						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
						return 1;
					}

					return 1;
				}
			}
			else if (scanner->ch == '%')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_PERCENT,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_PERCENT_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}
				return 1;
			}
			else if (scanner->ch == '&')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_AND,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '&')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_AND_AND,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_AND_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '|')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_OR,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '|')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_OR_OR,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_OR_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}
				return 1;
			}
			else if (scanner->ch == '^')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_CARET,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '~')
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_TILDE,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '<')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_LT,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '<')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_LT_LT,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					if (not_scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						not_scanner_set_token(scanner, (not_token_t){
														   .type = TOKEN_LT_LT_EQ,
														   .value = NOT_PTR_NULL,
														   .position = {
															   .path = scanner->path,
															   .offset = offset,
															   .column = column,
															   .line = line,
															   .length = 3}});

						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_LT_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
				}
				return 1;
			}
			else if (scanner->ch == '>')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_GT,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '>')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_GT_GT,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					if (not_scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						not_scanner_set_token(scanner, (not_token_t){
														   .type = TOKEN_GT_GT_EQ,
														   .value = NOT_PTR_NULL,
														   .position = {
															   .path = scanner->path,
															   .offset = offset,
															   .column = column,
															   .line = line,
															   .length = 2}});

						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_GT_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
				}

				return 1;
			}
			else if (scanner->ch == '!')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_NOT,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_NOT_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
				}
				return 1;
			}
			else if (scanner->ch == '=')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_EQ,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset,
													   .column = scanner->column,
													   .line = scanner->line,
													   .length = 1}});

				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (not_scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					not_scanner_set_token(scanner, (not_token_t){
													   .type = TOKEN_EQ_EQ,
													   .value = NOT_PTR_NULL,
													   .position = {
														   .path = scanner->path,
														   .offset = offset,
														   .column = column,
														   .line = line,
														   .length = 2}});

					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}
				}
				return 1;
			}
			else
			{
				not_error_lexer_by_position((not_position_t){
												.path = scanner->path,
												.offset = scanner->offset,
												.column = scanner->column,
												.line = scanner->line,
												.length = 1},
											"unknown token");
				return -1;
			}
		}
		else if (isdigit(scanner->ch))
		{
			uint64_t start_offset = scanner->offset;

			if (scanner->ch == '0')
			{
				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}

				if (tolower(scanner->ch) == 'x')
				{
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					while (!isspace(scanner->ch))
					{
						if (isxdigit(scanner->ch))
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						{
							break;
						}
					}
				}
				else if (tolower(scanner->ch) == 'b')
				{
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					while (!isspace(scanner->ch))
					{
						if (isbinary(scanner->ch))
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						{
							break;
						}
					}
				}
				else if (tolower(scanner->ch) == 'o')
				{
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					while (!isspace(scanner->ch))
					{
						if (isoctal(scanner->ch))
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						{
							break;
						}
					}
				}
				else if (tolower(scanner->ch) == '.')
				{
					if (not_scanner_next(scanner) == -1)
					{
						return -1;
					}

					while (!isspace(scanner->ch))
					{
						if (tolower(scanner->ch) == 'e')
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}

							if (tolower(scanner->ch) == '+')
							{
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else if (tolower(scanner->ch) == '-')
							{
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								not_error_lexer_by_position((not_position_t){
																.path = scanner->path,
																.offset = scanner->offset,
																.column = scanner->column - (scanner->offset - start_offset),
																.line = scanner->line,
																.length = scanner->offset - start_offset},
															"wrong number format");
								return -1;
							}

							while (!isspace(scanner->ch))
							{
								if (isdigit(scanner->ch))
								{
									if (not_scanner_next(scanner) == -1)
									{
										return -1;
									}
								}
								else
								{
									break;
								}
							}

							break;
						}
						else if (scanner->ch == '.')
						{
							break;
						}
						if (isdigit(scanner->ch))
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					while (!isspace(scanner->ch))
					{
						if (isoctal(scanner->ch))
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						{
							break;
						}
					}
				}
			}
			else
			{
				int is_float = 0;
				while (!isspace(scanner->ch))
				{
					if (tolower(scanner->ch) == 'e')
					{
						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}

						if (tolower(scanner->ch) == '+')
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
						}
						else if (tolower(scanner->ch) == '-')
						{
							if (not_scanner_next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						{
							break;
						}

						while (!isspace(scanner->ch))
						{
							if (isdigit(scanner->ch))
							{
								if (not_scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								break;
							}
						}

						break;
					}
					else if (scanner->ch == '.')
					{
						if (is_float > 0)
						{
							break;
						}

						is_float += 1;
						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
						continue;
						;
					}
					else if (isdigit(scanner->ch))
					{
						if (not_scanner_next(scanner) == -1)
						{
							return -1;
						}
						continue;
					}
					else
					{
						break;
					}
				}
			}

			char *data = not_memory_calloc(1, (scanner->offset - start_offset) + 1);
			if (data == NOT_PTR_NULL)
			{
				not_error_no_memory();
				return -1;
			}

			strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
			data[scanner->offset - start_offset] = '\0';

			not_scanner_set_token(scanner, (not_token_t){
											   .type = TOKEN_NUMBER,
											   .value = data,
											   .position = {
												   .path = scanner->path,
												   .offset = scanner->offset,
												   .column = scanner->column - (scanner->offset - start_offset),
												   .line = scanner->line,
												   .length = scanner->offset - start_offset}});

			return 1;
		}
		else
		{
			// parse identifier
			uint64_t start_offset = scanner->offset;
			while (isalpha(scanner->ch) || isdigit(scanner->ch) || (scanner->ch == '_'))
			{
				if (not_scanner_next(scanner) == -1)
				{
					return -1;
				}
			}
			uint32_t length = scanner->offset - start_offset;

			if (strncmp(scanner->source + start_offset, "null", max(length, 4)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_NULL_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "int", max(length, 3)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_INT_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "float", max(length, 5)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_FLOAT_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "char", max(length, 4)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_CHAR_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "string", max(length, 6)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_STRING_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "for", max(length, 3)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_FOR_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "class", max(length, 5)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_CLASS_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "extends", max(length, 7)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_EXTENDS_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "static", max(length, 6)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_STATIC_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "readonly", max(length, 8)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_READONLY_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "reference", max(length, 9)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_REFERENCE_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "if", max(length, 2)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_IF_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "else", max(length, 4)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_ELSE_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "try", max(length, 3)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_TRY_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "catch", max(length, 5)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_CATCH_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "throw", max(length, 5)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_THROW_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "break", max(length, 5)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_BREAK_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "continue", max(length, 8)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_CONTINUE_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "return", max(length, 6)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_RETURN_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "sizeof", max(length, 6)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_SIZEOF_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "typeof", max(length, 6)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_TYPEOF_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "instanceof", max(length, 10)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_INSTANCEOF_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "in", max(length, 2)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_IN_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "fun", max(length, 3)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_FUN_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "var", max(length, 3)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_VAR_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "export", max(length, 6)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_EXPORT_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "using", max(length, 5)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_USING_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}
			else if (strncmp(scanner->source + start_offset, "from", max(length, 4)) == 0)
			{
				not_scanner_set_token(scanner, (not_token_t){
												   .type = TOKEN_FROM_KEYWORD,
												   .value = NOT_PTR_NULL,
												   .position = {
													   .path = scanner->path,
													   .offset = scanner->offset - length,
													   .column = scanner->column - length,
													   .line = scanner->line,
													   .length = length}});

				return 1;
			}

			char *data = not_memory_calloc(1, length + 1);
			if (data == NOT_PTR_NULL)
			{
				not_error_no_memory();
				return -1;
			}
			strncpy(data, scanner->source + start_offset, length);
			data[length] = '\0';

			not_scanner_set_token(scanner, (not_token_t){
											   .type = TOKEN_ID,
											   .value = data,
											   .position = {
												   .path = scanner->path,
												   .offset = scanner->offset - length,
												   .column = scanner->column - length,
												   .line = scanner->line,
												   .length = length}});

			return 1;
		}

		if (not_scanner_next(scanner) == -1)
		{
			return -1;
		}
	}

	not_scanner_set_token(scanner, (not_token_t){
									   .type = TOKEN_EOF,
									   .value = NOT_PTR_NULL,
									   .position = {
										   .path = scanner->path,
										   .offset = scanner->offset,
										   .column = scanner->column,
										   .line = scanner->line,
										   .length = 1}});

	return 1;
}

int32_t
not_scanner_gt(not_scanner_t *scanner)
{
	if ((scanner->token.type == TOKEN_GT_EQ) || (scanner->token.type == TOKEN_GT_GT_EQ) || (scanner->token.type == TOKEN_GT_GT))
	{
		scanner->token.type = TOKEN_GT;
		scanner->line = scanner->token.position.line;
		scanner->column = scanner->token.position.column;
		scanner->offset = scanner->token.position.offset;
		int32_t r = (int32_t)(scanner->source[scanner->offset]);
		scanner->ch = r;
		scanner->reading_offset = scanner->offset + 1;
		if (not_scanner_next(scanner) == -1)
		{
			return -1;
		}

		if (not_scanner_skip_trivial(scanner) == -1)
		{
			return -1;
		}
	}

	return 1;
}