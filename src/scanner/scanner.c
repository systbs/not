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
#include "file.h"
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
scanner_set_token(scanner_t *scanner, token_t token)
{
	scanner->token = token;
}

scanner_t *
scanner_create(file_source_t *file_source, list_t *errors)
{
	scanner_t *scanner = (scanner_t *)malloc(sizeof(scanner_t));

	if (scanner == NULL)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(scanner_t));
		return NULL;
	}
	memset(scanner, 0, sizeof(scanner_t));

	scanner->file_source = file_source;
	scanner->errors = errors;

	scanner->source = file_source->text;
	scanner->offset = 0;
	scanner->reading_offset = 0;
	scanner->line = 1;
	scanner->column = 1;
	scanner->ch = ' ';

	scanner_advance(scanner);

	return scanner;
}

void 
scanner_destroy(scanner_t *scanner)
{
	free(scanner);
}

static error_t *
scanner_error(scanner_t *scanner, position_t position, const char *format, ...)
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

	error_t *error;
	error = error_create(position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(scanner->errors, error))
	{
		return NULL;
	}

	return error;
}

static int32_t
scanner_next(scanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		scanner->column += scanner->reading_offset - scanner->offset;
		scanner->offset = scanner->reading_offset;
		int32_t r = (int32_t)(scanner->source[scanner->reading_offset]);
		int32_t w = 1;
		if (r == 0)
		{
			scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column, .line = scanner->line}, "illegal character NUL");
			return -1;
		}
		if (r >= 0x80)
		{
			utf8_decode(scanner->source + scanner->reading_offset, &r, &w);
			if (r == utf8_error && w == 1)
			{
				scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column, .line = scanner->line}, "illegal UTF-8 encoding");
				return -1;
			}
			else if (r == bom && scanner->offset > 0)
			{
				scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column, .line = scanner->line}, "illegal byte order mark");
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
scanner_peek(scanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		return scanner->source[scanner->reading_offset];
	}
	return 0;
}

static int32_t
scanner_skip_trivial(scanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if (scanner->ch == '\n' || scanner->ch == '\v' || scanner->ch == '\r')
		{
			int32_t ch = scanner->ch;
			if (scanner_next(scanner) == -1)
			{
				return -1;
			}
			if (ch == '\r' && scanner->ch == '\n')
			{
				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
			}
			scanner->column = 1;
			scanner->line++;
			continue;
		}

		if (scanner->ch == '\t' || isspace(scanner->ch))
		{
			if (scanner_next(scanner) == -1)
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
scanner_advance(scanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if(scanner_skip_trivial(scanner) == -1)
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
				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				int32_t n = 0;
				while (true)
				{
					char ch = scanner->ch;
					if (ch == '\n' || ch < 0)
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "rune literal not terminated");
						return -1;
					}
					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					if (ch == '\'')
					{
						if (n == 0)
						{
							scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "empty rune literal or unescaped '");
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
							if (scanner_next(scanner) == -1)
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
							if (scanner_next(scanner) == -1)
							{
								return -1;
							}
							m = 2;
							base = 16;
							max = 255;
							break;

						case 'u':
							if (scanner_next(scanner) == -1)
							{
								return -1;
							}
							m = 4;
							base = 16;
							max = 1114111; // unicode max rune
							break;

						case 'U':
							if (scanner_next(scanner) == -1)
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
								scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
								return -1;
							}
							scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "unknown escape sequence");
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
									scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
									return -1;
								}
								scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "illegal character %c in escape sequence", scanner->ch);
								return -1;
							}
							x = x * base + d;
							if (scanner_next(scanner) == -1)
							{
								return -1;
							}
							m--;
						}
						if ((x > (uint32_t)max) || ((0xD800 <= x) && (x < 0xE000)))
						{
							scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence is invalid Unicode code point");
							return -1;
						}
					}
				}
				if (n != 1)
				{
					scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "more than one character in rune literal");
					return -1;
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					fprintf(stderr, "unable to allocted a block of %zu bytes",
									sizeof(char) * (scanner->offset - start_offset));
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_CHAR,
																			 .value = data,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = start_offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				return 1;
			}
			else if (scanner->ch == '"' || scanner->ch == '`')
			{
				uint64_t start_offset = scanner->offset + 1;
				char c = scanner->ch;
				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				while (scanner->ch != c)
				{
					if ((scanner->ch == '\n' || scanner->ch == '\r') && c != '`')
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset - 1, .column = scanner->column - (scanner->offset - start_offset) - 1, .line = scanner->line}, "newline in string");
						return -1;
					}
					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					fprintf(stderr, "unable to allocted a block of %zu bytes",
									sizeof(char) * (scanner->offset - start_offset));
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_STRING,
																			 .value = data,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = start_offset - 1,
																					 .column = scanner->column - (scanner->offset - start_offset) - 1,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '#')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_HASH,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '$')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_DOLLER,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '(')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_LPAREN,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ')')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_RPAREN,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '[')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_LBRACKET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ']')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_RBRACKET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '{')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_LBRACE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '}')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_RBRACE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ',')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_COMMA,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '.')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_DOT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
		{
			return -1;
		}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '.')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_DOT_DOT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
				{
					return -1;
				}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == ':')
			{
				uint64_t offset = scanner->offset;
				uint64_t column = scanner->column;
				uint64_t line = scanner->line;

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_COLON,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_COLON_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == ';')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_SEMICOLON,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '?')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_QUESTION,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '@')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_AT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '\\')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_BACKSLASH,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '_')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_UNDERLINE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_PLUS,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_PLUS_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_MINUS,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_MINUS_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '>')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_MINUS_GT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_STAR,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_STAR_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '*')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_POWER,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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
				if ((c = scanner_peek(scanner)) && c == '/')
				{
					while (scanner->ch != '\n')
					{
						if (scanner_next(scanner) == -1)
				{
					return -1;
				}
					}
					continue;
				}
				else if ((c = scanner_peek(scanner)) && c == '*')
				{
					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					int64_t depth = 0;
					while (true)
					{
						if (scanner->ch == '*')
						{
							if ((c = scanner_peek(scanner)) && c == '/' && depth < 1)
							{
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
								break;
							}
							depth--;
						}
						if (scanner->ch == '\n')
						{
							if (scanner_next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '\r')
						{
							if (scanner_next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '/')
						{
							if ((c = scanner_peek(scanner)) && c == '*')
							{
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
								depth++;
								continue;
							}
						}
						if (scanner_next(scanner) == -1)
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

					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_SLASH,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = scanner->offset,
																						 .column = scanner->column,
																						 .line = scanner->line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}

					if(scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						scanner_set_token(scanner, (token_t){
																					 .type = TOKEN_SLASH_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->file_source->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line}});

						if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_PERCENT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_PERCENT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_AND,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '&')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_AND_AND,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_AND_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_OR,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '|')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_OR_OR,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_OR_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}
				return 1;
			}
			else if (scanner->ch == '^')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_CARET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '~')
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_TILDE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_LT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '<')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_LT_LT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}

					if(scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						scanner_set_token(scanner, (token_t){
																					 .type = TOKEN_LT_LT_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->file_source->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line}});

						if (scanner_next(scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_LT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_GT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '>')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_GT_GT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}

					if(scanner_skip_trivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						scanner_set_token(scanner, (token_t){
																					 .type = TOKEN_GT_GT_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->file_source->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line}});

						if (scanner_next(scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_GT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_NOT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_NOT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
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

				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_EQ,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line}});

				if (scanner_next(scanner) == -1)
				{
					return -1;
				}

				if(scanner_skip_trivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					scanner_set_token(scanner, (token_t){
																				 .type = TOKEN_EQ_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->file_source->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line}});

					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
				}
				return 1;
			}
			else
			{
				scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column, .line = scanner->line}, "unknown token");
				return -1;
			}
		}
		else 
		if (isdigit(scanner->ch))
		{
			uint64_t start_offset = scanner->offset;

			if (scanner->ch == '0')
			{
				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
				switch (tolower(scanner->ch))
				{
				case 'x':
					if (scanner_next(scanner) == -1)
				{
					return -1;
				}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isxdigit(scanner->ch))
							{
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong hexadecimal number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "hexadecimal literal has no digits");
						return -1;
					}
					break;

				case 'b':
					if (scanner_next(scanner) == -1)
				{
					return -1;
				}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isbinary(scanner->ch))
							{
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong binary number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "binary literal has no digits");
						return -1;
					}
					break;

				case 'o':
					if (scanner_next(scanner) == -1)
				{
					return -1;
				}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isoctal(scanner->ch))
							{
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong octal number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "octal literal has no digits");
						return -1;
					}
					break;

				case '.':
					if (scanner_next(scanner) == -1)
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
										scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong decimal number format");
										return -1;
									}
									is_float++;
									if (scanner_next(scanner) == -1)
									{
										return -1;
									}
									continue;
								}
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "decimal literal has no digits");
						return -1;
					}
					break;

				default:
					if (!isoctal(scanner->ch))
					{
						break;
					}
					if (scanner_next(scanner) == -1)
					{
						return -1;
					}
					if (!isspace(scanner->ch))
					{
						while (!isspace(scanner->ch))
						{
							if (isoctal(scanner->ch))
							{
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong octal number format");
									return -1;
								}
								break;
							}
						}
					}
					else
					{
						scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "octal literal has no digits");
						return -1;
					}
					break;
				}
			}
			else
			{
				if (scanner_next(scanner) == -1)
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
								scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong exponent format");
								return -1;
							}
							is_exp++;
							if (scanner_next(scanner) == -1)
							{
								return -1;
							}
							if (scanner->ch == '+' || scanner->ch == '-')
							{
								if (sign > 0)
								{
									break;
								}
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
								sign++;
							}
							if (!isdigit(scanner->ch))
							{
								scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "exponent has no digits");
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
							ch = scanner_peek(scanner);
							if (isdigit(ch))
							{
								is_float++;
								if (scanner_next(scanner) == -1)
								{
									return -1;
								}
								continue;
							}
							break;
						}
						if (scanner_next(scanner) == -1)
						{
							return -1;
						}
					}
					else
					{
						if (isalpha(scanner->ch))
						{
							scanner_error(scanner, (position_t){.path = scanner->file_source->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "wrong number format");
							return -1;
						}
						break;
					}
				}
			}

			char *data;
			if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
			{
				fprintf(stderr, "unable to allocted a block of %zu bytes", (scanner->offset - start_offset));
				return -1;
			}
			strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
			data[scanner->offset - start_offset] = '\0';

			scanner_set_token(scanner, (token_t){
																		 .type = TOKEN_NUMBER,
																		 .value = data,
																		 .position = {
																				 .path = scanner->file_source->path,
																				 .offset = scanner->offset,
																				 .column = scanner->column - (scanner->offset - start_offset),
																				 .line = scanner->line}});
			return 1;
		}
		else
		{
			// parse identifier
			uint64_t start_offset = scanner->offset;
			while (isalpha(scanner->ch) || isdigit(scanner->ch) || (scanner->ch == '_'))
			{
				if (scanner_next(scanner) == -1)
				{
					return -1;
				}
			}
			uint32_t length = scanner->offset - start_offset;

			if (strncmp(scanner->source + start_offset, "for", max(length, 3)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_FOR_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "in", max(length, 2)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_IN_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "oo", max(length, 2)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_INFINITY_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "class", max(length, 5)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_CLASS_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "extends", max(length, 7)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_EXTENDS_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "static", max(length, 6)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_STATIC_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "readonly", max(length, 8)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_READONLY_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "reference", max(length, 9)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_REFERENCE_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "protected", max(length, 9)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_PROTECTED_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "if", max(length, 2)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_IF_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "else", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_ELSE_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
							}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "try", max(length, 3)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_TRY_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "catch", max(length, 5)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_CATCH_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "throw", max(length, 5)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_THROW_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "break", max(length, 5)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_BREAK_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "continue", max(length, 8)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_CONTINUE_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "true", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_TRUE_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "false", max(length, 5)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_FALSE_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "this", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_THIS_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "async", max(length, 5)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_ASYNC_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "return", max(length, 6)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_RETURN_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "sizeof", max(length, 6)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_SIZEOF_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "typeof", max(length, 6)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_TYPEOF_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "null", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_NULL_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "func", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_FUNC_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "var", max(length, 3)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_VAR_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "type", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
																			 .type = TOKEN_TYPE_KEYWORD,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->file_source->path,
																					 .offset = scanner->offset - length,
																					 .column = scanner->column - (scanner->offset - start_offset),
																					 .line = scanner->line}});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "enum", max(length, 4)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_ENUM_KEYWORD,
					.value = NULL,
					.position = {
						.path = scanner->file_source->path,
						.offset = scanner->offset - length,
						.column = scanner->column - (scanner->offset - start_offset),
						.line = scanner->line
					}
				});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "export", max(length, 6)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_EXPORT_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
						}
					});

				return 1;
			}
			else 
			if (strncmp(scanner->source + start_offset, "import", max(length, 6)) == 0)
			{
				scanner_set_token(scanner, (token_t){
					.type = TOKEN_IMPORT_KEYWORD,
					.value = NULL,
					.position = {
							.path = scanner->file_source->path,
							.offset = scanner->offset - length,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line
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

			scanner_set_token(scanner, (token_t){
				.type = TOKEN_ID,
				.value = data,
				.position = {
						.path = scanner->file_source->path,
						.offset = scanner->offset,
						.column = scanner->column - (scanner->offset - start_offset),
						.line = scanner->line
					}
				});

			return 1;
		}

		if (scanner_next(scanner) == -1)
		{
			return -1;
		}
	}

	scanner_set_token(scanner, (token_t){
		.type = TOKEN_EOF,
		.value = NULL,
		.position = {
				.path = scanner->file_source->path,
				.offset = scanner->offset,
				.column = scanner->column,
				.line = scanner->line
			}
		});

	return 1;
}
