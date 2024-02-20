#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include "../utils/dirent.h"
#elif defined(__arm__) || defined(__aarch64__)
#include "../utils/arm/dirent.h"
#else
#include <dirent.h>
#endif

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "parser.h"
#include "error.h"
#include "symbol.h"
#include "graph.h"
#include "syntax.h"
#include "response.h"


static error_t *
syntax_error(program_t *program, symbol_t *current, const char *format, ...)
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

	node_t *node;
	node = current->declaration;

	error_t *error;
	error = error_create(node->position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(program->errors, (uint64_t)error))
	{
		return NULL;
	}

	return error;
}


static int32_t
syntax_function(program_t *program, symbol_t *current);

static int32_t
syntax_block(program_t *program, symbol_t *current);

static int32_t
syntax_assign(program_t *program, symbol_t *current);

static int32_t
syntax_import(program_t *program, symbol_t *current);

static int32_t
syntax_generics(program_t *program, symbol_t *current);

static int32_t
syntax_generic(program_t *program, symbol_t *current);

static int32_t
syntax_fields(program_t *program, symbol_t *current);

static int32_t
syntax_field(program_t *program, symbol_t *current);




static symbol_t *
syntax_only(symbol_t *s2)
{
	symbol_t *a;
	for (a = s2->begin;a != s2->end;a = a->next)
	{
		return a;
	}
	return NULL;
}

static symbol_t *
syntax_only_with(symbol_t *s2, uint64_t flag)
{
	symbol_t *a;
	for (a = s2->begin;a != s2->end;a = a->next)
	{
		if (symbol_check_type(a, flag))
		{
			return a;
		}
	}
	return NULL;
}

static symbol_t *
syntax_extract_with(symbol_t *s2, uint64_t flag)
{
	symbol_t *a;
	a = syntax_only_with(s2, flag);
	if (a)
	{
		return syntax_only(a);
	}
	return NULL;
}



static int32_t
syntax_strcmp(symbol_t *id1, symbol_t *id2)
{
	node_t *nid1 = id1->declaration;
	node_basic_t *nbid1 = (node_basic_t *)nid1->value;

	node_t *nid2 = id2->declaration;
	node_basic_t *nbid2 = (node_basic_t *)nid2->value;

	//printf("%s %s\n", nbid1->value, nbid2->value);

	return strcmp(nbid1->value, nbid2->value);
}




static int32_t
syntax_expression(program_t *program, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_assign(program_t *program, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_return(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_continue(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_break(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_throw(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_var(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							symbol_t *ags;
							ags = syntax_only_with(a, SYMBOL_GENERICS);
							if (ags)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = ags->begin;(b != ags->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_extract_with(b, SYMBOL_VALUE);
										if (!agv)
										{
											no_value = 1;
											break;
										}
									}
								}
								if (no_value == -1)
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										ak->declaration->position.line, ak->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
									ak->declaration->position.line, ak->declaration->position.column);
								return -1;
							}
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FUNCTION))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							symbol_t *ags;
							ags = syntax_only_with(a, SYMBOL_GENERICS);
							if (ags)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = ags->begin;(b != ags->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_extract_with(b, SYMBOL_VALUE);
										if (!agv)
										{
											no_value = 1;
											break;
										}
									}
								}
								if (no_value == -1)
								{
									symbol_t *aps;
									aps = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (aps)
									{
										int32_t no_value2 = -1;
										symbol_t *b;
										for (b = aps->begin;(b != aps->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_PARAMETER))
											{
												symbol_t *cpv;
												cpv = syntax_only_with(b, SYMBOL_VALUE);
												if (!cpv)
												{
													no_value2 = 1;
													break;
												}
											}
										}
										if (no_value2 == -1)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
										else
										{
											continue;
										}
									}
									else
									{
										syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
											ak->declaration->position.line, ak->declaration->position.column);
										return -1;
									}
								}
								else
								{
									continue;
								}
							}
							else
							{
								symbol_t *aps;
								aps = syntax_only_with(a, SYMBOL_PARAMETERS);
								if (aps)
								{
									int32_t no_value2 = -1;
									symbol_t *b;
									for (b = aps->begin;(b != aps->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_PARAMETER))
										{
											symbol_t *cpv;
											cpv = syntax_only_with(b, SYMBOL_VALUE);
											if (!cpv)
											{
												no_value2 = 1;
												break;
											}
										}
									}
									if (no_value2 == -1)
									{
										syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
											ak->declaration->position.line, ak->declaration->position.column);
										return -1;
									}
									else
									{
										continue;
									}
								}
								else
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										ak->declaration->position.line, ak->declaration->position.column);
									return -1;
								}
							}
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "function without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "var without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "enum without key");
						return -1;
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "variable without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "variable without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_if(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "var without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_IF))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FOR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FORIN))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "if without parent");
			return -1;
		}
	}


	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ELSE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_IF))
				{
					int32_t result;
					result = syntax_if(program, b);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = syntax_block(program, b);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_catch(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_try(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CATCHS))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_CATCH))
				{
					int32_t result;
					result = syntax_catch(program, b);
					if (result == -1)
					{
						return -1;
					}
					continue;
				}
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_for(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "var without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_IF))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FOR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FORIN))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "if without parent");
			return -1;
		}
	}

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					int32_t result;
					result = syntax_var(program, b);
					if (result == -1)
					{
						return -1;
					}
					continue;
				}
				if (symbol_check_type(a, SYMBOL_ASSIGN))
				{
					int32_t result;
					result = syntax_assign(program, b);
					if (result == -1)
					{
						return -1;
					}
					continue;
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			int32_t result;
			result = syntax_expression(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_INCREMENTOR))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_ASSIGN))
				{
					int32_t result;
					result = syntax_assign(program, b);
					if (result == -1)
					{
						return -1;
					}
					continue;
				}
			}
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_forin(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "var without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_IF))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FOR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FORIN))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "for without parent");
			return -1;
		}
	}

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					int32_t result;
					result = syntax_var(program, b);
					if (result == -1)
					{
						return -1;
					}
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_EXPRESSION))
		{
			int32_t result;
			result = syntax_expression(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_statement(program_t *program, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_type(current, SYMBOL_BLOCK))
	{
		result = syntax_block(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_IF))
	{
		result = syntax_if(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_TRY))
	{
		result = syntax_try(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_FOR))
	{
		result = syntax_for(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_FORIN))
	{
		result = syntax_forin(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_FUNCTION))
	{
		result = syntax_function(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_CONTINUE))
	{
		result = syntax_continue(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_BREAK))
	{
		result = syntax_break(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_RETURN))
	{
		result = syntax_return(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_THROW))
	{
		result = syntax_throw(program, current);
	}
	else 
	if (symbol_check_type(current, SYMBOL_VAR))
	{
		result = syntax_var(program, current);
	}
	else
	{
		result = syntax_assign(program, current);
	}

	return result;
}

static int32_t
syntax_block(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end);a = a->next)
	{
		int32_t result;
		result = syntax_statement(program, a);
		if (result == -1)
		{
			return -1;
		}
	}
	return 1;
}



static int32_t
syntax_generic(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_GENERIC))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "generic without key");
						return -1;
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "generic without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "generic without key");
		return -1;
	}
	return 1;
}

static int32_t
syntax_generics(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_generic(program, a);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_parameter(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_PARAMETER))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "parameter without key");
						return -1;
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "parameter without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "parameter without key");
		return -1;
	}
	return 1;
}

static int32_t
syntax_parameters(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_parameter(program, a);
		if (result == -1)
		{
			return -1;
		}

		symbol_t *ak;
		ak = syntax_extract_with(a, SYMBOL_KEY);
		if (ak)
		{
			symbol_t *root = current->parent;
			symbol_t *gs;
			gs = syntax_only_with(root, SYMBOL_GENERICS);
			if (gs)
			{
				symbol_t *b;
				for (b = gs->begin;(b != gs->end); b = b->next)
				{
					if (symbol_check_type(b, SYMBOL_GENERIC))
					{
						symbol_t *bk;
						bk = syntax_extract_with(b, SYMBOL_KEY);
						if (bk)
						{
							if (syntax_strcmp(ak, bk) == 0)
							{
								syntax_error(program, ak, "defination repeated, another defination in %lld:%lld",
									bk->declaration->position.line, bk->declaration->position.column);
								return -1;
							}
							else
							{
								continue;
							}
						}
						else
						{
							syntax_error(program, b, "generic without key");
							return -1;
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, a, "parameter without key");
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_heritage(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_HERITAGE))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "heritage without key");
						return -1;
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "heritage without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "heritage without key");
		return -1;
	}
	return 1;
}

static int32_t
syntax_heritages(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_heritage(program, a);
		if (result == -1)
		{
			return -1;
		}

		symbol_t *ak;
		ak = syntax_extract_with(a, SYMBOL_KEY);
		if (ak)
		{
			symbol_t *root = current->parent;
			symbol_t *gs;
			gs = syntax_only_with(root, SYMBOL_GENERICS);
			if (gs)
			{
				symbol_t *b;
				for (b = gs->begin;(b != gs->end); b = b->next)
				{
					if (symbol_check_type(b, SYMBOL_GENERIC))
					{
						symbol_t *bk;
						bk = syntax_extract_with(b, SYMBOL_KEY);
						if (bk)
						{
							if (syntax_strcmp(ak, bk) == 0)
							{
								syntax_error(program, ak, "defination repeated, another defination in %lld:%lld",
									bk->declaration->position.line, bk->declaration->position.column);
								return -1;
							}
							else
							{
								continue;
							}
						}
						else
						{
							syntax_error(program, b, "generic without key");
							return -1;
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, a, "heritage without key");
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_member(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_MEMBER))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "enum member without key");
						return -1;
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "emum member without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "emum member without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_members(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_member(program, a);
		if (result == -1)
		{
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_enum(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FUNCTION))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "function without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "property without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "variable without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "enum without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_GENERICS))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_GENERIC))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "generic without key");
								return -1;
							}
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_HERITAGES))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_HERITAGE))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "heritage without key");
								return -1;
							}
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "enum without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "enum without key");
		return -1;
	}

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_MEMBERS))
		{
			int32_t result;
			result = syntax_members(program, a);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
syntax_property(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FUNCTION))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "function without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "property without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "enum without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_GENERICS))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_GENERIC))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "generic without key");
								return -1;
							}
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_HERITAGES))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_HERITAGE))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "heritage without key");
								return -1;
							}
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "property without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "property without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_function(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FUNCTION))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "function without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "property without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "variable without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "enum without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_GENERICS))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_GENERIC))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "generic without key");
								return -1;
							}
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_HERITAGES))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_HERITAGE))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "heritage without key");
								return -1;
							}
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "function without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "function without key");
		return -1;
	}

	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			int32_t result;
			result = syntax_generics(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_class(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FUNCTION))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_strcmp(ck, ak) == 0) && (current != a))
						{
							syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
								ak->declaration->position.line, ak->declaration->position.column);
							return -1;
						}
						else
						{
							continue;
						}
					}
					else
					{
						syntax_error(program, a, "class without key");
						return -1;
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_GENERICS))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_GENERIC))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "generic without key");
								return -1;
							}
						}
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_HERITAGES))
				{
					symbol_t *d;
					for (d = a->begin;(d != a->end);d = d->next)
					{
						if (symbol_check_type(d, SYMBOL_HERITAGE))
						{
							symbol_t *dk;
							dk = syntax_extract_with(d, SYMBOL_KEY);
							if (dk)
							{
								if ((syntax_strcmp(ck, dk) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										dk->declaration->position.line, dk->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							{
								syntax_error(program, d, "generic without key");
								return -1;
							}
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "class without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "class without key");
		return -1;
	}

	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			int32_t result;
			result = syntax_generics(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PROPERTY))
		{
			int32_t result;
			result = syntax_property(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			int32_t result;
			result = syntax_function(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_field(program_t *program, symbol_t *current)
{
	symbol_t *ck;
	ck = syntax_extract_with(current, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = current->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != current);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_FIELD))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (symbol_check_type(ck, SYMBOL_ID))
						{
							if (symbol_check_type(ak, SYMBOL_ID))
							{
								if ((syntax_strcmp(ck, ak) == 0) && (current != a))
								{
									syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
										ak->declaration->position.line, ak->declaration->position.column);
									return -1;
								}
								else
								{
									continue;
								}
							}
							else
							if (symbol_check_type(ak, SYMBOL_COMPOSITE))
							{
								symbol_t *akk;
								akk = syntax_extract_with(ak, SYMBOL_KEY);
								if (akk)
								{
									if (symbol_check_type(akk, SYMBOL_ID))
									{
										if (syntax_strcmp(ck, akk) == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												akk->declaration->position.line, akk->declaration->position.column);
											return -1;
										}
									}
									else
									{
										syntax_error(program, akk, "keyword is not an id");
										return -1;
									}
								}
								else
								{
									syntax_error(program, ak, "composite without key");
									return -1;
								}
							}
							else
							{
								syntax_error(program, ak, "unknown key");
								return -1;
							}
						}
						else
						if (symbol_check_type(ck, SYMBOL_COMPOSITE))
						{
							if (symbol_check_type(ak, SYMBOL_ID))
							{
								symbol_t *ckk;
								ckk = syntax_extract_with(ck, SYMBOL_KEY);
								if (ckk)
								{
									if (symbol_check_type(ckk, SYMBOL_ID))
									{
										if (syntax_strcmp(ak, ckk) == 0)
										{
											syntax_error(program, ckk, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
										else
										{
											continue;
										}
									}
									else
									{
										syntax_error(program, ckk, "keyword is not an id");
										return -1;
									}
								}
								else
								{
									syntax_error(program, ak, "composite without key");
									return -1;
								}
							}
							else
							if (symbol_check_type(ak, SYMBOL_COMPOSITE))
							{
								symbol_t *ckk;
								ckk = syntax_extract_with(ck, SYMBOL_KEY);
								if (ckk)
								{
									symbol_t *akk;
									akk = syntax_extract_with(ak, SYMBOL_KEY);
									if (akk)
									{
										if (syntax_strcmp(akk, ckk) == 0)
										{
											syntax_error(program, ckk, "defination repeated, another defination in %lld:%lld",
												akk->declaration->position.line, akk->declaration->position.column);
											return -1;
										}
										else
										{
											continue;
										}
									}
									else
									{
										syntax_error(program, ak, "composite without key");
										return -1;
									}
								}
								else
								{
									syntax_error(program, ck, "composite without key");
									return -1;
								}
							}
						}
						else
						{
							syntax_error(program, ck, "unknown key");
							return -1;
						}
					}
					else
					{
						syntax_error(program, a, "field without key");
						return -1;
					}
				}
			}
		}
		else
		{
			syntax_error(program, current, "field without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, current, "field without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_fields(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_FIELD))
		{
			int32_t result;
			result = syntax_field(program, a);
			if (result == -1)
			{
				return -1;
			}
			
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak)
			{
				symbol_t *parent = current->parent;
				if (parent)
				{
					symbol_t *root = parent->parent;
					if (root)
					{
						symbol_t *b;
						for (b = root->begin;(b != root->end) && (b != parent); b = b->next)
						{
							if (symbol_check_type(b, SYMBOL_CLASS))
							{
								if (symbol_check_type(ak, SYMBOL_ID))
								{
									symbol_t *bk;
									bk = syntax_extract_with(b, SYMBOL_KEY);
									if (bk)
									{
										if (syntax_strcmp(ak, bk) == 0)
										{
											symbol_t *bgs;
											bgs = syntax_only_with(b, SYMBOL_GENERICS);
											if (bgs)
											{
												int32_t no_value = -1;
												symbol_t *c;
												for (c = bgs->begin; c != bgs->end; c = c->next)
												{
													if (symbol_check_type(ak, SYMBOL_GENERIC))
													{
														symbol_t *cv;
														cv = syntax_only_with(c, SYMBOL_VALUE);
														if (!cv)
														{
															no_value = 1;
															break;
														}
													}
												}
												if (no_value == -1)
												{
													syntax_error(program, ak, "defination repeated, another defination in %lld:%lld",
														bk->declaration->position.line, bk->declaration->position.column);
													return -1;
												}
												else
												{
													continue;
												}
											}
											else
											{
												syntax_error(program, ak, "defination repeated, another defination in %lld:%lld",
													bk->declaration->position.line, bk->declaration->position.column);
												return -1;
											}
										}
										else
										{
											continue;
										}
									}
									else
									{
										syntax_error(program, b, "class without key");
										return -1;
									}
								}
								else
								if (symbol_check_type(ak, SYMBOL_COMPOSITE))
								{
									symbol_t *bk;
									bk = syntax_extract_with(b, SYMBOL_KEY);
									if (bk)
									{
										symbol_t *akk;
										akk = syntax_extract_with(ak, SYMBOL_KEY);	
										if (akk)
										{
											if (syntax_strcmp(akk, bk) == 0)
											{
												syntax_error(program, akk, "defination repeated, another defination in %lld:%lld",
													bk->declaration->position.line, bk->declaration->position.column);
												return -1;
											}
											else
											{
												continue;
											}
										}
										else
										{
											syntax_error(program, ak, "composite without key");
											return -1;
										}
									}
									else
									{
										syntax_error(program, b, "class without key");
										return -1;
									}
								}
								else
								{
									syntax_error(program, ak, "unknown key");
									return -1;
								}
							}
						}
					}
					else
					{
						syntax_error(program, parent, "import without parent");
						return -1;
					}
				}
				else
				{
					syntax_error(program, current, "fields without parent");
					return -1;
				}
			}
			else
			{
				syntax_error(program, a, "field without key");
				return -1;
			}

			continue;
		}
	}
	return 1;
}

static int32_t
syntax_import(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PATH))
		{
			symbol_t *sym_path;
			sym_path = syntax_only(a);

			node_t *node_path = sym_path->declaration;
			node_basic_t *basic_path = (node_basic_t *)node_path->value;
			char *path = basic_path->value;

			char *base_path = malloc(_MAX_DIR + _MAX_FNAME + _MAX_EXT);
			memset(base_path, 0, _MAX_DIR + _MAX_FNAME + _MAX_EXT);

			char *base_file = malloc(_MAX_DIR + _MAX_FNAME + _MAX_EXT);
			memset(base_file, 0, _MAX_DIR + _MAX_FNAME + _MAX_EXT);

			if (path_is_root(path))
			{
				path_normalize(getenv ("QALAM-PATH"), base_path, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
				path_join(base_path, path+2, base_file, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
			}
			else
			{
				path_get_current_directory(base_path, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
				if(path_is_relative(path))
				{
					path_join(base_path, path, base_file, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
				}
				else 
				{
					path_normalize(path, base_file, _MAX_DIR + _MAX_FNAME + _MAX_EXT);
				}
			}

			if (path_exist(base_file))
			{
				symbol_t *root = NULL;

				ilist_t *b;
				for (b = program->imports->begin;b != program->imports->end; b = b->next)
				{
					symbol_t *bs = (symbol_t *)b->value;

					node_t *node_bs = bs->declaration;
					node_basic_t *basic_bs = (node_basic_t *)node_bs->value;

					if (strncmp(basic_bs->value, base_file, 
						max(strlen(basic_bs->value), strlen(base_file))) == 0)
					{
						root = bs;
						break;
					}
				}

				if (root)
				{
					continue;
				}

				parser_t *parser;
				parser = parser_create(program, base_file);
				if(!parser)
				{
					return -1;
				}
				
				node_t *node;
				node = parser_module(program, parser);
				if(!node)
				{
					return -1;
				}

				node_t *node_path;
				node_path = node_make_string(node->position, base_file);

				root = symbol_create(SYMBOL_ROOT, node_path);
				if(!root)
				{
					return -1;
				}

				list_rpush(program->imports, (uint64_t)root);

				int32_t graph_result;
				graph_result = graph_run(program, root, node);
				if(graph_result == -1)
				{
					return -1;
				}

				int32_t syntax_result;
				syntax_result = syntax_run(program, root);
				if(syntax_result == -1)
				{
					return -1;
				}

				if(!symbol_link(current, current->end, root))
				{
					return -1;
				}

				continue;
			}
			else
			{
				syntax_error(program, current, "unable to access path");
				return -1;	
			}
		}

		if (symbol_check_type(a, SYMBOL_FIELDS))
		{
			int32_t result;
			result = syntax_fields(program, a);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	symbol_set_flag(current, SYMBOL_FLAG_SYNTAX);

	return 1;
}


static int32_t
syntax_module(program_t *program, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_IMPORT))
		{
			int32_t result;
			result = syntax_import(program, a);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(program, a);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(program, a);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_VAR))
		{
			int32_t result;
			result = syntax_var(program, a);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			int32_t result;
			result = syntax_function(program, a);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	symbol_set_flag(current, SYMBOL_FLAG_SYNTAX);
	return 1;
}

int32_t
syntax_run(program_t *program, symbol_t *root)
{
	symbol_t *a;
	for(a = root->begin; a != root->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_MODULE))
		{
			int32_t result;
			result = syntax_module(program, a);
			if(result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}


