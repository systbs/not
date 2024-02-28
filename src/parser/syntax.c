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
#include "../container/response.h"
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
syntax_idcmp(symbol_t *id1, symbol_t *id2)
{
	node_t *nid1 = id1->declaration;
	node_basic_t *nbid1 = (node_basic_t *)nid1->value;

	node_t *nid2 = id2->declaration;
	node_basic_t *nbid2 = (node_basic_t *)nid2->value;

	//printf("%s %s\n", nbid1->value, nbid2->value);

	return strcmp(nbid1->value, nbid2->value);
}

static int32_t
syntax_idstrcmp(symbol_t *id1, char *name)
{
	node_t *nid1 = id1->declaration;
	node_basic_t *nbid1 = (node_basic_t *)nid1->value;
	return strcmp(nbid1->value, name);
}


typedef enum syntax_route_type {
	SYNTAX_ROUTE_NONE 			= 0,
	SYNTAX_ROUTE_FORWARD
} syntax_route_type_t;

static int32_t
syntax_find(program_t *program, symbol_t *p, symbol_t *response);

static int32_t
syntax_match_gsas(program_t *program, symbol_t *gs1, symbol_t *as2)
{
	return 1;
}

static int32_t
syntax_find_in_scope(program_t *program, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route, symbol_t *applicant, symbol_t *response)
{
	symbol_t *a;
	for (a = base->begin;(a != base->end);a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak)
			{
				if (syntax_idcmp(ak, t1) == 0)
				{
					symbol_t *gs;
					gs = syntax_only_with(a, SYMBOL_GENERICS);
					if (gs)
					{
						if (arguments)
						{
							int32_t result;
							result = syntax_match_gsas(program, gs, arguments);
							if (result == -1)
							{
								return -1;
							}
							else
							if (result == 1)
							{
								symbol_t *nearest = a->parent;
								while (nearest)
								{
									if (symbol_check_type(nearest, SYMBOL_CLASS))
									{
										break;
									}
									nearest = nearest->parent;
								}

								if (nearest)
								{
									if (nearest->id != applicant->id)
									{
										node_class_t *class = response->declaration->value;
										if ((class->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
										{
											syntax_error(program, response, "private access");
											return -1;
										}
									}
								}

								*response = *a;
								return 1;
							}
						}
						else
						{
							int32_t no_value = 0;
							symbol_t *b;
							for (b = gs->begin;b != gs->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_VALUE);
									if (!bv)
									{
										no_value = 1;
										break;
									}
								}
							}
							if (no_value == 0)
							{
								symbol_t *nearest;
								nearest = a->parent;
								while (nearest)
								{
									if (symbol_check_type(nearest, SYMBOL_CLASS))
									{
										break;
									}
									nearest = nearest->parent;
								}

								if (nearest)
								{
									if (nearest->id != applicant->id)
									{
										node_class_t *class = response->declaration->value;
										if ((class->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
										{
											syntax_error(program, response, "private access");
											return -1;
										}
									}
								}

								*response = *a;
								return 1;
							}
						}
					}
					else
					{
						if (!arguments)
						{
							symbol_t *nearest;
							nearest = a->parent;
							while (nearest)
							{
								if (symbol_check_type(nearest, SYMBOL_CLASS))
								{
									break;
								}
								nearest = nearest->parent;
							}

							if (nearest)
							{
								if (nearest->id != applicant->id)
								{
									node_class_t *class = response->declaration->value;
									if ((class->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
									{
										syntax_error(program, response, "private access");
										return -1;
									}
								}
							}

							*response = *a;
							return 1;
						}
					}
				}
			}
			else
			{
				syntax_error(program, a, "class without key");
				return -1;
			}
			continue;
		}

		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_GENERIC))
				{
					symbol_t *bk;
					bk = syntax_extract_with(b, SYMBOL_KEY);
					if (bk)
					{
						if ((syntax_idcmp(bk, t1) == 0) && (bk != t1) && !arguments)
						{
							*response = *b;
							return 1;
						}
					}
				}
			}
			continue;
		}
	}

	if (symbol_check_type(base, SYMBOL_MODULE))
	{
		return 0;
	}

	return syntax_find_in_scope(program, base->parent, t1, arguments, route, applicant, response);
}

static int32_t
syntax_find_by_arguments(program_t *program, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route, symbol_t *applicant, symbol_t *response)
{
	if (symbol_check_type(t1, SYMBOL_ATTR))
	{
		symbol_t *left;
		left = syntax_extract_with(t1, SYMBOL_LEFT);
		if (left)
		{
			symbol_t *right;
			right = syntax_extract_with(t1, SYMBOL_RIGHT);
			if (right)
			{
				symbol_t r1;
				int32_t result;
				result = syntax_find_by_arguments(program, base, left, NULL, route, applicant, &r1);
				if (result == 1)
				{
					symbol_t r2;
					result = syntax_find_by_arguments(program, &r1, right, arguments, SYNTAX_ROUTE_FORWARD, applicant, &r2);
					if (result == 1)
					{
						*response = r2;
						return 1;
					}
					else
					if (result == -1)
					{
						return -1;
					}
					else
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
				}
				else
				if (result == -1)
				{
					return -1;
				}
				else
				{
					syntax_error(program, left, "reference not found");
					return -1;
				}
			}
			else
			{
				syntax_error(program, t1, "attribute does not include the rhs");
				return -1;
			}
		}
		else
		{
			syntax_error(program, t1, "attribute does not include the left field");
			return -1;
		}
	}
	else 
	if (symbol_check_type(t1, SYMBOL_COMPOSITE))
	{
		symbol_t *key;
		key = syntax_extract_with(t1, SYMBOL_KEY);
		if (key)
		{
			symbol_t *arguments1;
			arguments1 = syntax_extract_with(t1, SYMBOL_ARGUMENTS);
			if (arguments1)
			{
				symbol_t r1;
				int32_t result;
				result = syntax_find_by_arguments(program, base, key, arguments1, route, applicant, &r1);
				if (result == 1)
				{
					*response = r1;
					return 1;
				}
				else
				if (result == -1)
				{
					return -1;
				}
				else
				{
					syntax_error(program, key, "reference not found");
					return -1;
				}
			}
			else
			{
				syntax_error(program, t1, "attribute does not include the arguments");
				return -1;
			}
		}
		else
		{
			syntax_error(program, t1, "attribute does not include the key field");
			return -1;
		}
	}
	else
	if (symbol_check_type(t1, SYMBOL_ID))
	{
		int32_t result;
		result = syntax_find_in_scope(program, base, t1, arguments, route, applicant, response);
		if (result == 1)
		{
			return 1;
		}
		else
		if (result == -1)
		{
			return -1;
		}
		else
		{
			syntax_error(program, t1, "reference not found");
			return -1;
		}
	}
	else
	{
		syntax_error(program, t1, "reference is not a routable");
		return -1;
	}
}

static int32_t
syntax_find(program_t *program, symbol_t *p, symbol_t *response)
{
	symbol_t *applicant;
	applicant = p->parent;
	while (applicant)
	{
		if (symbol_check_type(applicant, SYMBOL_CLASS))
		{
			break;
		}
		if (applicant->parent)
		{
			applicant = applicant->parent;
		}
	}
	return syntax_find_by_arguments(program, p->parent, p, NULL, SYNTAX_ROUTE_NONE, applicant, response);
}

static void
syntax_xchg(symbol_t *a, symbol_t *b)
{
	symbol_t c = *a;
	*a = *b;
	*b = c;
}

static int32_t
syntax_equal_gsgs(program_t *program, symbol_t *gs1, symbol_t *gs2)
{
	int32_t changed = 0;
	uint64_t gcnt1;
	symbol_t *g1;
	region_type:
	gcnt1 = 0;
	for (g1 = gs1->begin;g1 != gs1->end;g1 = g1->next)
	{
		if (symbol_check_type(g1, SYMBOL_GENERIC))
		{
			gcnt1 += 1;
			uint64_t gcnt2 = 0;
			symbol_t *g2;
			for (g2 = gs2->begin;g2 != gs2->end;g2 = g2->next)
			{
				if (symbol_check_type(g2, SYMBOL_GENERIC))
				{
					gcnt2 += 1;
					if (gcnt2 < gcnt1)
					{
						continue;
					}
					symbol_t *gt1;
					gt1 = syntax_extract_with(g1, SYMBOL_TYPE);
					if (gt1)
					{
						symbol_t *gt2;
						gt2 = syntax_extract_with(g2, SYMBOL_TYPE);
						if (gt2)
						{
							symbol_t gr1;
							int32_t result;
							result = syntax_find(program, gt1, &gr1);
							if (result == 1)
							{
								symbol_t gr2;
								result = syntax_find(program, gt2, &gr2);
								if (result == 1)
								{
									if (gr1.id != gr2.id)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(program, gt2, "reference not found");
									return -1;
								}
							}
							else
							{
								syntax_error(program, gt1, "reference not found");
								return -1;
							}
						}
					}
				}
			}

			if (gcnt2 < gcnt1)
			{
				symbol_t *gv1;
				gv1 = syntax_only_with(g1, SYMBOL_VALUE);
				if (gv1 == NULL)
				{
					return 0;
				}
			}
		}
	}

	if (changed == 0)
	{
		changed = 1;
		syntax_xchg(gs1, gs2);
		goto region_type;
	}
	
	changed = 0;
	region_key:
	for (g1 = gs1->begin;g1 != gs1->end;g1 = g1->next)
	{
		if (symbol_check_type(g1, SYMBOL_GENERIC))
		{
			symbol_t *gk1;
			gk1 = syntax_extract_with(g1, SYMBOL_KEY);
			if (gk1)
			{
				int32_t found = 0;
				symbol_t *g2;
				for (g2 = gs2->begin;g2 != gs2->end;g2 = g2->next)
				{
					if (symbol_check_type(g2, SYMBOL_GENERIC))
					{
						symbol_t *gk2;
						gk2 = syntax_extract_with(g2, SYMBOL_KEY);
						if (gk2 != NULL)
						{
							if (syntax_idcmp(gk1, gk2) == 0)
							{
								found = 1;
								break;
							}
						}
					}
				}
				
				if (found == 0)
				{
					symbol_t *gv1;
					gv1 = syntax_only_with(g1, SYMBOL_VALUE);
					if (gv1 == NULL)
					{
						return 0;
					}
				}
			}
		}
	}

	if (changed == 0)
	{
		changed = 1;
		syntax_xchg(gs1, gs2);
		goto region_key;
	}
	else
	{
		return 1;
	}
}

static int32_t
syntax_equal_psps(program_t *program, symbol_t *ps1, symbol_t *ps2)
{
	int32_t changed = 0;
	symbol_t *p1;
	uint64_t gcnt1;
	region_type:
	gcnt1 = 0;
	for (p1 = ps1->begin;p1 != ps1->end;p1 = p1->next)
	{
		if (symbol_check_type(p1, SYMBOL_GENERIC))
		{
			gcnt1 += 1;
			uint64_t gcnt2 = 0;
			symbol_t *p2;
			for (p2 = ps2->begin;p2 != ps2->end;p2 = p2->next)
			{
				if (symbol_check_type(p2, SYMBOL_GENERIC))
				{
					gcnt2 += 1;
					if (gcnt2 < gcnt1)
					{
						continue;
					}
					symbol_t *gt1;
					gt1 = syntax_extract_with(p1, SYMBOL_TYPE);
					if (gt1)
					{
						symbol_t *gt2;
						gt2 = syntax_extract_with(p2, SYMBOL_TYPE);
						if (gt2)
						{
							symbol_t gr1;
							int32_t result;
							result = syntax_find(program, gt1, &gr1);
							if (result == 1)
							{
								symbol_t gr2;
								result = syntax_find(program, gt2, &gr2);
								if (result == 1)
								{
									if (gr1.id != gr2.id)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(program, gt2, "reference not found");
									return -1;
								}
							}
							else
							{
								syntax_error(program, gt1, "reference not found");
								return -1;
							}
						}
					}
				}
			}

			if (gcnt2 < gcnt1)
			{
				symbol_t *pv1;
				pv1 = syntax_only_with(p1, SYMBOL_VALUE);
				if (pv1 == NULL)
				{
					return 0;
				}
			}
		}
	}

	if (changed == 0)
	{
		changed = 1;
		syntax_xchg(ps1, ps2);
		goto region_type;
	}

	changed = 0;
	region_key:
	for (p1 = ps1->begin;p1 != ps1->end;p1 = p1->next)
	{
		if (symbol_check_type(p1, SYMBOL_GENERIC))
		{
			symbol_t *pk1;
			pk1 = syntax_extract_with(p1, SYMBOL_KEY);
			if (pk1)
			{
				int32_t found = 0;
				symbol_t *p2;
				for (p2 = ps2->begin;p2 != ps2->end;p2 = p2->next)
				{
					if (symbol_check_type(p2, SYMBOL_GENERIC))
					{
						symbol_t *pk2;
						pk2 = syntax_extract_with(p2, SYMBOL_KEY);
						if (pk2 != NULL)
						{
							if (syntax_idcmp(pk1, pk2) == 0)
							{
								found = 1;
								break;
							}
						}
					}
				}
				
				if (found == 0)
				{
					symbol_t *pv1;
					pv1 = syntax_only_with(p1, SYMBOL_VALUE);
					if (pv1 == NULL)
					{
						return 0;
					}
				}
			}
		}
	}

	if (changed == 0)
	{
		changed = 1;
		syntax_xchg(ps1, ps2);
		goto region_key;
	}
	else
	{
		return 1;
	}
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
							if (syntax_idcmp(ak, bk) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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

	symbol_t *ct;
	ct = syntax_extract_with(current, SYMBOL_TYPE);
	if (ct)
	{

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
							if (syntax_idcmp(ak, bk) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *ags;
							ags = syntax_only_with(a, SYMBOL_GENERICS);
							if (ags)
							{
								int32_t generic_without_value = 0;
								symbol_t *ag;
								for (ag = ags->begin;ag != ags->end;ag = ag->next)
								{
									if (symbol_check_type(ag, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(ag, SYMBOL_VALUE);
										if (!agv)
										{
											generic_without_value = 1;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *b2;
									for (b2 = a->begin;b2 != a->end;b2 = b2->next)
									{
										if (symbol_check_type(b2, SYMBOL_FUNCTION))
										{
											symbol_t *bk2;
											bk2 = syntax_extract_with(b2, SYMBOL_KEY);
											if (bk2)
											{
												if (syntax_idstrcmp(bk2, "constructor") == 0)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (!bpv2)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
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
										}
									}
								}
							}
							else
							{
								// check parameters
								symbol_t *b2;
								for (b2 = a->begin;b2 != a->end;b2 = b2->next)
								{
									if (symbol_check_type(b2, SYMBOL_FUNCTION))
									{
										symbol_t *bk2;
										bk2 = syntax_extract_with(b2, SYMBOL_KEY);
										if (bk2)
										{
											if (syntax_idstrcmp(bk2, "constructor") == 0)
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
												if (bps2)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (!bpv2)
														{
															parameter_without_value = 1;
															break;
														}
													}
													if (parameter_without_value == 0)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															ak->declaration->position.line, ak->declaration->position.column);
														return -1;
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
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *ags;
							ags = syntax_only_with(a, SYMBOL_GENERICS);
							if (ags)
							{
								int32_t generic_without_value = 0;
								symbol_t *ag;
								for (ag = ags->begin;ag != ags->end;ag = ag->next)
								{
									if (symbol_check_type(ag, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(ag, SYMBOL_VALUE);
										if (!agv)
										{
											generic_without_value = 1;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (!bpv2)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
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
								// check parameters
								symbol_t *bps2;
								bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
								if (bps2)
								{
									int32_t parameter_without_value = 0;
									symbol_t *bp2;
									for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
									{
										symbol_t *bpv2;
										bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
										if (!bpv2)
										{
											parameter_without_value = 1;
											break;
										}
									}
									if (parameter_without_value == 0)
									{
										syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
											ak->declaration->position.line, ak->declaration->position.column);
										return -1;
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
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
								if (syntax_idcmp(ck, dk) == 0)
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
								if (syntax_idcmp(ck, dk) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *ags;
							ags = syntax_only_with(a, SYMBOL_GENERICS);
							if (ags)
							{
								int32_t generic_without_value = 0;
								symbol_t *ag;
								for (ag = ags->begin;ag != ags->end;ag = ag->next)
								{
									if (symbol_check_type(ag, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(ag, SYMBOL_VALUE);
										if (!agv)
										{
											generic_without_value = 1;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *b2;
									for (b2 = a->begin;b2 != a->end;b2 = b2->next)
									{
										if (symbol_check_type(b2, SYMBOL_FUNCTION))
										{
											symbol_t *bk2;
											bk2 = syntax_extract_with(b2, SYMBOL_KEY);
											if (bk2)
											{
												if (syntax_idstrcmp(bk2, "constructor") == 0)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (!bpv2)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
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
										}
									}
								}
							}
							else
							{
								// check parameters
								symbol_t *b2;
								for (b2 = a->begin;b2 != a->end;b2 = b2->next)
								{
									if (symbol_check_type(b2, SYMBOL_FUNCTION))
									{
										symbol_t *bk2;
										bk2 = syntax_extract_with(b2, SYMBOL_KEY);
										if (bk2)
										{
											if (syntax_idstrcmp(bk2, "constructor") == 0)
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
												if (bps2)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (!bpv2)
														{
															parameter_without_value = 1;
															break;
														}
													}
													if (parameter_without_value == 0)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															ak->declaration->position.line, ak->declaration->position.column);
														return -1;
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
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *ags;
							ags = syntax_only_with(a, SYMBOL_GENERICS);
							if (ags)
							{
								int32_t generic_without_value = 0;
								symbol_t *ag;
								for (ag = ags->begin;ag != ags->end;ag = ag->next)
								{
									if (symbol_check_type(ag, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(ag, SYMBOL_VALUE);
										if (!agv)
										{
											generic_without_value = 1;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (!bpv2)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
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
								// check parameters
								symbol_t *bps2;
								bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
								if (bps2)
								{
									int32_t parameter_without_value = 0;
									symbol_t *bp2;
									for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
									{
										symbol_t *bpv2;
										bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
										if (!bpv2)
										{
											parameter_without_value = 1;
											break;
										}
									}
									if (parameter_without_value == 0)
									{
										syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
											ak->declaration->position.line, ak->declaration->position.column);
										return -1;
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
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_idcmp(ck, ak) == 0)
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
						if (syntax_idcmp(ck, ak) == 0)
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
								if (syntax_idcmp(ck, dk) == 0)
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
								if (syntax_idcmp(ck, dk) == 0)
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
		int32_t result;
		result = syntax_idstrcmp(ck, "constructor");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "constructor with generic");
				return -1;
			}
		}

		result = syntax_idstrcmp(ck, "+");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator + with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator + with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "-");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator - with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator + with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "*");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator * with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator * with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "/");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator / with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator / with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "**");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator ** with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator ** with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "%");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator %% with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator %% with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "&");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator & with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator & with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "|");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator | with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator | with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "^");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator ^ with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator ^ with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, ">>");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator >> with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator >> with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "<<");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator << with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator << with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, ">");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator > with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator > with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "<");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator < with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(current, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, current, "operator < with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "[]");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(current, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, current, "operator [] with generics");
				return -1;
			}
		}

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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, cgs, ags);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *b2;
										for (b2 = a->begin;b2 != a->end;b2 = b2->next)
										{
											if (symbol_check_type(b2, SYMBOL_FUNCTION))
											{
												symbol_t *bk2;
												bk2 = syntax_extract_with(b2, SYMBOL_KEY);
												if (bk2)
												{
													if (syntax_idstrcmp(bk2, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
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
												}
											}
										}
									}
								}
								else
								{
									int32_t generic_without_value = 0;
									symbol_t *cg;
									for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
									{
										if (symbol_check_type(cg, SYMBOL_GENERIC))
										{
											symbol_t *cgv;
											cgv = syntax_only_with(cg, SYMBOL_VALUE);
											if (!cgv)
											{
												generic_without_value = 1;
												break;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b2;
										for (b2 = a->begin;b2 != a->end;b2 = b2->next)
										{
											if (symbol_check_type(b2, SYMBOL_FUNCTION))
											{
												symbol_t *bk2;
												bk2 = syntax_extract_with(b2, SYMBOL_KEY);
												if (bk2)
												{
													if (syntax_idstrcmp(bk2, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
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
												}
											}
										}
									}
								}
							}
							else
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t generic_without_value = 0;
									symbol_t *ag;
									for (ag = ags->begin;ag != ags->end;ag = ag->next)
									{
										if (symbol_check_type(ag, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(ag, SYMBOL_VALUE);
											if (!agv)
											{
												generic_without_value = 1;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b2;
										for (b2 = a->begin;b2 != a->end;b2 = b2->next)
										{
											if (symbol_check_type(b2, SYMBOL_FUNCTION))
											{
												symbol_t *bk2;
												bk2 = syntax_extract_with(b2, SYMBOL_KEY);
												if (bk2)
												{
													if (syntax_idstrcmp(bk2, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
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
												}
											}
										}
									}
								}
								else
								{
									// check parameters
									symbol_t *b2;
									for (b2 = a->begin;b2 != a->end;b2 = b2->next)
									{
										if (symbol_check_type(b2, SYMBOL_FUNCTION))
										{
											symbol_t *bk2;
											bk2 = syntax_extract_with(b2, SYMBOL_KEY);
											if (bk2)
											{
												if (syntax_idstrcmp(bk2, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
													if (bps1)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t result;
															result = syntax_equal_psps(program, bps1, bps2);
															if (result == -1)
															{
																return -1;
															}
															else 
															if (result == 1)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
														else
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp1;
															for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
															{
																symbol_t *bpv1;
																bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																if (!bpv1)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
													}
													else
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (!bpv2)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
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
											}
										}
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, cgs, ags);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *bps1;
										bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (bps1)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t result;
												result = syntax_equal_psps(program, bps1, bps2);
												if (result == -1)
												{
													return -1;
												}
												else 
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
											}
											else
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp1;
												for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
												{
													symbol_t *bpv1;
													bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
													if (!bpv1)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
											}
										}
										else
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (!bpv2)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
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
								}
								else
								{
									int32_t generic_without_value = 0;
									symbol_t *cg;
									for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
									{
										if (symbol_check_type(cg, SYMBOL_GENERIC))
										{
											symbol_t *cgv;
											cgv = syntax_only_with(cg, SYMBOL_VALUE);
											if (!cgv)
											{
												generic_without_value = 1;
												break;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *bps1;
										bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (bps1)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t result;
												result = syntax_equal_psps(program, bps1, bps2);
												if (result == -1)
												{
													return -1;
												}
												else 
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
											}
											else
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp1;
												for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
												{
													symbol_t *bpv1;
													bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
													if (!bpv1)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
											}
										}
										else
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (!bpv2)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
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
								}
							}
							else
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t generic_without_value = 0;
									symbol_t *ag;
									for (ag = ags->begin;ag != ags->end;ag = ag->next)
									{
										if (symbol_check_type(ag, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(ag, SYMBOL_VALUE);
											if (!agv)
											{
												generic_without_value = 1;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *bps1;
										bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (bps1)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t result;
												result = syntax_equal_psps(program, bps1, bps2);
												if (result == -1)
												{
													return -1;
												}
												else 
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
											}
											else
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp1;
												for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
												{
													symbol_t *bpv1;
													bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
													if (!bpv1)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
											}
										}
										else
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (!bpv2)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
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
								}
								else
								{
									// check parameters
									symbol_t *bps1;
									bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (bps1)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t result;
											result = syntax_equal_psps(program, bps1, bps2);
											if (result == -1)
											{
												return -1;
											}
											else 
											if (result == 1)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
										else
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp1;
											for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
											{
												symbol_t *bpv1;
												bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
												if (!bpv1)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
									}
									else
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (!bpv2)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
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
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (!cgv)
										{
											generic_without_value = 1;
											break;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *bps1;
									bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (bps1)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t result;
											result = syntax_equal_psps(program, bps1, bps2);
											if (result == -1)
											{
												return -1;
											}
											else 
											if (result == 1)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
										else
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp1;
											for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
											{
												symbol_t *bpv1;
												bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
												if (!bpv1)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
									}
									else
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (!bpv2)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
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
							}
							else
							{
								// check parameters
								symbol_t *bps1;
								bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
								if (bps1)
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t result;
										result = syntax_equal_psps(program, bps1, bps2);
										if (result == -1)
										{
											return -1;
										}
										else 
										if (result == 1)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
									}
									else
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp1;
										for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
										{
											symbol_t *bpv1;
											bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
											if (!bpv1)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
									}
								}
								else
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (!bpv2)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (!cgv)
										{
											generic_without_value = 1;
											break;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *bps1;
									bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (bps1)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t result;
											result = syntax_equal_psps(program, bps1, bps2);
											if (result == -1)
											{
												return -1;
											}
											else 
											if (result == 1)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
										else
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp1;
											for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
											{
												symbol_t *bpv1;
												bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
												if (!bpv1)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
									}
									else
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (!bpv2)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
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
							}
							else
							{
								// check parameters
								symbol_t *bps1;
								bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
								if (bps1)
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t result;
										result = syntax_equal_psps(program, bps1, bps2);
										if (result == -1)
										{
											return -1;
										}
										else 
										if (result == 1)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
									}
									else
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp1;
										for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
										{
											symbol_t *bpv1;
											bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
											if (!bpv1)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
									}
								}
								else
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (!bpv2)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (!cgv)
										{
											generic_without_value = 1;
											break;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *bps1;
									bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (bps1)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t result;
											result = syntax_equal_psps(program, bps1, bps2);
											if (result == -1)
											{
												return -1;
											}
											else 
											if (result == 1)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
										else
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp1;
											for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
											{
												symbol_t *bpv1;
												bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
												if (!bpv1)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
										}
									}
									else
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (!bpv2)
												{
													parameter_without_value = 1;
													break;
												}
											}
											if (parameter_without_value == 0)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
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
							}
							else
							{
								// check parameters
								symbol_t *bps1;
								bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
								if (bps1)
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t result;
										result = syntax_equal_psps(program, bps1, bps2);
										if (result == -1)
										{
											return -1;
										}
										else 
										if (result == 1)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
									}
									else
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp1;
										for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
										{
											symbol_t *bpv1;
											bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
											if (!bpv1)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
										}
									}
								}
								else
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (!bpv2)
											{
												parameter_without_value = 1;
												break;
											}
										}
										if (parameter_without_value == 0)
										{
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												ak->declaration->position.line, ak->declaration->position.column);
											return -1;
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
								if (syntax_idcmp(ck, dk) == 0)
								{
									symbol_t *cgs;
									cgs = syntax_only_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (!cgv)
												{
													generic_without_value = 1;
													break;
												}
											}
										}
										if (generic_without_value == 0)
										{
											// check parameters
											symbol_t *bps1;
											bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
											if (bps1)
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
												if (bps2)
												{
													int32_t result;
													result = syntax_equal_psps(program, bps1, bps2);
													if (result == -1)
													{
														return -1;
													}
													else 
													if (result == 1)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															dk->declaration->position.line, dk->declaration->position.column);
														return -1;
													}
												}
												else
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp1;
													for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
													{
														symbol_t *bpv1;
														bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
														if (!bpv1)
														{
															parameter_without_value = 1;
															break;
														}
													}
													if (parameter_without_value == 0)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															dk->declaration->position.line, dk->declaration->position.column);
														return -1;
													}
												}
											}
											else
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
												if (bps2)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (!bpv2)
														{
															parameter_without_value = 1;
															break;
														}
													}
													if (parameter_without_value == 0)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															dk->declaration->position.line, dk->declaration->position.column);
														return -1;
													}
												}
												else
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
										}
									}
									else
									{
										// check parameters
										symbol_t *bps1;
										bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (bps1)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t result;
												result = syntax_equal_psps(program, bps1, bps2);
												if (result == -1)
												{
													return -1;
												}
												else 
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
											else
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp1;
												for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
												{
													symbol_t *bpv1;
													bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
													if (!bpv1)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
										}
										else
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (!bpv2)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
											else
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													dk->declaration->position.line, dk->declaration->position.column);
												return -1;
											}
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
								if (syntax_idcmp(ck, dk) == 0)
								{
									symbol_t *cgs;
									cgs = syntax_only_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (!cgv)
												{
													generic_without_value = 1;
													break;
												}
											}
										}
										if (generic_without_value == 0)
										{
											// check parameters
											symbol_t *bps1;
											bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
											if (bps1)
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
												if (bps2)
												{
													int32_t result;
													result = syntax_equal_psps(program, bps1, bps2);
													if (result == -1)
													{
														return -1;
													}
													else 
													if (result == 1)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															dk->declaration->position.line, dk->declaration->position.column);
														return -1;
													}
												}
												else
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp1;
													for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
													{
														symbol_t *bpv1;
														bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
														if (!bpv1)
														{
															parameter_without_value = 1;
															break;
														}
													}
													if (parameter_without_value == 0)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															dk->declaration->position.line, dk->declaration->position.column);
														return -1;
													}
												}
											}
											else
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
												if (bps2)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (!bpv2)
														{
															parameter_without_value = 1;
															break;
														}
													}
													if (parameter_without_value == 0)
													{
														syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
															dk->declaration->position.line, dk->declaration->position.column);
														return -1;
													}
												}
												else
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
										}
									}
									else
									{
										// check parameters
										symbol_t *bps1;
										bps1 = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (bps1)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t result;
												result = syntax_equal_psps(program, bps1, bps2);
												if (result == -1)
												{
													return -1;
												}
												else 
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
											else
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp1;
												for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
												{
													symbol_t *bpv1;
													bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
													if (!bpv1)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
										}
										else
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
											if (bps2)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (!bpv2)
													{
														parameter_without_value = 1;
														break;
													}
												}
												if (parameter_without_value == 0)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														dk->declaration->position.line, dk->declaration->position.column);
													return -1;
												}
											}
											else
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													dk->declaration->position.line, dk->declaration->position.column);
												return -1;
											}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, cgs, ags);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *b2;
														for (b2 = a->begin;b2 != a->end;b2 = b2->next)
														{
															if (symbol_check_type(b2, SYMBOL_FUNCTION))
															{
																symbol_t *bk2;
																bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																if (bk2)
																{
																	if (syntax_idstrcmp(bk2, "constructor") == 0)
																	{
																		symbol_t *bps1;
																		bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																		if (bps1)
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2)
																			{
																				int32_t result;
																				result = syntax_equal_psps(program, bps1, bps2);
																				if (result == -1)
																				{
																					return -1;
																				}
																				else 
																				if (result == 1)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
																				}
																			}
																			else
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp1;
																				for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																				{
																					symbol_t *bpv1;
																					bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																					if (!bpv1)
																					{
																						parameter_without_value = 1;
																						break;
																					}
																				}
																				if (parameter_without_value == 0)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
																				}
																			}
																		}
																		else
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2)
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp2;
																				for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																				{
																					symbol_t *bpv2;
																					bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																					if (!bpv2)
																					{
																						parameter_without_value = 1;
																						break;
																					}
																				}
																				if (parameter_without_value == 0)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
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
																}
															}
														}
													}
												}
											}
										}
									}
								}
								else
								{
									int32_t generic_without_value = 0;
									symbol_t *cg;
									for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
									{
										if (symbol_check_type(cg, SYMBOL_GENERIC))
										{
											symbol_t *cgv;
											cgv = syntax_only_with(cg, SYMBOL_VALUE);
											if (!cgv)
											{
												generic_without_value = 1;
												break;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *b2;
														for (b2 = a->begin;b2 != a->end;b2 = b2->next)
														{
															if (symbol_check_type(b2, SYMBOL_FUNCTION))
															{
																symbol_t *bk2;
																bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																if (bk2)
																{
																	if (syntax_idstrcmp(bk2, "constructor") == 0)
																	{
																		symbol_t *bps1;
																		bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																		if (bps1)
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2)
																			{
																				int32_t result;
																				result = syntax_equal_psps(program, bps1, bps2);
																				if (result == -1)
																				{
																					return -1;
																				}
																				else 
																				if (result == 1)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
																				}
																			}
																			else
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp1;
																				for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																				{
																					symbol_t *bpv1;
																					bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																					if (!bpv1)
																					{
																						parameter_without_value = 1;
																						break;
																					}
																				}
																				if (parameter_without_value == 0)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
																				}
																			}
																		}
																		else
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2)
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp2;
																				for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																				{
																					symbol_t *bpv2;
																					bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																					if (!bpv2)
																					{
																						parameter_without_value = 1;
																						break;
																					}
																				}
																				if (parameter_without_value == 0)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
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
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
							else
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t generic_without_value = 0;
									symbol_t *ag;
									for (ag = ags->begin;ag != ags->end;ag = ag->next)
									{
										if (symbol_check_type(ag, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(ag, SYMBOL_VALUE);
											if (!agv)
											{
												generic_without_value = 1;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *b2;
														for (b2 = a->begin;b2 != a->end;b2 = b2->next)
														{
															if (symbol_check_type(b2, SYMBOL_FUNCTION))
															{
																symbol_t *bk2;
																bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																if (bk2)
																{
																	if (syntax_idstrcmp(bk2, "constructor") == 0)
																	{
																		symbol_t *bps1;
																		bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																		if (bps1)
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2)
																			{
																				int32_t result;
																				result = syntax_equal_psps(program, bps1, bps2);
																				if (result == -1)
																				{
																					return -1;
																				}
																				else 
																				if (result == 1)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
																				}
																			}
																			else
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp1;
																				for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																				{
																					symbol_t *bpv1;
																					bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																					if (!bpv1)
																					{
																						parameter_without_value = 1;
																						break;
																					}
																				}
																				if (parameter_without_value == 0)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
																				}
																			}
																		}
																		else
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2)
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp2;
																				for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																				{
																					symbol_t *bpv2;
																					bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																					if (!bpv2)
																					{
																						parameter_without_value = 1;
																						break;
																					}
																				}
																				if (parameter_without_value == 0)
																				{
																					syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																						ak->declaration->position.line, ak->declaration->position.column);
																					return -1;
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
																}
															}
														}
													}
												}
											}
										}
									}
								}
								else
								{
									// check parameters
									symbol_t *b1;
									for (b1 = current->begin;b1 != current->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *b2;
													for (b2 = a->begin;b2 != a->end;b2 = b2->next)
													{
														if (symbol_check_type(b2, SYMBOL_FUNCTION))
														{
															symbol_t *bk2;
															bk2 = syntax_extract_with(b2, SYMBOL_KEY);
															if (bk2)
															{
																if (syntax_idstrcmp(bk2, "constructor") == 0)
																{
																	symbol_t *bps1;
																	bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																	if (bps1)
																	{
																		symbol_t *bps2;
																		bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																		if (bps2)
																		{
																			int32_t result;
																			result = syntax_equal_psps(program, bps1, bps2);
																			if (result == -1)
																			{
																				return -1;
																			}
																			else 
																			if (result == 1)
																			{
																				syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																					ak->declaration->position.line, ak->declaration->position.column);
																				return -1;
																			}
																		}
																		else
																		{
																			int32_t parameter_without_value = 0;
																			symbol_t *bp1;
																			for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																			{
																				symbol_t *bpv1;
																				bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																				if (!bpv1)
																				{
																					parameter_without_value = 1;
																					break;
																				}
																			}
																			if (parameter_without_value == 0)
																			{
																				syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																					ak->declaration->position.line, ak->declaration->position.column);
																				return -1;
																			}
																		}
																	}
																	else
																	{
																		symbol_t *bps2;
																		bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																		if (bps2)
																		{
																			int32_t parameter_without_value = 0;
																			symbol_t *bp2;
																			for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																			{
																				symbol_t *bpv2;
																				bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																				if (!bpv2)
																				{
																					parameter_without_value = 1;
																					break;
																				}
																			}
																			if (parameter_without_value == 0)
																			{
																				syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																					ak->declaration->position.line, ak->declaration->position.column);
																				return -1;
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
															}
														}
													}
												}
											}
										}
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, cgs, ags);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
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
												}
											}
										}
									}
								}
								else
								{
									int32_t generic_without_value = 0;
									symbol_t *cg;
									for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
									{
										if (symbol_check_type(cg, SYMBOL_GENERIC))
										{
											symbol_t *cgv;
											cgv = syntax_only_with(cg, SYMBOL_VALUE);
											if (!cgv)
											{
												generic_without_value = 1;
												break;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
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
												}
											}
										}
									}
								}
							}
							else
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t generic_without_value = 0;
									symbol_t *ag;
									for (ag = ags->begin;ag != ags->end;ag = ag->next)
									{
										if (symbol_check_type(ag, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(ag, SYMBOL_VALUE);
											if (!agv)
											{
												generic_without_value = 1;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		ak->declaration->position.line, ak->declaration->position.column);
																	return -1;
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
												}
											}
										}
									}
								}
								else
								{
									// check parameters
									symbol_t *b1;
									for (b1 = current->begin;b1 != current->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t result;
															result = syntax_equal_psps(program, bps1, bps2);
															if (result == -1)
															{
																return -1;
															}
															else 
															if (result == 1)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
														else
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp1;
															for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
															{
																symbol_t *bpv1;
																bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																if (!bpv1)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
													}
													else
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (!bpv2)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
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
											}
										}
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (!cgv)
										{
											generic_without_value = 1;
											break;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *b1;
									for (b1 = current->begin;b1 != current->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t result;
															result = syntax_equal_psps(program, bps1, bps2);
															if (result == -1)
															{
																return -1;
															}
															else 
															if (result == 1)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
														else
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp1;
															for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
															{
																symbol_t *bpv1;
																bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																if (!bpv1)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
													}
													else
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (!bpv2)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
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
											}
										}
									}
								}
							}
							else
							{
								// check parameters
								symbol_t *b1;
								for (b1 = current->begin;b1 != current->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "constructor") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t result;
														result = syntax_equal_psps(program, bps1, bps2);
														if (result == -1)
														{
															return -1;
														}
														else 
														if (result == 1)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
														}
													}
													else
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp1;
														for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
														{
															symbol_t *bpv1;
															bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
															if (!bpv1)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
														}
													}
												}
												else
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (!bpv2)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
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
										}
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (!cgv)
										{
											generic_without_value = 1;
											break;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *b1;
									for (b1 = current->begin;b1 != current->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t result;
															result = syntax_equal_psps(program, bps1, bps2);
															if (result == -1)
															{
																return -1;
															}
															else 
															if (result == 1)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
														else
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp1;
															for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
															{
																symbol_t *bpv1;
																bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																if (!bpv1)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
													}
													else
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (!bpv2)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
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
											}
										}
									}
								}
							}
							else
							{
								// check parameters
								symbol_t *b1;
								for (b1 = current->begin;b1 != current->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "constructor") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t result;
														result = syntax_equal_psps(program, bps1, bps2);
														if (result == -1)
														{
															return -1;
														}
														else 
														if (result == 1)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
														}
													}
													else
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp1;
														for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
														{
															symbol_t *bpv1;
															bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
															if (!bpv1)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
														}
													}
												}
												else
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (!bpv2)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
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
										}
									}
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
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (!cgv)
										{
											generic_without_value = 1;
											break;
										}
									}
								}
								if (generic_without_value == 0)
								{
									// check parameters
									symbol_t *b1;
									for (b1 = current->begin;b1 != current->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t result;
															result = syntax_equal_psps(program, bps1, bps2);
															if (result == -1)
															{
																return -1;
															}
															else 
															if (result == 1)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
														else
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp1;
															for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
															{
																symbol_t *bpv1;
																bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																if (!bpv1)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
															}
														}
													}
													else
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (!bpv2)
																{
																	parameter_without_value = 1;
																	break;
																}
															}
															if (parameter_without_value == 0)
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	ak->declaration->position.line, ak->declaration->position.column);
																return -1;
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
											}
										}
									}
								}
							}
							else
							{
								// check parameters
								symbol_t *b1;
								for (b1 = current->begin;b1 != current->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "constructor") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t result;
														result = syntax_equal_psps(program, bps1, bps2);
														if (result == -1)
														{
															return -1;
														}
														else 
														if (result == 1)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
														}
													}
													else
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp1;
														for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
														{
															symbol_t *bpv1;
															bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
															if (!bpv1)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
														}
													}
												}
												else
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (!bpv2)
															{
																parameter_without_value = 1;
																break;
															}
														}
														if (parameter_without_value == 0)
														{
															syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																ak->declaration->position.line, ak->declaration->position.column);
															return -1;
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
										}
									}
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
								if (syntax_idcmp(ck, dk) == 0)
								{
									symbol_t *cgs;
									cgs = syntax_only_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (!cgv)
												{
													generic_without_value = 1;
													break;
												}
											}
										}
										if (generic_without_value == 0)
										{
											// check parameters
											symbol_t *b1;
											for (b1 = current->begin;b1 != current->end;b1 = b1->next)
											{
												if (symbol_check_type(b1, SYMBOL_FUNCTION))
												{
													symbol_t *bk1;
													bk1 = syntax_extract_with(b1, SYMBOL_KEY);
													if (bk1)
													{
														if (syntax_idstrcmp(bk1, "constructor") == 0)
														{
															symbol_t *bps1;
															bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
															if (bps1)
															{
																symbol_t *bps2;
																bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
																if (bps2)
																{
																	int32_t result;
																	result = syntax_equal_psps(program, bps1, bps2);
																	if (result == -1)
																	{
																		return -1;
																	}
																	else 
																	if (result == 1)
																	{
																		syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																			dk->declaration->position.line, dk->declaration->position.column);
																		return -1;
																	}
																}
																else
																{
																	int32_t parameter_without_value = 0;
																	symbol_t *bp1;
																	for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																	{
																		symbol_t *bpv1;
																		bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																		if (!bpv1)
																		{
																			parameter_without_value = 1;
																			break;
																		}
																	}
																	if (parameter_without_value == 0)
																	{
																		syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																			dk->declaration->position.line, dk->declaration->position.column);
																		return -1;
																	}
																}
															}
															else
															{
																symbol_t *bps2;
																bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
																if (bps2)
																{
																	int32_t parameter_without_value = 0;
																	symbol_t *bp2;
																	for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																	{
																		symbol_t *bpv2;
																		bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																		if (!bpv2)
																		{
																			parameter_without_value = 1;
																			break;
																		}
																	}
																	if (parameter_without_value == 0)
																	{
																		syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																			dk->declaration->position.line, dk->declaration->position.column);
																		return -1;
																	}
																}
																else
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
														}
													}
												}
											}
										}
									}
									else
									{
										// check parameters
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	dk->declaration->position.line, dk->declaration->position.column);
																return -1;
															}
														}
													}
												}
											}
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
								if ((syntax_idcmp(ck, dk) == 0) && (current != a))
								{
									symbol_t *cgs;
									cgs = syntax_only_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (!cgv)
												{
													generic_without_value = 1;
													break;
												}
											}
										}
										if (generic_without_value == 0)
										{
											// check parameters
											symbol_t *b1;
											for (b1 = current->begin;b1 != current->end;b1 = b1->next)
											{
												if (symbol_check_type(b1, SYMBOL_FUNCTION))
												{
													symbol_t *bk1;
													bk1 = syntax_extract_with(b1, SYMBOL_KEY);
													if (bk1)
													{
														if (syntax_idstrcmp(bk1, "constructor") == 0)
														{
															symbol_t *bps1;
															bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
															if (bps1)
															{
																symbol_t *bps2;
																bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
																if (bps2)
																{
																	int32_t result;
																	result = syntax_equal_psps(program, bps1, bps2);
																	if (result == -1)
																	{
																		return -1;
																	}
																	else 
																	if (result == 1)
																	{
																		syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																			dk->declaration->position.line, dk->declaration->position.column);
																		return -1;
																	}
																}
																else
																{
																	int32_t parameter_without_value = 0;
																	symbol_t *bp1;
																	for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																	{
																		symbol_t *bpv1;
																		bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																		if (!bpv1)
																		{
																			parameter_without_value = 1;
																			break;
																		}
																	}
																	if (parameter_without_value == 0)
																	{
																		syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																			dk->declaration->position.line, dk->declaration->position.column);
																		return -1;
																	}
																}
															}
															else
															{
																symbol_t *bps2;
																bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
																if (bps2)
																{
																	int32_t parameter_without_value = 0;
																	symbol_t *bp2;
																	for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																	{
																		symbol_t *bpv2;
																		bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																		if (!bpv2)
																		{
																			parameter_without_value = 1;
																			break;
																		}
																	}
																	if (parameter_without_value == 0)
																	{
																		syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																			dk->declaration->position.line, dk->declaration->position.column);
																		return -1;
																	}
																}
																else
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
														}
													}
												}
											}
										}
									}
									else
									{
										// check parameters
										symbol_t *b1;
										for (b1 = current->begin;b1 != current->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t result;
																result = syntax_equal_psps(program, bps1, bps2);
																if (result == -1)
																{
																	return -1;
																}
																else 
																if (result == 1)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp1;
																for (bp1 = bps1->begin;bp1 != bps1->end;bps1 = bps1->next)
																{
																	symbol_t *bpv1;
																	bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																	if (!bpv1)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
															if (bps2)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (!bpv2)
																	{
																		parameter_without_value = 1;
																		break;
																	}
																}
																if (parameter_without_value == 0)
																{
																	syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																		dk->declaration->position.line, dk->declaration->position.column);
																	return -1;
																}
															}
															else
															{
																syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
																	dk->declaration->position.line, dk->declaration->position.column);
																return -1;
															}
														}
													}
												}
											}
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
								if (syntax_idcmp(ck, ak) == 0)
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
										if (syntax_idcmp(ck, akk) == 0)
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
										if (syntax_idcmp(ak, ckk) == 0)
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
										if (syntax_idcmp(akk, ckk) == 0)
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
										if (syntax_idcmp(ak, bk) == 0)
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
											if (syntax_idcmp(akk, bk) == 0)
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


