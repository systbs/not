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
#include "scanner.h"
#include "utf8.h"

#define max(a, b) a > b ? a : b
#define isoctal(c) (c >= '0' && c <= '7')
#define isbinary(c) (c == '0' || c == '1')
#define bom 0xFEFF
#define eof -1

int32_t
SyScanner_DigitValue(int32_t ch)
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
SyScanner_SetToken(SyScanner_t *scanner, SyToken_t token)
{
	//printf("%s-%lld:%lld  %s\n", token.position.path, token.position.line, token.position.column, sy_token_get_name(token.type));
	scanner->token = token;
}

SyScanner_t *
SyScanner_Create(char *path)
{
	SyScanner_t *scanner = (SyScanner_t *)sy_memory_calloc(1, sizeof(SyScanner_t));
	if (scanner == NULL)
	{
		sy_error_no_memory();
		return NULL;
	}

	strcpy(scanner->path, path);

	FILE *fd = fopen(scanner->path, "rb");
    if (fd == NULL)
	{
        sy_error_system("could not open(%s)\n", scanner->path);
        return NULL;
    }
    
    int64_t pos = ftell(fd);
    fseek(fd, 0, SEEK_END);
    int64_t chunk = ftell(fd);
    fseek(fd, pos, SEEK_SET);

    char *buf = sy_memory_calloc(1, chunk + 1);
    if (buf == NULL)
	{
    	sy_error_no_memory();
        return NULL;
    }

	int64_t i;
    if ((i = fread(buf, 1, chunk, fd)) < chunk)
	{
        sy_error_system("read returned %ld\n", i);
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

	SyScanner_Advance(scanner);

	return scanner;
}

void 
SyScanner_Destroy(SyScanner_t *scanner)
{
	free(scanner);
}

static int32_t
SyScanner_Next(SyScanner_t *scanner)
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
			sy_error_lexer_by_position((sy_position_t){
				.path = scanner->path, 
				.offset = scanner->offset, 
				.column = scanner->column, 
				.line = scanner->line,
				.length = 1}, "illegal character NUL");
			return -1;
		}
		if (r >= 0x80)
		{
			Sy_Utf8Decode(scanner->source + scanner->reading_offset, &r, &w);
			if (r == utf8_error && w == 1)
			{
				sy_error_lexer_by_position((sy_position_t){
					.path = scanner->path, 
					.offset = scanner->offset, 
					.column = scanner->column, 
					.line = scanner->line,
					.length = 1}, "illegal UTF-8 encoding");
				return -1;
			}
			else if (r == bom && scanner->offset > 0)
			{
				sy_error_lexer_by_position((sy_position_t){
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
SyScanner_Peek(SyScanner_t *scanner)
{
	if (scanner->reading_offset < strlen(scanner->source))
	{
		return scanner->source[scanner->reading_offset];
	}
	return 0;
}

static int32_t
SyScanner_SkipTrivial(SyScanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if (scanner->ch == '\n' || scanner->ch == '\v' || scanner->ch == '\r')
		{
			scanner->column = 0;
			scanner->line++;

			int32_t ch = scanner->ch;
			if (SyScanner_Next(scanner) == -1)
			{
				return -1;
			}
			if (ch == '\r' && scanner->ch == '\n')
			{
				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
			}
			continue;
		}

		if (scanner->ch == '\t' || isspace(scanner->ch))
		{
			if (SyScanner_Next(scanner) == -1)
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
SyScanner_Advance(SyScanner_t *scanner)
{
	while (scanner->ch != eof)
	{
		if(SyScanner_SkipTrivial(scanner) == -1)
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
				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				int32_t n = 0;
				while (true)
				{
					char ch = scanner->ch;
					if (ch == '\n' || ch < 0)
					{
						sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "rune literal not terminated");
						return -1;
					}
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					if (ch == '\'')
					{
						if (n == 0)
						{
							sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "empty rune literal or unescaped '");
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
							if (SyScanner_Next(scanner) == -1)
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
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
							m = 2;
							base = 16;
							max = 255;
							break;

						case 'u':
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
							m = 4;
							base = 16;
							max = 1114111; // unicode max rune
							break;

						case 'U':
							if (SyScanner_Next(scanner) == -1)
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
								sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
								return -1;
							}
							sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "unknown escape sequence");
							return -1;
						}

						uint32_t x = 0;
						while (m > 0)
						{
							int32_t d = (int32_t)(SyScanner_DigitValue(scanner->ch));
							if (d >= base)
							{
								if (scanner->ch < 0)
								{
									sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence not terminated");
									return -1;
								}
								sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "illegal character %c in escape sequence", scanner->ch);
								return -1;
							}
							x = x * base + d;
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
							m--;
						}
						if ((x > (uint32_t)max) || ((0xD800 <= x) && (x < 0xE000)))
						{
							sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "escape sequence is invalid Unicode code point");
							return -1;
						}
					}
				}
				if (n != 1)
				{
					sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset, .column = scanner->column - (scanner->offset - start_offset), .line = scanner->line}, "more than one character in rune literal");
					return -1;
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					sy_error_no_memory();
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				SyScanner_SetToken(scanner, (SyToken_t){
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
				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				while (scanner->ch != c)
				{
					if ((scanner->ch == '\n' || scanner->ch == '\r') && c != '"')
					{
						sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset - 1, .column = scanner->column - (scanner->offset - start_offset) - 1, .line = scanner->line}, "newline in string");
						return -1;
					}
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					sy_error_no_memory();
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_STRING,
																			 .value = data,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = start_offset - 1,
																					 .column = scanner->column - (scanner->offset - start_offset) - 1,
																					 .line = scanner->line,
																					 .length = scanner->offset - start_offset}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '`')
			{
				uint64_t start_offset = scanner->offset + 1;
				char c = scanner->ch;
				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				while (scanner->ch != c)
				{
					if ((scanner->ch == '\n' || scanner->ch == '\r') && c != '`')
					{
						sy_error_lexer_by_position((sy_position_t){.path = scanner->path, .offset = scanner->offset - 1, .column = scanner->column - (scanner->offset - start_offset) - 1, .line = scanner->line}, "newline in string");
						return -1;
					}
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
				}

				char *data;
				if (!(data = malloc(sizeof(char) * (scanner->offset - start_offset))))
				{
					sy_error_no_memory();
					return -1;
				}
				strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
				data[scanner->offset - start_offset] = '\0';

				SyScanner_SetToken(scanner, (SyToken_t){
						.type = TOKEN_STRING,
						.value = data,
						.position = {
								.path = scanner->path,
								.offset = start_offset - 1,
								.column = scanner->column - (scanner->offset - start_offset) - 1,
								.line = scanner->line,
								.length = scanner->offset - start_offset
								}
						});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '#')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_HASH,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '$')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_DOLLER,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '(')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_LPAREN,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ')')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_RPAREN,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '[')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_LBRACKET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ']')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_RBRACKET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '{')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_LBRACE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '}')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_RBRACE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == ',')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_COMMA,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '.')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_DOT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
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
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_COLON,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
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
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_SEMICOLON,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '?')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_QUESTION,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '@')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_AT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_BACKSLASH,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_BACKSLASH_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				return 1;
			}
			else if (scanner->ch == '_')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_UNDERLINE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_PLUS,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_PLUS_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_MINUS,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_MINUS_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '>')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_MINUS_GT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_STAR,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_STAR_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '*')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_POWER,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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
				if ((c = SyScanner_Peek(scanner)) && c == '/')
				{
					while (scanner->ch != '\n')
					{
						if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
					}
					continue;
				}
				else if ((c = SyScanner_Peek(scanner)) && c == '*')
				{
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					int64_t depth = 0;
					while (true)
					{
						if (scanner->ch == '*')
						{
							if ((c = SyScanner_Peek(scanner)) && c == '/' && depth < 1)
							{
								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
								break;
							}
							depth--;
						}
						if (scanner->ch == '\n')
						{
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '\r')
						{
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
							continue;
						}
						if (scanner->ch == '/')
						{
							if ((c = SyScanner_Peek(scanner)) && c == '*')
							{
								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
								depth++;
								continue;
							}
						}
						if (SyScanner_Next(scanner) == -1)
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

					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_SLASH,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = scanner->offset,
																						 .column = scanner->column,
																						 .line = scanner->line,
																					 .length = 1}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if(SyScanner_SkipTrivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						SyScanner_SetToken(scanner, (SyToken_t){
																					 .type = TOKEN_SLASH_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line,
																						 	.length = 2}});

						if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_PERCENT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_PERCENT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_AND,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '&')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_AND_AND,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_AND_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_OR,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '|')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_OR_OR,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_OR_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
					return 1;
				}
				return 1;
			}
			else if (scanner->ch == '^')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_CARET,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
				return 1;
			}
			else if (scanner->ch == '~')
			{
				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_TILDE,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_LT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '<')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_LT_LT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if(SyScanner_SkipTrivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						SyScanner_SetToken(scanner, (SyToken_t){
																					 .type = TOKEN_LT_LT_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line,
																						 .length = 3}});

						if (SyScanner_Next(scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_LT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_GT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '>')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_GT_GT,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if(SyScanner_SkipTrivial(scanner) == -1)
					{
						return -1;
					}
					if (scanner->ch == eof)
					{
						break;
					}

					if (scanner->ch == '=')
					{
						SyScanner_SetToken(scanner, (SyToken_t){
																					 .type = TOKEN_GT_GT_EQ,
																					 .value = NULL,
																					 .position = {
																							 .path = scanner->path,
																							 .offset = offset,
																							 .column = column,
																							 .line = line,
																						 .length = 2}});

						if (SyScanner_Next(scanner) == -1)
						{
							return -1;
						}
					}

					return 1;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_GT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_NOT,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_NOT_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
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

				SyScanner_SetToken(scanner, (SyToken_t){
																			 .type = TOKEN_EQ,
																			 .value = NULL,
																			 .position = {
																					 .path = scanner->path,
																					 .offset = scanner->offset,
																					 .column = scanner->column,
																					 .line = scanner->line,
																					 .length = 1}});

				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if(SyScanner_SkipTrivial(scanner) == -1)
				{
					return -1;
				}
				if (scanner->ch == eof)
				{
					break;
				}

				if (scanner->ch == '=')
				{
					SyScanner_SetToken(scanner, (SyToken_t){
																				 .type = TOKEN_EQ_EQ,
																				 .value = NULL,
																				 .position = {
																						 .path = scanner->path,
																						 .offset = offset,
																						 .column = column,
																						 .line = line,
																						 .length = 2}});

					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}
				}
				return 1;
			}
			else
			{
				sy_error_lexer_by_position((sy_position_t){
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
				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}

				if (tolower(scanner->ch) == 'x')
				{
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if (!isspace(scanner->ch))
					{
						__uint128_t number = 0;

						while (!isspace(scanner->ch))
						{
							if (isxdigit(scanner->ch))
							{
								char c = scanner->ch;
								uint8_t value = ((c & 0xF) + (c >> 6)) | ((c >> 3) & 0x8);
								number = (number << 4) | (uint64_t) value;

								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									sy_error_lexer_by_position((sy_position_t){
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
						sy_error_lexer_by_position((sy_position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "hexadecimal literal has no digits");
						return -1;
					}
				}
				else
				if (tolower(scanner->ch) == 'b')
				{
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if (!isspace(scanner->ch))
					{
						__uint128_t number = 0;

						while (!isspace(scanner->ch))
						{
							if (isbinary(scanner->ch))
							{
								char c = scanner->ch;

								uint8_t value = ((c & 0x1) - '0');
								number = (number << 1) | (uint64_t) value;

								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									sy_error_lexer_by_position((sy_position_t){
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
						sy_error_lexer_by_position((sy_position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "binary literal has no digits");
						return -1;
					}
				}
				else
				if (tolower(scanner->ch) == 'o')
				{
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if (!isspace(scanner->ch))
					{
						__uint128_t number = 0;

						while (!isspace(scanner->ch))
						{
							if (isoctal(scanner->ch))
							{
								char c = scanner->ch;

								uint8_t value = (c - '0');
								number = (number << 3) | (uint64_t) value;

								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									sy_error_lexer_by_position((sy_position_t){
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
						sy_error_lexer_by_position((sy_position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "octal literal has no digits");
						return -1;
					}
				}
				else
				if (tolower(scanner->ch) == '.')
				{
					if (SyScanner_Next(scanner) == -1)
					{
						return -1;
					}

					if (!isspace(scanner->ch))
					{
						if (isdigit(scanner->ch))
						{
							double128_t number = 0, fact = 1;
							int32_t sign = 1;

							while (!isspace(scanner->ch))
							{
								if (tolower(scanner->ch) == 'e')
								{
									if (SyScanner_Next(scanner) == -1)
									{
										return -1;
									}

									if (tolower(scanner->ch) == '+')
									{
										sign = 1;
										if (SyScanner_Next(scanner) == -1)
										{
											return -1;
										}
									}
									else
									if (tolower(scanner->ch) == '-')
									{
										sign = -1;
										if (SyScanner_Next(scanner) == -1)
										{
											return -1;
										}
									}
									else
									{
										sy_error_lexer_by_position((sy_position_t){
											.path = scanner->path, 
											.offset = scanner->offset, 
											.column = scanner->column - (scanner->offset - start_offset), 
											.line = scanner->line,
											.length = scanner->offset - start_offset}, "wrong number format");
										return -1;
									}

									uint64_t sym = 0;
									while (!isspace(scanner->ch))
									{
										if (isdigit(scanner->ch))
										{
											char c = scanner->ch;
											uint8_t value = (c - '0');
											sym = (sym * 10) + (uint64_t) value;

											if (SyScanner_Next(scanner) == -1)
											{
												return -1;
											}
										}
										else
										{
											break;
										}
									}

									for (uint64_t k = 0;k < sym;k++)
									{
										if (sign == 1)
										{
											number *= 10;
										}
										else
										{
											number /= 10;
										}
									}
									break;
								}
								else
								if (scanner->ch == '.')
								{
									break;
								}
								if (isdigit(scanner->ch))
								{
									char c = scanner->ch;

									uint8_t value = (c - '0');
									fact /= 10;
									number = (number *10 ) + (uint64_t) value;

									if (SyScanner_Next(scanner) == -1)
									{
										return -1;
									}
									continue;
								}
								else
								{
									if (isalpha(scanner->ch))
									{
										sy_error_lexer_by_position((sy_position_t){
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

							number = number * fact;
						}
						else
						{
							break;
						}
					}
					else
					{
						sy_error_lexer_by_position((sy_position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "decimal literal has no digits");
						return -1;
					}
				}
				else
				{
					if (!isspace(scanner->ch))
					{
						__uint128_t number = 0;
						while (!isspace(scanner->ch))
						{
							if (isoctal(scanner->ch))
							{
								char c = scanner->ch;

								uint8_t value = (c - '0');
								number = (number << 3) | (uint64_t) value;

								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								if (isalpha(scanner->ch))
								{
									sy_error_lexer_by_position((sy_position_t){
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
						sy_error_lexer_by_position((sy_position_t){
							.path = scanner->path, 
							.offset = scanner->offset, 
							.column = scanner->column - (scanner->offset - start_offset), 
							.line = scanner->line,
							.length = scanner->offset - start_offset}, "octal literal has no digits");
						return -1;
					}
				}
			}
			else
			{
				double128_t number = 0, fact = 1;

				int is_float = 0;
				int sign = 1;
				while (!isspace(scanner->ch))
				{
					if (tolower(scanner->ch) == 'e')
					{
						if (SyScanner_Next(scanner) == -1)
						{
							return -1;
						}

						if (tolower(scanner->ch) == '+')
						{
							sign = 1;
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						if (tolower(scanner->ch) == '-')
						{
							sign = -1;
							if (SyScanner_Next(scanner) == -1)
							{
								return -1;
							}
						}
						else
						{
							sy_error_lexer_by_position((sy_position_t){
								.path = scanner->path, 
								.offset = scanner->offset, 
								.column = scanner->column - (scanner->offset - start_offset), 
								.line = scanner->line,
								.length = scanner->offset - start_offset}, "wrong number format");
							return -1;
						}

						uint64_t sym = 0;
						while (!isspace(scanner->ch))
						{
							if (isdigit(scanner->ch))
							{
								char c = scanner->ch;
								uint8_t value = (c - '0');
								sym = (sym * 10) + (uint64_t) value;

								if (SyScanner_Next(scanner) == -1)
								{
									return -1;
								}
							}
							else
							{
								break;
							}
						}

						for (uint64_t k = 0;k < sym;k++)
						{
							if (sign == 1)
							{
								number *= 10;
							}
							else
							{
								number /= 10;
							}
						}
						break;
					}
					else
					if (scanner->ch == '.')
					{
						if (is_float > 0)
						{
							break;
						}

						is_float += 1;
						if (SyScanner_Next(scanner) == -1)
						{
							return -1;
						}
						continue;;
					}
					else
					if (isdigit(scanner->ch))
					{
						char c = scanner->ch;

						uint8_t value = (c - '0');
						if (is_float == 1)
						{
							fact /= 10;
						}
						number = (number * 10) + (uint64_t) value;

						if (SyScanner_Next(scanner) == -1)
						{
							return -1;
						}
						continue;
					}
					else
					{
						if (isalpha(scanner->ch))
						{
							sy_error_lexer_by_position((sy_position_t){
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

				number = number * fact;
			}

			char *data = malloc(sizeof(char) * (scanner->offset - start_offset));
			if (data == NULL)
			{
				sy_error_no_memory();
				return -1;
			}
			strncpy(data, scanner->source + start_offset, scanner->offset - start_offset);
			data[scanner->offset - start_offset] = '\0';

			SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_NUMBER,
					.value = data,
					.position = {
							.path = scanner->path,
							.offset = scanner->offset,
							.column = scanner->column - (scanner->offset - start_offset),
							.line = scanner->line,
							.length = scanner->offset - start_offset
						}
					});
			return 1;
		}
		else
		{
			// parse identifier
			uint64_t start_offset = scanner->offset;
			while (isalpha(scanner->ch) || isdigit(scanner->ch) || (scanner->ch == '_'))
			{
				if (SyScanner_Next(scanner) == -1)
				{
					return -1;
				}
			}
			uint32_t length = scanner->offset - start_offset;

			if (strncmp(scanner->source + start_offset, "null", max(length, 4)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
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
			if (strncmp(scanner->source + start_offset, "int8", max(length, 4)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_INT8_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "int16", max(length, 5)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_INT16_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "int32", max(length, 5)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_INT32_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "int64", max(length, 5)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_INT64_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "uint8", max(length, 5)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_UINT8_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "uint16", max(length, 6)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_UINT16_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "uint32", max(length, 6)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_UINT32_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "uint64", max(length, 6)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_UINT64_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "bigint", max(length, 6)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_BIGINT_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "float32", max(length, 7)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_FLOAT32_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "float64", max(length, 7)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_FLOAT64_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "bigfloat", max(length, 8)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_BIGFLOAT_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "char", max(length, 4)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_CHAR_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "string", max(length, 6)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_STRING_KEYWORD,
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
			if (strncmp(scanner->source + start_offset, "for", max(length, 3)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
			if (strncmp(scanner->source + start_offset, "self", max(length, 4)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
					.type = TOKEN_SELF_KEYWORD,
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
			if (strncmp(scanner->source + start_offset, "fun", max(length, 3)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
			if (strncmp(scanner->source + start_offset, "export", max(length, 6)) == 0)
			{
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				SyScanner_SetToken(scanner, (SyToken_t){
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
				sy_error_no_memory();
				return -1;
			}
			strncpy(data, scanner->source + start_offset, length);
			data[length] = '\0';

			SyScanner_SetToken(scanner, (SyToken_t){
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

		if (SyScanner_Next(scanner) == -1)
		{
			return -1;
		}
	}

	SyScanner_SetToken(scanner, (SyToken_t){
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
SyScanner_Gt(SyScanner_t *scanner)
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
		if (SyScanner_Next(scanner) == -1)
		{
			return -1;
		}

		if(SyScanner_SkipTrivial(scanner) == -1)
		{
			return -1;
		}
	}

	return 1;
}