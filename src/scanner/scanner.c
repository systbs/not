#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../parser/error.h"
#include "../program.h"
#include "scanner.h"
#include "utf8.h"

#define max(a, b) a > b ? a : b
#define isoctal(c) (c >= '0' && c <= '7')
#define isbinary(c) (c == '0' || c == '1')
#define bom 0xFEFF
#define eof -1

int32_t
digit_val(int32_t ch)
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

void 
scanner_set_token(program_t *program, scanner_t *scanner, token_t token)
{
	//printf("%s-%lld:%lld  %s\n", token.position.path, token.position.line, token.position.column, token_get_name(token.type));
	scanner->token = token;
}

scanner_t *
scanner_create(program_t *program, char *path)
{
	scanner_t *scanner = (scanner_t *)malloc(sizeof(scanner_t));

	if (scanner == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(scanner_t));
		return NULL;
	}
	memset(scanner, 0, sizeof(scanner_t));

	strcpy(scanner->path, path);

	FILE *fd = fopen(scanner->path, "rb");

    if (fd == NULL)
	{
        fprintf(stderr, "could not open(%s)\n", scanner->path);
        return NULL;
    }
    
    int64_t pos = ftell(fd);
    fseek(fd, 0, SEEK_END);
    int64_t chunk = ftell(fd);
    fseek(fd, pos, SEEK_SET);

    char *buf = malloc(chunk + 1);
    if (buf == NULL)
	{
    	fprintf(stderr, "unable to allocted a block of %lu bytes", chunk);
        return NULL;
    }

	int64_t i;
    if ((i = fread(buf, 1, chunk, fd)) < chunk)
	{
        fprintf(stderr, "read returned %ld\n", i);
        return NULL;
    }

    buf[i] = '\0';

    fclose(fd);

	scanner->source = buf;
	scanner->offset = 0;
	scanner->reading_offset = 0;
	scanner->line = 1;
	scanner->column = 1;
	scanner->ch = ' ';

	scanner_advance(program, scanner);

	return scanner;
}

void 
scanner_destroy(program_t *program, scanner_t *scanner)
{
	free(scanner);
}

static error_t *
scanner_error(program_t *program, position_t position, const char *format, ...)
{
	char *message;
	message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	va_list arg;
	if (format)
	{
		va_start(arg, format);
		vsprintf(message, format, arg);
		va_end(arg);
	}

	error_t *error = error_create(position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(program->errors, error))
	{
		return NULL;
	}

	return error;
}

static int32_t
scanner_next(program_t *program, scanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		scanner->column += scanner->reading_offset - scanner->offset;
		scanner->offset = scanner->reading_offset;
		int32_t r = (int32_t)(scanner->source[scanner->reading_offset]);
		int32_t w = 1;

		scanner->column += ((r == '\t') ? 3 : 0);
		//printf ("%lld:%lld ch %c %d\n", scanner->line, scanner->column, r, r);

		if (r == 0)
		{
			scanner_error(program, (position_t){
				.path = scanner->path, 
				.offset = scanner->offset, 
				.column = scanner->column, 
				.line = scanner->line,
				.length = 1}, "illegal character NUL");
			return -1;
		}
		if (r >= 0x80)
		{
			utf8_decode(scanner->source + scanner->reading_offset, &r, &w);
			if (r == utf8_error && w == 1)
			{
				scanner_error(program, (position_t){
					.path = scanner->path, 
					.offset = scanner->offset, 
					.column = scanner->column, 
					.line = scanner->line,
					.length = 1}, "illegal UTF-8 encoding");
				return -1;
			}
			else if (r == bom && scanner->offset > 0)
			{
				scanner_error(program, (position_t){
					.path = scanner->path, 
					.offset = scanner->offset, 
					.column = scanner->column, 
					.line = scanner->line,
					.length = 1}, "illegal byte order mark");
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
scanner_peek(program_t *program, scanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		return scanner->source[scanner->reading_offset];
	}
	return 0;
}

static int32_t
scanner_skip_trivial(program_t *program, scanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if (scanner->ch == '\n' || scanner->ch == '\v' || scanner->ch == '\r')
		{
			scanner->column = 0;
			scanner->line++;

			int32_t ch = scanner->ch;
			if (scanner_next(program, scanner) == -1)
			{
				return -1;
			}
			if (ch == '\r' && scanner->ch == '\n')
			{
				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
			}
			continue;
		}

		if (scanner->ch == '\t' || isspace(scanner->ch))
		{
			if (scanner_next(program, scanner) == -1)
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
scanner_advance(program_t *program, scanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if(scanner_skip_trivial(program, scanner) == -1)
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
				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				int32_t n = 0;
				while (true)
				{
					char ch = scanner->ch;
					if (ch == '\n' || ch < 0)
					{
						scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "rune literal not terminated");
						return -1;
					}
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					if (ch == '\'')
					{
						if (n == 0)
						{
							scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "empty rune literal or unescaped '");
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
							if (scanner_next(program, scanner) == -1)
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
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							m = 2;
							base = 16;
							max = 255;
							break;

						case 'u':
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							m = 4;
							base = 16;
							max = 1114111; // unicode max rune
							break;

						case 'U':
							if (scanner_next(program, scanner) == -1)
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
								scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
								return -1;
							}
							scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "unknown escape sequence");
							return -1;
						}

						uint32_t x = 0;
						while (m > 0)
						{
							int32_t d = (int32_t)(digit_val(scanner->ch));
							if (d >= base)
							{
								if (scanner->ch < 0)
								{
									scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
									return -1;
								}
								scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "illegal character %c in escape sequence", scanner->ch);
								return -1;
							}
							x = x * base + d;
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							m--;
						}
						if ((x > (uint32_t)max) || ((0xD800 <= x) && (x < 0xE000)))
						{
							scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence is invalid Unicode code point");
							return -1;
						}
					}
				}
				if (n != 1)
				{
					scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "more than one character in rune literal");
					return -1;
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					fprintf(stderr, "unable to allocted a block of %llu bytes",
									sizeof(char) * (scanner->offset - start_offset));
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				scanner_set_token(program, scanner, (token_t){
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
			else if (scanner->ch == '"' || scanner->ch == '`')
			{
				uint64_t start_offset = scanner->offset + 1;
				char c = scanner->ch;
				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				while (scanner->ch != c)
				{
					if ((scanner->ch == '\n' || scanner->ch == '\r') && c != '`')
					{
						scanner_error(program, (position_t){.path = scanner->path, .offset = scanner->offset - 1, .column = scanner->column - (scanner->offset - start_offset) - 1, .line = scanner->line}, "newline in string");
						return -1;
					}
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					fprintf(stderr, "unable to allocted a block of %llu bytes",
									sizeof(char) * (scanner->offset - start_offset));
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_STRING,
																			 .value = data,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = start_offset - 1,
																					 .column = scanner->column - (scanner->offset - start_offset) - 1,
																					 .line = scanner->line,
																					 .length = scanner->offset - start_offset}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '#')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_HASH,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '$')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_DOLLER,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '(')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_LPAREN,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ')')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_RPAREN,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '[')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_LBRACKET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ']')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_RBRACKET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '{')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_LBRACE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '}')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_RBRACE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ',')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_COMMA,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '.')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_DOT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
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
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_COLON,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
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
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_SEMICOLON,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '?')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_QUESTION,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '@')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_AT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_BACKSLASH,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_BACKSLASH_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '_')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_UNDERLINE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_PLUS,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_PLUS_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_MINUS,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_MINUS_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '>')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_MINUS_GT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_STAR,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_STAR_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '*')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_POWER,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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
				if ((c = scanner_peek(program, scanner)) && c == '/')
				{
					while (scanner->ch != '\n')
					{
						if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
					}
					continue;
				}
				else if ((c = scanner_peek(program, scanner)) && c == '*')
				{
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					int64_t depth = 0;
					while (true)
					{
						if (scanner->ch == '*')
						{
							if ((c = scanner_peek(program, scanner)) && c == '/' && depth < 1)
							{
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
								break;
							}
							depth--;
						}
						if (scanner->ch == '\n')
						{
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '\r')
						{
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '/')
						{
							if ((c = scanner_peek(program, scanner)) && c == '*')
							{
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
								depth++;
								continue;
							}
						}
						if (scanner_next(program, scanner) == -1)
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

					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_SLASH,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = scanner->offset,
																						 .column = scanner->column,
																						 .line = scanner->line,
																					 .length = 1}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}

					if(scanner_skip_trivial(program, scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						scanner_set_token(program, scanner, (token_t){
																					 .type = TOKEN_SLASH_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line,
																						 	.length = 2}});

						if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_PERCENT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_PERCENT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_AND,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '&')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_AND_AND,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_AND_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_OR,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '|')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_OR_OR,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_OR_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					return 1;
				}
				return 1;
			}
			else if (scanner->ch == '^')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_CARET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '~')
			{
				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_TILDE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_LT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '<')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_LT_LT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}

					if(scanner_skip_trivial(program, scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						scanner_set_token(program, scanner, (token_t){
																					 .type = TOKEN_LT_LT_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line,
																						 .length = 3}});

						if (scanner_next(program, scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_LT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_GT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '>')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_GT_GT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}

					if(scanner_skip_trivial(program, scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						scanner_set_token(program, scanner, (token_t){
																					 .type = TOKEN_GT_GT_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line,
																						 .length = 2}});

						if (scanner_next(program, scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_GT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_NOT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_NOT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
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

				scanner_set_token(program, scanner, (token_t){
																			 .type = TOKEN_EQ,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(program, scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(program, scanner, (token_t){
																				 .type = TOKEN_EQ_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
				}
				return 1;
			}
			else
			{
				scanner_error(program, (position_t){
					.path = scanner->path, 
					.offset = scanner->offset, 
					.column = scanner->column, 
					.line = scanner->line, 
					.length = 1}, "unknown token");
				return -1;
			}
		}
		else 
		if (isdigit(scanner->ch))
		{
			uint64_t start_offset = scanner->offset;

			if (scanner->ch == '0')
			{
				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				switch (tolower(scanner->ch))
				{
				case 'x':
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isxdigit(scanner->ch))
							{
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (scanner->ch == 'j')
								{
									if (scanner_next(program, scanner) == -1)
									{
										return -1;
									}
									break;
								}

								if (isalpha(scanner->ch))
								{
									scanner_error(program, (position_t){
										.path = scanner->path, 
										.offset = scanner->offset, 
										.column = scanner->column - (scanner->offset - start_offset), 
										.line = scanner->line,
										.length = scanner->offset - start_offset}, "wrong hexadecimal number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(program, (position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "hexadecimal literal has no digits");
						return -1;
					}
					break;

				case 'b':
					if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isbinary(scanner->ch))
							{
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (scanner->ch == 'j')
								{
									if (scanner_next(program, scanner) == -1)
									{
										return -1;
									}
									break;
								}

								if (isalpha(scanner->ch))
								{
									scanner_error(program, (position_t){
										.path = scanner->path, 
										.offset = scanner->offset, 
										.column = scanner->column - (scanner->offset - start_offset), 
										.line = scanner->line,
										.length = scanner->offset - start_offset}, "wrong binary number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(program, (position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "binary literal has no digits");
						return -1;
					}
					break;

				case 'o':
					if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isoctal(scanner->ch))
							{
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (scanner->ch == 'j')
								{
									if (scanner_next(program, scanner) == -1)
									{
										return -1;
									}
									break;
								}

								if (isalpha(scanner->ch))
								{
									scanner_error(program, (position_t){
										.path = scanner->path, 
										.offset = scanner->offset, 
										.column = scanner->column - (scanner->offset - start_offset), 
										.line = scanner->line,
										.length = scanner->offset - start_offset}, "wrong octal number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(program, (position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "octal literal has no digits");
						return -1;
					}
					break;

				case '.':
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					int is_float = 1;
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isdigit(scanner->ch) || scanner->ch == '.')
							{
								if (scanner->ch == '.')
								{
									if (is_float > 0)
									{
										scanner_error(program, (position_t){
											.path = scanner->path, 
											.offset = scanner->offset, 
											.column = scanner->column - (scanner->offset - start_offset), 
											.line = scanner->line,
											.length = scanner->offset - start_offset}, "wrong decimal number format");
										return -1;
									}
									is_float++;
									if (scanner_next(program, scanner) == -1)
									{
										return -1;
									}
									continue;
								}
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (scanner->ch == 'j')
								{
									if (scanner_next(program, scanner) == -1)
									{
										return -1;
									}
									break;
								}

								if (isalpha(scanner->ch))
								{
									scanner_error(program, (position_t){
										.path = scanner->path, 
										.offset = scanner->offset, 
										.column = scanner->column - (scanner->offset - start_offset), 
										.line = scanner->line,
										.length = scanner->offset - start_offset}, "wrong number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(program, (position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "decimal literal has no digits");
						return -1;
					}
					break;

				default:
					if (!isoctal(scanner->ch))
					{
						break;
					}
					if (scanner_next(program, scanner) == -1)
					{
						return -1;
					}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isoctal(scanner->ch))
							{
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (scanner->ch == 'j')
								{
									if (scanner_next(program, scanner) == -1)
									{
										return -1;
									}
									break;
								}
								
								if (isalpha(scanner->ch))
								{
									scanner_error(program, (position_t){
										.path = scanner->path, 
										.offset = scanner->offset, 
										.column = scanner->column - (scanner->offset - start_offset), 
										.line = scanner->line,
										.length = scanner->offset - start_offset}, "wrong octal number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(program, (position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "octal literal has no digits");
						return -1;
					}
					break;
				}
			}
			else
			{
				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
				int is_float = 0;
				int is_exp = 0;
				int sign = 0;
				while (!isspace(scanner->ch))
				{
					if (isdigit(scanner->ch) || scanner->ch == '.' || tolower(scanner->ch) == 'e')
					{
						if (tolower(scanner->ch) == 'e')
						{
							if (is_exp > 0)
							{
								scanner_error(program, (position_t){
									.path = scanner->path, 
									.offset = scanner->offset, 
									.column = scanner->column - (scanner->offset - start_offset), 
									.line = scanner->line,
									.length = scanner->offset - start_offset}, "wrong exponent format");
								return -1;
							}
							is_exp++;
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							if (scanner->ch == '+' || scanner->ch == '-')
							{
								if (sign > 0)
								{
									break;
								}
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
								sign++;
							}
							if (!isdigit(scanner->ch))
							{
								scanner_error(program, (position_t){
									.path = scanner->path, 
									.offset = scanner->offset, 
									.column = scanner->column - (scanner->offset - start_offset), 
									.line = scanner->line,
									.length = scanner->offset - start_offset}, "exponent has no digits");
								return -1;
							}
							continue;
						}
						if (scanner->ch == '.')
						{
							if (is_float > 0)
							{
								break;
							}
							int32_t ch;
							ch = scanner_peek(program, scanner);
							if (isdigit(ch))
							{
								is_float++;
								if (scanner_next(program, scanner) == -1)
								{
									return -1;
								}
								continue;
							}
							break;
						}
						if (scanner_next(program, scanner) == -1)
						{
							return -1;
						}
					}
					else
					{
						if (scanner->ch == 'j')
						{
							if (scanner_next(program, scanner) == -1)
							{
								return -1;
							}
							break;
						}

						if (isalpha(scanner->ch))
						{
							scanner_error(program, (position_t){
								.path = scanner->path, 
								.offset = scanner->offset, 
								.column = scanner->column - (scanner->offset - start_offset), 
								.line = scanner->line,
								.length = scanner->offset - start_offset}, "wrong number format");
							return -1;
						}
						break;
					}
				}
			}

			char *data;
			if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
			{
				fprintf(stderr, "unable to allocted a block of %llu bytes", (scanner->offset - start_offset));
				return -1;
			}
			strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
			data[scanner->offset - start_offset] = '\0';

			scanner_set_token(program, scanner, (token_t){
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
				if (scanner_next(program, scanner) == -1)
				{
					return -1;
				}
			}
			uint32_t length = scanner->offset - start_offset;

			if (strncmp(scanner->source + start_offset, "for", max(length, 3)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_FOR_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "this", max(length, 4)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_THIS_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "class", max(length, 5)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_CLASS_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "extends", max(length, 7)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_EXTENDS_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "static", max(length, 6)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_STATIC_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "readonly", max(length, 8)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_READONLY_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "reference", max(length, 9)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_REFERENCE_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "override", max(length, 8)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_OVERRIDE_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "if", max(length, 2)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_IF_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "else", max(length, 4)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_ELSE_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "try", max(length, 3)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_TRY_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "catch", max(length, 5)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_CATCH_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "throw", max(length, 5)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_THROW_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "break", max(length, 5)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_BREAK_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "continue", max(length, 8)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_CONTINUE_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "return", max(length, 6)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_RETURN_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "sizeof", max(length, 6)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_SIZEOF_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "typeof", max(length, 6)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_TYPEOF_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "null", max(length, 4)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_NULL_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "fun", max(length, 3)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_FUN_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "var", max(length, 3)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_VAR_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "enum", max(length, 4)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_ENUM_KEYWORD,
					.value = NULL,
					.position = {
						.path = scanner->path,
						.offset = scanner->offset - length,
						.column = scanner->column - length,
						.line = scanner->line,
							.length = length
					}
				});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "export", max(length, 6)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_EXPORT_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "using", max(length, 5)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_USING_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "from", max(length, 4)) == 0)
			{
				scanner_set_token(program, scanner, (token_t){
					.type = TOKEN_FROM_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset - length,
							.column = scanner->column - length,
							.line = scanner->line,
							.length = length
						}
					});

				return 1;
			}

			char *data;
			if (!(data = malloc(sizeof(char) * (length))))
			{
				fprintf(stderr, "unable to allocted a block of %zu bytes",
					sizeof(char) * (length));
				return -1;
			}
			strncpy(data, scanner->source + start_offset, length);
			data[length] = '\0';

			scanner_set_token(program, scanner, (token_t){
				.type = TOKEN_ID,
				.value = data,
				.position = {
						.path = scanner->path,
						.offset = scanner->offset - length,
						.column = scanner->column - length,
						.line = scanner->line,
						.length = length
					}
				});

			return 1;
		}

		if (scanner_next(program, scanner) == -1)
		{
			return -1;
		}
	}

	scanner_set_token(program, scanner, (token_t){
		.type = TOKEN_EOF,
		.value = NULL,
		.position = {
				.path = scanner->path,
				.offset = scanner->offset,
				.column = scanner->column,
				.line = scanner->line,
				.length = 1
			}
		});

	return 1;
}

int32_t
scanner_gt (program_t *program, scanner_t *scanner)
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
		if (scanner_next(program, scanner) == -1)
		{
			return -1;
		}

		if(scanner_skip_trivial(program, scanner) == -1)
		{
			return -1;
		}
	}

	return 1;
}