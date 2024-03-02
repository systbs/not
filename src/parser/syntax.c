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
#include "table.h"



static error_t *
syntax_error(program_t *program, symbol_t *target, const char *format, ...)
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
	node = target->declaration;

	error_t *error;
	error = error_create(node->position, message);
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
syntax_function(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_block(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_import(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_generics(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_generic(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_fields(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_field(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_expression(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_assign(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag);




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


typedef enum syntax_flag_type {
	SYNTAX_FLAG_NONE 			= 1 << 0,
	SYNTAX_FLAG_INIT			= 1 << 1
} syntax_flag_type_t;

typedef enum syntax_route_type {
	SYNTAX_ROUTE_NONE 			= 0,
	SYNTAX_ROUTE_FORWARD
} syntax_route_type_t;

static int32_t
syntax_find(program_t *program, symbol_t *p, symbol_t *response);

static int32_t
syntax_match_gsast(program_t *program, symbol_t *gs1, symbol_t *as1)
{
	int32_t use_value = 0, use_key = 0;
	uint64_t acnt1 = 0;
	symbol_t *a1;
	for (a1 = as1->begin;a1 != as1->end;a1 = a1->next)
	{
		if (symbol_check_type(a1, SYMBOL_ARGUMENT))
		{
			symbol_t *av1;
			av1 = syntax_extract_with(a1, SYMBOL_VALUE);
			if (av1 != NULL)
			{
				use_value = 1;
				if (use_key == 1)
				{
					syntax_error(program, a1, "all entries must be the same");
					return -1;
				}
				symbol_t *g1;
				for (g1 = gs1->begin;g1 != gs1->end;g1 = g1->next)
				{
					if (symbol_check_type(g1, SYMBOL_GENERIC))
					{
						symbol_t *gt1;
						gt1 = syntax_extract_with(g1, SYMBOL_TYPE);
						if (gt1)
						{
							symbol_t ar1;
							int32_t result;
							result = syntax_find(program, av1, &ar1);
							if (result == 1)
							{
								symbol_t gr1;
								result = syntax_find(program, gt1, &gr1);
								if (result == 1)
								{
									if (ar1.id != gr1.id)
									{
										return 0;
									}
								}
							}
						}
					}
				}
			}
			else
			{
				use_key = 1;
				if (use_value == 1)
				{
					syntax_error(program, a1, "all entries must be the same");
					return -1;
				}

				acnt1 += 1;
				uint64_t gcnt1 = 0;
				symbol_t *g1;
				for (g1 = gs1->begin;g1 != gs1->end;g1 = g1->next)
				{
					if (symbol_check_type(g1, SYMBOL_GENERIC))
					{
						gcnt1 += 1;
						if (gcnt1 < acnt1)
						{
							continue;
						}

						symbol_t *ak1;
						ak1 = syntax_extract_with(a1, SYMBOL_KEY);
						if (ak1 != NULL)
						{
							symbol_t *gk1;
							gk1 = syntax_extract_with(g1, SYMBOL_KEY);
							if (gk1 != NULL)
							{
								if (syntax_idcmp(ak1, gk1) != 0)
								{
									return 0;
								}
							}
						}
					}
				}
			}
			
		}
	}
	return 1;
}

static int32_t
syntax_find_in_scope(program_t *program, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route, symbol_t *response)
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
							result = syntax_match_gsast(program, gs, arguments);
							if (result == -1)
							{
								return -1;
							}
							else
							if (result == 1)
							{
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
								*response = *a;
								return 1;
							}
						}
					}
					else
					{
						if (!arguments)
						{
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

	return syntax_find_in_scope(program, base->parent, t1, arguments, route, response);
}

static int32_t
syntax_find_by_arguments(program_t *program, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route, symbol_t *response)
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
				result = syntax_find_by_arguments(program, base, left, NULL, route, &r1);
				if (result == 1)
				{
					symbol_t r2;
					result = syntax_find_by_arguments(program, &r1, right, arguments, SYNTAX_ROUTE_FORWARD, &r2);
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
						return 0;
					}
				}
				else
				if (result == -1)
				{
					return -1;
				}
				else
				{
					return 0;
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
				result = syntax_find_by_arguments(program, base, key, arguments1, route, &r1);
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
					return 0;
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
		result = syntax_find_in_scope(program, base, t1, arguments, route, response);
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
			return 0;
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
	return syntax_find_by_arguments(program, p->parent, p, NULL, SYNTAX_ROUTE_NONE, response);
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
								if (result == -1)
								{
									return -1;
								}
								else
								{
									syntax_error(program, gt2, "reference not found");
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
								if (result == -1)
								{
									return -1;
								}
								else
								{
									syntax_error(program, gt2, "reference not found");
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
syntax_match_psasv(program_t *program, symbol_t *ps1, symbol_t *as1)
{
	return 1;
}

static int32_t
syntax_match_pst(program_t *program, symbol_t *ps1, symbol_t *s1)
{
	return 1;
}



static int32_t
syntax_id(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idcmp(key, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_id(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_number(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idstrcmp(key, "Int") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					continue;
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_number(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_string(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idstrcmp(key, "String") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					continue;
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_string(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_char(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idstrcmp(key, "Char") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					continue;
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_char(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_null(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idstrcmp(key, "Null") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					continue;
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_null(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_true(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idstrcmp(key, "Boolean") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					continue;
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_true(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_false(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *key;
			key = syntax_extract_with(it1->original, SYMBOL_KEY);
			if (key != NULL)
			{
				if (syntax_idstrcmp(key, "Boolean") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					continue;
				}
			}
		}
	}

	if(frame->parent)
	{
		return syntax_false(program, scope, frame->parent, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_primary(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_ID))
	{
		return syntax_id(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_NUMBER))
	{
		return syntax_number(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_STRING))
	{
		return syntax_string(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_CHAR))
	{
		return syntax_char(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_NULL))
	{
		return syntax_null(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_TRUE))
	{
		return syntax_true(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_FALSE))
	{
		return syntax_false(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_PARENTHESIS))
	{
		return syntax_expression(program, scope, frame, syntax_only(target), response, flag);
	}
	else
	{
		itable_t *it1;
		it1 = table_new();
		if (it1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}
		it1->flag |= ITABLE_FLAG_TEMPORARY;
		it1->original = target;

		ilist_t *r1;
		r1 = list_rpush(response, it1);
		if (r1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}
		return 1;
	}
}

static int32_t
syntax_hierarchy(program_t *program, list_t *scope, table_t *frame, symbol_t *base, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a1;
	for (a1 = base->begin;a1 != base->end;a1 = a1->next)
	{
		if (symbol_check_type(a1, SYMBOL_CLASS))
		{
			symbol_t *ak1;
			ak1 = syntax_extract_with(a1, SYMBOL_KEY);
			if (ak1 != NULL)
			{
				if (syntax_idcmp(ak1, target) == 0)
				{
					itable_t *it1;
					it1 = table_rpush(frame, a1);
					if (it1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
					it1->flag |= ITABLE_FLAG_TEMPORARY;

					ilist_t *r1;
					r1 = list_rpush(response, it1);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	symbol_t *hs1;
	hs1 = syntax_only_with(base, SYMBOL_HERITAGES);
	if (hs1 != NULL)
	{
		symbol_t *h1;
		for (h1 = hs1->begin;h1 != hs1->end;h1 = h1->next)
		{
			if (symbol_check_type(h1, SYMBOL_HERITAGE))
			{
				symbol_t *ht1;
				ht1 = syntax_extract_with(h1, SYMBOL_TYPE);
				if (ht1 != NULL)
				{
					list_t response1;

					int32_t r1;
					r1 = syntax_expression(program, scope, frame, ht1, &response1, flag);
					if (r1 == -1)
					{
						return -1;
					}
					else
					if (r1 == 0)
					{
						syntax_error(program, ht1, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *b1;
						for (b1 = response1.begin;b1 != response1.end;b1 = b1->next)
						{
							ilist_t *br1 = (ilist_t *)b1->value;
							if (br1 != NULL)
							{
								symbol_t *brs1 = (symbol_t *)br1->value;
								if (symbol_check_type(brs1, SYMBOL_CLASS))
								{
									int32_t r2;
									r2 = syntax_hierarchy(program, scope, frame, brs1, target, response, flag);
									if (r2 == -1)
									{
										return -1;
									}
								}
								else
								{
									syntax_error(program, brs1, "reference not a class");
									return -1;
								}
							}
						}
					}
				}
			}
		}
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_attribute(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *left;
	left = syntax_extract_with(target, SYMBOL_LEFT);
	if (left)
	{
		list_t response1;
		
		list_t *r0;
		r0 = list_apply(&response1);
		if (r0 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, frame, left, &response1, flag);
		if (r1 == -1)
		{
			return -1;
		}
		else
		if (r1 == 0)
		{
			syntax_error(program, left, "reference not found");
			return -1;
		}
		else
		{
			ilist_t *a1;
			for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
			{
				itable_t *it1 = (itable_t *)a1->value;
				if (it1->original != NULL)
				{
					symbol_t *s1 = it1->original;
					if (symbol_check_type(s1, SYMBOL_CLASS))
					{
						symbol_t *right;
						right = syntax_extract_with(target, SYMBOL_RIGHT);
						if (right)
						{
							list_t response2;
		
							list_t *r2;
							r2 = list_apply(&response2);
							if (r2 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}

							int32_t r3;
							r3 = syntax_hierarchy(program, scope, frame, s1, right, &response2, flag);
							if (r3 == -1)
							{
								return -1;
							}
							else
							if (r3 == 1)
							{
								it1->reference += 1;
							}

							ilist_t *a2;
							for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
							{
								itable_t *it2 = (itable_t *)a2->value;
								if (it2 != NULL)
								{
									ilist_t *r4;
									r4 = list_rpush(response, it2);
									if (r4 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
										return -1;
									}
								}
							}
							list_clear(&response2);
						}
					}
					else
					if (symbol_check_type(s1, SYMBOL_ENUM))
					{
						symbol_t *right;
						right = syntax_extract_with(target, SYMBOL_RIGHT);
						if (right)
						{
							symbol_t *ms1;
							ms1 = syntax_extract_with(s1, SYMBOL_MEMBERS);
							if (ms1 != NULL)
							{
								symbol_t *m1;
								for (m1 = ms1->begin;m1 != ms1->end;m1 = m1->next)
								{
									if (symbol_check_type(m1, SYMBOL_MEMBER))
									{
										symbol_t *mk1;
										mk1 = syntax_extract_with(m1, SYMBOL_KEY);
										if (mk1)
										{
											if (syntax_idcmp(mk1, right) == 0)
											{
												itable_t *it2;
												it2 = table_rpush(frame, m1);
												if (it2 == NULL)
												{
													fprintf(stderr, "unable to allocate memory\n");
													return -1;
												}
												it2->flag |= ITABLE_FLAG_TEMPORARY;

												ilist_t *r2;
												r2 = list_rpush(response, it2);
												if (r2 == NULL)
												{
													fprintf(stderr, "unable to allocate memory\n");
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
					if (symbol_check_type(s1, SYMBOL_OBJECT))
					{
						symbol_t *right;
						right = syntax_extract_with(target, SYMBOL_RIGHT);
						if (right)
						{
							symbol_t *m1;
							for (m1 = s1->begin;m1 != s1->end;m1 = m1->next)
							{
								if (symbol_check_type(m1, SYMBOL_MEMBER))
								{
									symbol_t *mk1;
									mk1 = syntax_extract_with(m1, SYMBOL_KEY);
									if (mk1)
									{
										if (syntax_idcmp(mk1, right) == 0)
										{
											itable_t *it2;
											it2 = table_rpush(frame, m1);
											if (it2 == NULL)
											{
												fprintf(stderr, "unable to allocate memory\n");
												return -1;
											}
											it2->flag |= ITABLE_FLAG_TEMPORARY;

											ilist_t *r2;
											r2 = list_rpush(response, it2);
											if (r2 == NULL)
											{
												fprintf(stderr, "unable to allocate memory\n");
												return -1;
											}
										}
									}
								}
							}
						}
					}
					else
					if (symbol_check_type(s1, SYMBOL_VAR))
					{
						symbol_t *s2 = it1->refer;
						if (s2 != NULL)
						{
							symbol_t *right;
							right = syntax_extract_with(target, SYMBOL_RIGHT);
							if (right)
							{
								if (symbol_check_type(s2, SYMBOL_CLASS))
								{
									symbol_t *right;
									right = syntax_extract_with(target, SYMBOL_RIGHT);
									if (right)
									{
										list_t response2;
					
										list_t *r2;
										r2 = list_apply(&response2);
										if (r2 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r3;
										r3 = syntax_hierarchy(program, scope, frame, s2, right, &response2, flag);
										if (r3 == -1)
										{
											return -1;
										}
										else
										if (r3 == 1)
										{
											it1->reference += 1;
										}

										ilist_t *a2;
										for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
										{
											itable_t *it2 = (itable_t *)a2->value;
											if (it2 != NULL)
											{
												ilist_t *r4;
												r4 = list_rpush(response, it2);
												if (r4 == NULL)
												{
													fprintf(stderr, "unable to allocate memory\n");
													return -1;
												}
											}
										}
										list_clear(&response2);
									}
								}
								else
								if (symbol_check_type(s2, SYMBOL_ENUM))
								{
									symbol_t *right;
									right = syntax_extract_with(target, SYMBOL_RIGHT);
									if (right)
									{
										symbol_t *ms1;
										ms1 = syntax_extract_with(s2, SYMBOL_MEMBERS);
										if (ms1 != NULL)
										{
											symbol_t *m1;
											for (m1 = ms1->begin;m1 != ms1->end;m1 = m1->next)
											{
												if (symbol_check_type(m1, SYMBOL_MEMBER))
												{
													symbol_t *mk1;
													mk1 = syntax_extract_with(m1, SYMBOL_KEY);
													if (mk1)
													{
														if (syntax_idcmp(mk1, right) == 0)
														{
															itable_t *it2;
															it2 = table_rpush(frame, m1);
															if (it2 == NULL)
															{
																fprintf(stderr, "unable to allocate memory\n");
																return -1;
															}
															it2->flag |= ITABLE_FLAG_TEMPORARY;

															ilist_t *r2;
															r2 = list_rpush(response, it2);
															if (r2 == NULL)
															{
																fprintf(stderr, "unable to allocate memory\n");
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
								if (symbol_check_type(s2, SYMBOL_OBJECT))
								{
									symbol_t *right;
									right = syntax_extract_with(target, SYMBOL_RIGHT);
									if (right)
									{
										symbol_t *m1;
										for (m1 = s2->begin;m1 != s2->end;m1 = m1->next)
										{
											if (symbol_check_type(m1, SYMBOL_MEMBER))
											{
												symbol_t *mk1;
												mk1 = syntax_extract_with(m1, SYMBOL_KEY);
												if (mk1)
												{
													if (syntax_idcmp(mk1, right) == 0)
													{
														itable_t *it2;
														it2 = table_rpush(frame, m1);
														if (it2 == NULL)
														{
															fprintf(stderr, "unable to allocate memory\n");
															return -1;
														}
														it2->flag |= ITABLE_FLAG_TEMPORARY;

														ilist_t *r2;
														r2 = list_rpush(response, it2);
														if (r2 == NULL)
														{
															fprintf(stderr, "unable to allocate memory\n");
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
							syntax_error(program, s1, "not initialized");
							return -1;
						}
					}
					else
					{
						syntax_error(program, s1, "not a route");
						return -1;
					}
				}
			}
		}
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_composite(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *key;
	key = syntax_extract_with(target, SYMBOL_KEY);
	if (key)
	{
		list_t response1;

		list_t *r0;
		r0 = list_apply(&response1);
		if (r0 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, frame, key, &response1, flag);
		if (r1 == -1)
		{
			return -1;
		}
		else
		if (r1 == 0)
		{
			syntax_error(program, key, "reference not found");
			return -1;
		}
		else
		{
			ilist_t *a1;
			for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
			{
				itable_t *it1 = (itable_t *)a1->value;
				if (it1->original != NULL)
				{
					symbol_t *s1 = it1->original;
					if (symbol_check_type(s1, SYMBOL_CLASS))
					{
						symbol_t *gs1;
						gs1 = syntax_only_with(s1, SYMBOL_GENERICS);
						if (gs1 != NULL)
						{
							symbol_t *arguments;
							arguments = syntax_extract_with(target, SYMBOL_ARGUMENTS);
							if (arguments)
							{
								int32_t r2;
								r2 = syntax_match_gsast(program, gs1, arguments);
								if (r2 == -1)
								{
									return -1;
								}
								else
								if (r2 == 1)
								{
									itable_t *it2;
									it2 = table_rpush(frame, s1);
									if (it2 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
										return -1;
									}
									it2->flag |= ITABLE_FLAG_TEMPORARY;

									ilist_t *r3;
									r3 = list_rpush(response, it2);
									if (r3 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
										return -1;
									}
								}
							}
						}
					}
					else
					if (symbol_check_type(s1, SYMBOL_FUNCTION))
					{
						symbol_t *gs;
						gs = syntax_only_with(s1, SYMBOL_GENERICS);
						if (gs)
						{
							symbol_t *arguments;
							arguments = syntax_extract_with(target, SYMBOL_ARGUMENTS);
							if (arguments)
							{
								int32_t r2;
								r2 = syntax_match_gsast(program, gs, arguments);
								if (r2 == 1)
								{
									itable_t *it2;
									it2 = table_rpush(frame, s1);
									if (it2 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
										return -1;
									}
									it2->flag |= ITABLE_FLAG_TEMPORARY;

									ilist_t *r3;
									r3 = list_rpush(response, it2);
									if (r3 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
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

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_call(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *key;
	key = syntax_extract_with(target, SYMBOL_KEY);
	if (key)
	{
		list_t response1;

		list_t *r0;
		r0 = list_apply(&response1);
		if (r0 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, frame, key, &response1, flag);
		if (r1 == -1)
		{
			return -1;
		}
		else
		if (r1 == 0)
		{
			syntax_error(program, key, "reference not found");
			return -1;
		}
		else
		{
			ilist_t *a1;
			for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
			{
				itable_t *it1 = (itable_t *)a1->value;
				if (it1->original != NULL)
				{
					symbol_t *s1 = it1->original;
					if (symbol_check_type(s1, SYMBOL_CLASS))
					{
						symbol_t *b1;
						for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
						{
							if (symbol_check_type(b1, SYMBOL_FUNCTION))
							{
								symbol_t *bk1;
								bk1 = syntax_extract_with(b1, SYMBOL_KEY);
								if (bk1 != NULL)
								{
									if (syntax_idstrcmp(bk1, "constructor") == 0)
									{
										symbol_t *ps1;
										ps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
										if (ps1 != NULL)
										{
											symbol_t *arguments;
											arguments = syntax_extract_with(target, SYMBOL_ARGUMENTS);
											if (arguments)
											{
												int32_t r2;
												r2 = syntax_match_psasv(program, ps1, arguments);
												if (r2 == 1)
												{
													itable_t *it2;
													it2 = table_rpush(frame, s1);
													if (it2 == NULL)
													{
														fprintf(stderr, "unable to allocate memory\n");
														return -1;
													}
													it2->flag |= ITABLE_FLAG_TEMPORARY;

													ilist_t *r3;
													r3 = list_rpush(response, it2);
													if (r3 == NULL)
													{
														fprintf(stderr, "unable to allocate memory\n");
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
					if (symbol_check_type(s1, SYMBOL_FUNCTION))
					{
						symbol_t *ps1;
						ps1 = syntax_only_with(s1, SYMBOL_PARAMETERS);
						if (ps1 != NULL)
						{
							symbol_t *arguments;
							arguments = syntax_extract_with(target, SYMBOL_ARGUMENTS);
							if (arguments)
							{
								int32_t r2;
								r2 = syntax_match_psasv(program, ps1, arguments);
								if (r2 == 1)
								{
									itable_t *it2;
									it2 = table_rpush(frame, s1);
									if (it2 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
										return -1;
									}
									it2->flag |= ITABLE_FLAG_TEMPORARY;

									ilist_t *r3;
									r3 = list_rpush(response, it2);
									if (r3 == NULL)
									{
										fprintf(stderr, "unable to allocate memory\n");
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

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_postfix(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_ATTR))
	{
		return syntax_attribute(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_COMPOSITE))
	{
		return syntax_composite(program, scope, frame, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_CALL))
	{
		return syntax_call(program, scope, frame, target, response, flag);
	}
	else
	{
		return syntax_primary(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_prefix(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_TILDE))
	{
		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right)
		{
			return syntax_prefix(program, scope, frame, right, response, flag);
		}
		return 0;
	}
	else
	if (symbol_check_type(target, SYMBOL_NOT))
	{
		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right)
		{
			return syntax_prefix(program, scope, frame, right, response, flag);
		}
		return 0;
	}
	else
	if (symbol_check_type(target, SYMBOL_NEG))
	{
		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right)
		{
			return syntax_prefix(program, scope, frame, right, response, flag);
		}
		return 0;
	}
	else
	if (symbol_check_type(target, SYMBOL_POS))
	{
		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right)
		{
			return syntax_prefix(program, scope, frame, right, response, flag);
		}
		return 0;
	}
	else
	if (symbol_check_type(target, SYMBOL_TYPEOF))
	{
		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right)
		{
			return syntax_prefix(program, scope, frame, right, response, flag);
		}
		return 0;
	}
	else
	if (symbol_check_type(target, SYMBOL_SIZEOF))
	{
		return syntax_number(program, scope, frame, target, response, flag);
	}
	else
	{
		return syntax_postfix(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_power(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_POW))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "**") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_prefix(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_multiplicative(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_MUL))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "*") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_DIV))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "/") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_MOD))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "%") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_EPI))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "\\") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_prefix(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_addative(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_PLUS))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "+") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_MINUS))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "-") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_multiplicative(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_shifting(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_SHL))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "<<") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_SHR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, ">>") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_addative(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_relational(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_LT))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "<") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_GT))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, ">") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_LE))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "<=") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_GE))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, ">=") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_shifting(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_equality(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_EQ))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "==") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_NEQ))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "!=") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_relational(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_bitwise_and(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_AND))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "&") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_equality(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_bitwise_xor(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_XOR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "^") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_bitwise_and(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_bitwise_or(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_OR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "|") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_bitwise_xor(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_logical_and(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_LAND))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "&&") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_bitwise_or(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_logical_or(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_LOR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;

					list_t *r2;
					r2 = list_apply(&response2);
					if (r2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							itable_t *it1 = (itable_t *)a1->value;
							if (it1->original != NULL)
							{
								symbol_t *s1 = it1->original;
								if (symbol_check_type(s1, SYMBOL_CLASS))
								{
									symbol_t *b1;
									for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "||") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r4;
																	r4 = syntax_match_pst(program, bps1, s2);
																	if (r4 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r4 == 1)
																	{
																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, response, flag);
																		if (r5 == -1)
																		{
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
					}
				}
			}
		}
	}
	else
	{
		return syntax_logical_and(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_conditional(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_CONDITIONAL))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, response, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, response, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
				}
			}
		}
	}
	else
	{
		return syntax_logical_or(program, scope, frame, target, response, flag);
	}
}

static int32_t
syntax_expression(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	return syntax_conditional(program, scope, frame, target, response, flag);
}

static int32_t
syntax_assign(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;

			list_t *r0;
			r0 = list_apply(&response1);
			if (r0 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag | SYNTAX_FLAG_INIT);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				ilist_t *a1;
				for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
				{
					itable_t *it1 = (itable_t *)a1->value;
					if (it1->original != NULL)
					{
						symbol_t *s1 = it1->original;
						if (symbol_check_type(s1, SYMBOL_CLASS))
						{
							symbol_t *b1;
							for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
							{
								if (symbol_check_type(b1, SYMBOL_FUNCTION))
								{
									symbol_t *bk1;
									bk1 = syntax_extract_with(b1, SYMBOL_KEY);
									if (bk1)
									{
										if (syntax_idstrcmp(bk1, "=") == 0)
										{
											symbol_t *bps1;
											bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
											if (bps1)
											{
												symbol_t *right;
												right = syntax_extract_with(target, SYMBOL_RIGHT);
												if (right)
												{
													list_t response2;

													list_t *r2;
													r2 = list_apply(&response2);
													if (r2 == NULL)
													{
														fprintf(stderr, "unable to allocate memory\n");
														return -1;
													}

													int32_t r3;
													r3 = syntax_expression(program, scope, frame, right, &response2, flag);
													if (r3 == -1)
													{
														return -1;
													}
													else
													if (r3 == 0)
													{
														syntax_error(program, right, "reference not found");
														return -1;
													}
													else
													{
														ilist_t *a2;
														for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
														{
															itable_t *it2 = (itable_t *)a2->value;
															if (it2->original != NULL)
															{
																symbol_t *s2 = it2->original;
																if (symbol_check_type(s2, SYMBOL_CLASS))
																{
																	int32_t r3;
																	r3 = syntax_match_pst(program, bps1, s2);
																	if (r3 == -1)
																	{
																		return -1;
																	}
																	else
																	if (r3 == 1)
																	{
																		list_t response3;

																		list_t *r4;
																		r4 = list_apply(&response3);
																		if (r4 == NULL)
																		{
																			fprintf(stderr, "unable to allocate memory\n");
																			return -1;
																		}

																		int32_t r5;
																		r5 = syntax_function(program, scope, frame, b1, &response3, flag);
																		if (r5 == -1)
																		{
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
												syntax_error(program, b1, "function must have parameters");
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
	else
	if (symbol_check_type(target, SYMBOL_ADD_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "+") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_SUB_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "-") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_MUL_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "*") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_DIV_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "/") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_MOD_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "%") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_AND_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "&") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_OR_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "|") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_SHL_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, "<<") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	if (symbol_check_type(target, SYMBOL_SHR_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left)
		{
			list_t response1;
			int32_t r1;
			r1 = syntax_expression(program, scope, frame, left, &response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, left, "reference not found");
				return -1;
			}
			else
			{
				symbol_t *right;
				right = syntax_extract_with(target, SYMBOL_RIGHT);
				if (right)
				{
					list_t response2;
					int32_t r2;
					r2 = syntax_expression(program, scope, frame, right, &response2, flag);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 0)
					{
						syntax_error(program, right, "reference not found");
						return -1;
					}
					else
					{
						ilist_t *a1;
						for (a1 = response1.begin;a1 != response1.end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (symbol_check_type(s1, SYMBOL_CLASS))
							{
								symbol_t *b1;
								for (b1 = s1->begin;b1 != s1->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1)
										{
											if (syntax_idstrcmp(bk1, ">>") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1)
												{
													ilist_t *a2;
													for (a2 = response2.begin;a2 != response2.end;a2 = a2->next)
													{
														symbol_t *s2 = (symbol_t *)a2->value;
														if (symbol_check_type(s2, SYMBOL_CLASS))
														{
															int32_t r3;
															r3 = syntax_match_pst(program, bps1, s2);
															if (r3 == -1)
															{
																return -1;
															}
															else
															if (r3 == 1)
															{
																list_t response3;
																int32_t r4;
																r4 = syntax_function(program, scope, frame, b1, &response3, flag);
																if (r4 == -1)
																{
																	return -1;
																}
																if (r4 == 0)
																{
																	syntax_error(program, b1, "no return");
																	return -1;
																}
																else
																{
																	ilist_t *a3;
																	for (a3 = response3.begin;a3 != response3.end;a3 = a3->next)
																	{
																		symbol_t *s3 = (symbol_t *)a3->value;
																		if (symbol_check_type(s3, SYMBOL_CLASS))
																		{
																			symbol_t *b2;
																			for (b2 = s3->begin;b2 != s3->end;b2 = b2->next)
																			{
																				if (symbol_check_type(b2, SYMBOL_FUNCTION))
																				{
																					symbol_t *bk2;
																					bk2 = syntax_extract_with(b2, SYMBOL_KEY);
																					if (bk2)
																					{
																						if (syntax_idstrcmp(bk2, "=") == 0)
																						{
																							symbol_t *bps2;
																							bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																							if (bps2)
																							{
																								ilist_t *a4;
																								for (a4 = response2.begin;a4 != response2.end;a4 = a4->next)
																								{
																									symbol_t *s4 = (symbol_t *)a4->value;
																									if (symbol_check_type(s4, SYMBOL_CLASS))
																									{
																										int32_t r4;
																										r4 = syntax_match_pst(program, bps2, s4);
																										if (r4 == -1)
																										{
																											return -1;
																										}
																										else
																										if (r4 == 1)
																										{
																											int32_t r5;
																											r5 = syntax_function(program, scope, frame, b2, response, flag);
																											if (r5 == -1)
																											{
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
	}
	else
	{
		return syntax_expression(program, scope, frame, target, response, flag);
	}
}





static int32_t
syntax_return(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	return 1;
}

static int32_t
syntax_continue(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	/*
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}
	*/

	return 1;
}

static int32_t
syntax_break(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	/*
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}
	*/
	return 1;
}

static int32_t
syntax_throw(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	/*
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(program, a);
		if (result == -1)
		{
			return -1;
		}
	}
	*/

	return 1;
}

static int32_t
syntax_var(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "variable without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "variable without key");
		return -1;
	}

	itable_t *it1;
	it1 = table_rpush(frame, target);
	if (it1 == NULL)
	{
		fprintf(stderr, "unable to allocate memory\n");
		return -1;
	}

	return 1;
}

static int32_t
syntax_if(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "if without parent");
			return -1;
		}
	}


	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
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
					result = syntax_if(program, scope, frame, b, response, flag);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = syntax_block(program, scope, frame, b, response, flag);
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
syntax_catch(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
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
syntax_try(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
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
					result = syntax_catch(program, scope, frame, b, response, flag);
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
syntax_for(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "if without parent");
			return -1;
		}
	}

	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					int32_t result;
					result = syntax_var(program, scope, frame, b, response, flag);
					if (result == -1)
					{
						return -1;
					}
					continue;
				}
				if (symbol_check_type(a, SYMBOL_ASSIGN))
				{
					/*
					int32_t result;
					result = syntax_assign(program, scope, frame, b, response, flag);
					if (result == -1)
					{
						return -1;
					}
					*/
					continue;
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			/*
			int32_t result;
			result = syntax_expression(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			*/
			continue;
		}
		if (symbol_check_type(a, SYMBOL_INCREMENTOR))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_ASSIGN))
				{
					/*
					int32_t result;
					result = syntax_assign(program, scope, frame, b, response, flag);
					if (result == -1)
					{
						return -1;
					}
					*/
					continue;
				}
			}
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
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
syntax_forin(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "for without parent");
			return -1;
		}
	}

	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					int32_t result;
					result = syntax_var(program, scope, frame, b, response, flag);
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
			/*
			int32_t result;
			result = syntax_expression(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			*/
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
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
syntax_statement(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	int32_t result = 1;

	if (symbol_check_type(target, SYMBOL_BLOCK))
	{
		result = syntax_block(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_IF))
	{
		result = syntax_if(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_TRY))
	{
		result = syntax_try(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_FOR))
	{
		result = syntax_for(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_FORIN))
	{
		result = syntax_forin(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_FUNCTION))
	{
		result = syntax_function(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_CONTINUE))
	{
		result = syntax_continue(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_BREAK))
	{
		result = syntax_break(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_RETURN))
	{
		result = syntax_return(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_THROW))
	{
		result = syntax_throw(program, scope, frame, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_VAR))
	{
		result = syntax_var(program, scope, frame, target, response, flag);
	}
	else
	{
		/*
		result = syntax_assign(program, scope, frame, target, response, flag);
		*/
	}

	return result;
}

static int32_t
syntax_block(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end);a = a->next)
	{
		int32_t result;
		result = syntax_statement(program, scope, frame, a, response, flag);
		if (result == -1)
		{
			return -1;
		}
	}
	return 1;
}



static int32_t
syntax_generic(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "generic without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "generic without key");
		return -1;
	}
	return 1;
}

static int32_t
syntax_generics(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_generic(program, scope, frame, a, response, flag);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_parameter(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "parameter without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "parameter without key");
		return -1;
	}
	return 1;
}

static int32_t
syntax_parameters(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_parameter(program, scope, frame, a, response, flag);
		if (result == -1)
		{
			return -1;
		}

		symbol_t *ak;
		ak = syntax_extract_with(a, SYMBOL_KEY);
		if (ak)
		{
			symbol_t *root = target->parent;
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
syntax_heritage(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "heritage without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "heritage without key");
		return -1;
	}

	symbol_t *ct;
	ct = syntax_extract_with(target, SYMBOL_TYPE);
	if (ct)
	{

	}

	return 1;
}

static int32_t
syntax_heritages(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_heritage(program, scope, frame, a, response, flag);
		if (result == -1)
		{
			return -1;
		}

		symbol_t *ak;
		ak = syntax_extract_with(a, SYMBOL_KEY);
		if (ak)
		{
			symbol_t *root = target->parent;
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
syntax_member(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "emum member without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "emum member without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_members(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_member(program, scope, frame, a, response, flag);
		if (result == -1)
		{
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_enum(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "enum without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "enum without key");
		return -1;
	}

	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_MEMBERS))
		{
			int32_t result;
			result = syntax_members(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
syntax_property(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "property without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "property without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_function(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		int32_t result;
		result = syntax_idstrcmp(ck, "constructor");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "constructor with generic");
				return -1;
			}
		}

		result = syntax_idstrcmp(ck, "+");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator + with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator + with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "-");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator - with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator + with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "*");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator * with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator * with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "/");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator / with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator / with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "**");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator ** with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator ** with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "%");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator %% with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator %% with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "&");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator & with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator & with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "|");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator | with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator | with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "^");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator ^ with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator ^ with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, ">>");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator >> with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator >> with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "<<");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator << with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator << with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, ">");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator > with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator > with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "<");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator < with generics");
				return -1;
			}

			symbol_t *ps;
			ps = syntax_only_with(target, SYMBOL_PARAMETERS);
			if (ps)
			{
				if (symbol_count(ps) > 1)
				{
					syntax_error(program, target, "operator < with multiple parameters");
					return -1;
				}
			}
		}

		result = syntax_idstrcmp(ck, "[]");
		if (result == 0)
		{
			symbol_t *gs;
			gs = syntax_only_with(target, SYMBOL_GENERICS);
			if (gs)
			{
				syntax_error(program, target, "operator [] with generics");
				return -1;
			}
		}

		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
														bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
														bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
														bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
													bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
										bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
										bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
										bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
									bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
									bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
								bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
									bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
								bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
									bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
								bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
									cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
											bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
										bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
									cgs = syntax_only_with(target, SYMBOL_GENERICS);
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
											bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
										bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
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
			syntax_error(program, target, "function without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "function without key");
		return -1;
	}

	itable_t *it1;
	it1 = table_rpush(frame, target);
	if (it1 == NULL)
	{
		fprintf(stderr, "unable to allocate memory\n");
		return -1;
	}

	table_t *frame2;
	frame2 = table_create();
	if (frame2 == NULL)
	{
		fprintf(stderr, "unable to allocate memory\n");
		return -1;
	}
	frame2->parent = frame;

	symbol_t *a;
	for (a = target->begin;a != target->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			int32_t result;
			result = syntax_generics(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	table_destroy(frame2);
	return 1;
}

static int32_t
syntax_class(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	itable_t *it1;
	for (it1 = frame->begin;(it1 != frame->end);it1 = it1->next)
	{
		if ((it1->original != NULL) && ((it1->flag & ITABLE_FLAG_TEMPORARY) != ITABLE_FLAG_TEMPORARY))
		{
			symbol_t *o1 = it1->original;
			if (o1 != NULL)
			{
				symbol_t *ok1;
				ok1 = syntax_extract_with(it1->original, SYMBOL_KEY);
				if (ok1 != NULL)
				{
					symbol_t *tk1;
					tk1 = syntax_extract_with(target, SYMBOL_KEY);
					if (tk1 != NULL)
					{
						if (syntax_idcmp(ok1, tk1) == 0)
						{
							if (symbol_check_type(o1, SYMBOL_CLASS))
							{
								symbol_t *ogs1;
								ogs1 = syntax_only_with(o1, SYMBOL_GENERICS);
								if (ogs1 != NULL)
								{
									symbol_t *tgs1;
									tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
									if (tgs1 != NULL)
									{
										int32_t r1;
										r1 = syntax_equal_gsgs(program, tgs1, ogs1);
										if (r1 == -1)
										{
											return -1;
										}
										else
										if (r1 == 1)
										{
											symbol_t *ob1;
											for (ob1 = o1->begin;ob1 != o1->end;ob1 = ob1->next)
											{
												if (symbol_check_type(ob1, SYMBOL_FUNCTION))
												{
													symbol_t *obk1;
													obk1 = syntax_extract_with(ob1, SYMBOL_KEY);
													if (obk1 != NULL)
													{
														if (syntax_idstrcmp(obk1, "constructor") == 0)
														{
															symbol_t *tb1;
															for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
															{
																if (symbol_check_type(tb1, SYMBOL_FUNCTION))
																{
																	symbol_t *tbk1;
																	tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
																	if (tbk1 != NULL)
																	{
																		if (syntax_idstrcmp(tbk1, "constructor") == 0)
																		{
																			symbol_t *obps1;
																			obps1 = syntax_only_with(ob1, SYMBOL_PARAMETERS);
																			if (obps1 != NULL)
																			{
																				symbol_t *tbps1;
																				tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																				if (tbps1 != NULL)
																				{
																					int32_t r2;
																					r2 = syntax_equal_psps(program, obps1, tbps1);
																					if (r2 == -1)
																					{
																						return -1;
																					}
																					else 
																					if (r2 == 1)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}
																					else
																					{
																						itable_t *it2;
																						it2 = table_rpush(frame, target);
																						if (it2 == NULL)
																						{
																							fprintf(stderr, "unable to allocate memory\n");
																							return -1;
																						}
																					}
																				}
																				else
																				{
																					int32_t pwv = 0;
																					symbol_t *obp1;
																					for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																					{
																						symbol_t *obpv1;
																						obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																						if (obpv1 == NULL)
																						{
																							pwv = 1;
																							break;
																						}
																					}

																					if (pwv == 0)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}

																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																			}
																			else
																			{
																				symbol_t *tbps1;
																				tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																				if (tbps1 != NULL)
																				{
																					int32_t pwv = 0;
																					symbol_t *tbp1;
																					for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																					{
																						symbol_t *tbpv1;
																						tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																						if (tbpv1 == NULL)
																						{
																							pwv = 1;
																							break;
																						}
																					}

																					if (pwv == 0)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}

																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																				else
																				{
																					syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																						ok1->declaration->position.line, ok1->declaration->position.column);
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
										else
										{
											itable_t *it2;
											it2 = table_rpush(frame, target);
											if (it2 == NULL)
											{
												fprintf(stderr, "unable to allocate memory\n");
												return -1;
											}
										}
									}
									else
									{
										int32_t gwv = 0;
										symbol_t *og1;
										for (og1 = ogs1->begin;og1 != ogs1->end;og1 = og1->next)
										{
											if (symbol_check_type(og1, SYMBOL_GENERIC))
											{
												symbol_t *ogv1;
												ogv1 = syntax_only_with(og1, SYMBOL_VALUE);
												if (ogv1 != NULL)
												{
													gwv = 1;
													break;
												}
											}
										}
										if (gwv == 0)
										{
											symbol_t *ob1;
											for (ob1 = o1->begin;ob1 != o1->end;ob1 = ob1->next)
											{
												if (symbol_check_type(ob1, SYMBOL_FUNCTION))
												{
													symbol_t *obk1;
													obk1 = syntax_extract_with(ob1, SYMBOL_KEY);
													if (obk1 != NULL)
													{
														if (syntax_idstrcmp(obk1, "constructor") == 0)
														{
															symbol_t *tb1;
															for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
															{
																if (symbol_check_type(tb1, SYMBOL_FUNCTION))
																{
																	symbol_t *tbk1;
																	tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
																	if (tbk1 != NULL)
																	{
																		if (syntax_idstrcmp(tbk1, "constructor") == 0)
																		{
																			symbol_t *obps1;
																			obps1 = syntax_only_with(ob1, SYMBOL_PARAMETERS);
																			if (obps1 != NULL)
																			{
																				symbol_t *tbps1;
																				tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																				if (tbps1 != NULL)
																				{
																					int32_t r2;
																					r2 = syntax_equal_psps(program, obps1, tbps1);
																					if (r2 == -1)
																					{
																						return -1;
																					}
																					else 
																					if (r2 == 1)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}
																					else
																					{
																						itable_t *it2;
																						it2 = table_rpush(frame, target);
																						if (it2 == NULL)
																						{
																							fprintf(stderr, "unable to allocate memory\n");
																							return -1;
																						}
																					}
																				}
																				else
																				{
																					int32_t pwv = 0;
																					symbol_t *obp1;
																					for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																					{
																						symbol_t *obpv1;
																						obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																						if (obpv1 == NULL)
																						{
																							pwv = 1;
																							break;
																						}
																					}

																					if (pwv == 0)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}

																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																			}
																			else
																			{
																				symbol_t *tbps1;
																				tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																				if (tbps1 != NULL)
																				{
																					int32_t pwv = 0;
																					symbol_t *tbp1;
																					for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																					{
																						symbol_t *tbpv1;
																						tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																						if (tbpv1 == NULL)
																						{
																							pwv = 1;
																							break;
																						}
																					}

																					if (pwv == 0)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}

																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																				else
																				{
																					syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																						ok1->declaration->position.line, ok1->declaration->position.column);
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
									symbol_t *tgs1;
									tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
									if (tgs1 != NULL)
									{
										int32_t gwv = 0;
										symbol_t *tg1;
										for (tg1 = tgs1->begin;tg1 != tgs1->end;tg1 = tg1->next)
										{
											if (symbol_check_type(tg1, SYMBOL_GENERIC))
											{
												symbol_t *tgv1;
												tgv1 = syntax_only_with(tg1, SYMBOL_VALUE);
												if (tgv1 != NULL)
												{
													gwv = 1;
													break;
												}
											}
										}
										if (gwv == 0)
										{
											symbol_t *ob1;
											for (ob1 = o1->begin;ob1 != o1->end;ob1 = ob1->next)
											{
												if (symbol_check_type(ob1, SYMBOL_FUNCTION))
												{
													symbol_t *obk1;
													obk1 = syntax_extract_with(ob1, SYMBOL_KEY);
													if (obk1 != NULL)
													{
														if (syntax_idstrcmp(obk1, "constructor") == 0)
														{
															symbol_t *tb1;
															for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
															{
																if (symbol_check_type(tb1, SYMBOL_FUNCTION))
																{
																	symbol_t *tbk1;
																	tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
																	if (tbk1 != NULL)
																	{
																		if (syntax_idstrcmp(tbk1, "constructor") == 0)
																		{
																			symbol_t *obps1;
																			obps1 = syntax_only_with(ob1, SYMBOL_PARAMETERS);
																			if (obps1 != NULL)
																			{
																				symbol_t *tbps1;
																				tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																				if (tbps1 != NULL)
																				{
																					int32_t r2;
																					r2 = syntax_equal_psps(program, obps1, tbps1);
																					if (r2 == -1)
																					{
																						return -1;
																					}
																					else 
																					if (r2 == 1)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}
																					else
																					{
																						itable_t *it2;
																						it2 = table_rpush(frame, target);
																						if (it2 == NULL)
																						{
																							fprintf(stderr, "unable to allocate memory\n");
																							return -1;
																						}
																					}
																				}
																				else
																				{
																					int32_t pwv = 0;
																					symbol_t *obp1;
																					for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																					{
																						symbol_t *obpv1;
																						obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																						if (obpv1 == NULL)
																						{
																							pwv = 1;
																							break;
																						}
																					}

																					if (pwv == 0)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}

																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																			}
																			else
																			{
																				symbol_t *tbps1;
																				tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																				if (tbps1 != NULL)
																				{
																					int32_t pwv = 0;
																					symbol_t *tbp1;
																					for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																					{
																						symbol_t *tbpv1;
																						tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																						if (tbpv1 == NULL)
																						{
																							pwv = 1;
																							break;
																						}
																					}

																					if (pwv == 0)
																					{
																						syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																							ok1->declaration->position.line, ok1->declaration->position.column);
																						return -1;
																					}

																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																				else
																				{
																					syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																						ok1->declaration->position.line, ok1->declaration->position.column);
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
										symbol_t *ob1;
										for (ob1 = o1->begin;ob1 != o1->end;ob1 = ob1->next)
										{
											if (symbol_check_type(ob1, SYMBOL_FUNCTION))
											{
												symbol_t *obk1;
												obk1 = syntax_extract_with(ob1, SYMBOL_KEY);
												if (obk1 != NULL)
												{
													if (syntax_idstrcmp(obk1, "constructor") == 0)
													{
														symbol_t *tb1;
														for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
														{
															if (symbol_check_type(tb1, SYMBOL_FUNCTION))
															{
																symbol_t *tbk1;
																tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
																if (tbk1 != NULL)
																{
																	if (syntax_idstrcmp(tbk1, "constructor") == 0)
																	{
																		symbol_t *obps1;
																		obps1 = syntax_only_with(ob1, SYMBOL_PARAMETERS);
																		if (obps1 != NULL)
																		{
																			symbol_t *tbps1;
																			tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																			if (tbps1 != NULL)
																			{
																				int32_t r2;
																				r2 = syntax_equal_psps(program, obps1, tbps1);
																				if (r2 == -1)
																				{
																					return -1;
																				}
																				else 
																				if (r2 == 1)
																				{
																					syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																						ok1->declaration->position.line, ok1->declaration->position.column);
																					return -1;
																				}
																				else
																				{
																					itable_t *it2;
																					it2 = table_rpush(frame, target);
																					if (it2 == NULL)
																					{
																						fprintf(stderr, "unable to allocate memory\n");
																						return -1;
																					}
																				}
																			}
																			else
																			{
																				int32_t pwv = 0;
																				symbol_t *obp1;
																				for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																				{
																					symbol_t *obpv1;
																					obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																					if (obpv1 == NULL)
																					{
																						pwv = 1;
																						break;
																					}
																				}

																				if (pwv == 0)
																				{
																					syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																						ok1->declaration->position.line, ok1->declaration->position.column);
																					return -1;
																				}

																				itable_t *it2;
																				it2 = table_rpush(frame, target);
																				if (it2 == NULL)
																				{
																					fprintf(stderr, "unable to allocate memory\n");
																					return -1;
																				}
																			}
																		}
																		else
																		{
																			symbol_t *tbps1;
																			tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																			if (tbps1 != NULL)
																			{
																				int32_t pwv = 0;
																				symbol_t *tbp1;
																				for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																				{
																					symbol_t *tbpv1;
																					tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																					if (tbpv1 == NULL)
																					{
																						pwv = 1;
																						break;
																					}
																				}

																				if (pwv == 0)
																				{
																					syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																						ok1->declaration->position.line, ok1->declaration->position.column);
																					return -1;
																				}

																				itable_t *it2;
																				it2 = table_rpush(frame, target);
																				if (it2 == NULL)
																				{
																					fprintf(stderr, "unable to allocate memory\n");
																					return -1;
																				}
																			}
																			else
																			{
																				syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																					ok1->declaration->position.line, ok1->declaration->position.column);
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
							if (symbol_check_type(o1, SYMBOL_FUNCTION))
							{
								symbol_t *ogs1;
								ogs1 = syntax_only_with(o1, SYMBOL_GENERICS);
								if (ogs1 != NULL)
								{
									symbol_t *tgs1;
									tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
									if (tgs1 != NULL)
									{
										int32_t r1;
										r1 = syntax_equal_gsgs(program, tgs1, ogs1);
										if (r1 == -1)
										{
											return -1;
										}
										else
										if (r1 == 1)
										{
											symbol_t *tb1;
											for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
											{
												if (symbol_check_type(tb1, SYMBOL_FUNCTION))
												{
													symbol_t *tbk1;
													tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
													if (tbk1 != NULL)
													{
														if (syntax_idstrcmp(tbk1, "constructor") == 0)
														{
															symbol_t *obps1;
															obps1 = syntax_only_with(o1, SYMBOL_PARAMETERS);
															if (obps1 != NULL)
															{
																symbol_t *tbps1;
																tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																if (tbps1 != NULL)
																{
																	int32_t r2;
																	r2 = syntax_equal_psps(program, obps1, tbps1);
																	if (r2 == -1)
																	{
																		return -1;
																	}
																	else 
																	if (r2 == 1)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}
																	else
																	{
																		itable_t *it2;
																		it2 = table_rpush(frame, target);
																		if (it2 == NULL)
																		{
																			fprintf(stderr, "unable to allocate memory\n");
																			return -1;
																		}
																	}
																}
																else
																{
																	int32_t pwv = 0;
																	symbol_t *obp1;
																	for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																	{
																		symbol_t *obpv1;
																		obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																		if (obpv1 == NULL)
																		{
																			pwv = 1;
																			break;
																		}
																	}

																	if (pwv == 0)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}

																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
															}
															else
															{
																symbol_t *tbps1;
																tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																if (tbps1 != NULL)
																{
																	int32_t pwv = 0;
																	symbol_t *tbp1;
																	for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																	{
																		symbol_t *tbpv1;
																		tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																		if (tbpv1 == NULL)
																		{
																			pwv = 1;
																			break;
																		}
																	}

																	if (pwv == 0)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}

																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
																else
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
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
											itable_t *it2;
											it2 = table_rpush(frame, target);
											if (it2 == NULL)
											{
												fprintf(stderr, "unable to allocate memory\n");
												return -1;
											}
										}
									}
									else
									{
										int32_t gwv = 0;
										symbol_t *og1;
										for (og1 = ogs1->begin;og1 != ogs1->end;og1 = og1->next)
										{
											if (symbol_check_type(og1, SYMBOL_GENERIC))
											{
												symbol_t *ogv1;
												ogv1 = syntax_only_with(og1, SYMBOL_VALUE);
												if (ogv1 != NULL)
												{
													gwv = 1;
													break;
												}
											}
										}
										if (gwv == 0)
										{
											symbol_t *tb1;
											for (tb1 = o1->begin;tb1 != o1->end;tb1 = tb1->next)
											{
												if (symbol_check_type(tb1, SYMBOL_FUNCTION))
												{
													symbol_t *tbk1;
													tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
													if (tbk1 != NULL)
													{
														if (syntax_idstrcmp(tbk1, "constructor") == 0)
														{
															symbol_t *obps1;
															obps1 = syntax_only_with(o1, SYMBOL_PARAMETERS);
															if (obps1 != NULL)
															{
																symbol_t *tbps1;
																tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																if (tbps1 != NULL)
																{
																	int32_t r2;
																	r2 = syntax_equal_psps(program, obps1, tbps1);
																	if (r2 == -1)
																	{
																		return -1;
																	}
																	else 
																	if (r2 == 1)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}
																	else
																	{
																		itable_t *it2;
																		it2 = table_rpush(frame, target);
																		if (it2 == NULL)
																		{
																			fprintf(stderr, "unable to allocate memory\n");
																			return -1;
																		}
																	}
																}
																else
																{
																	int32_t pwv = 0;
																	symbol_t *obp1;
																	for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																	{
																		symbol_t *obpv1;
																		obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																		if (obpv1 == NULL)
																		{
																			pwv = 1;
																			break;
																		}
																	}

																	if (pwv == 0)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}

																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
															}
															else
															{
																symbol_t *tbps1;
																tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																if (tbps1 != NULL)
																{
																	int32_t pwv = 0;
																	symbol_t *tbp1;
																	for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																	{
																		symbol_t *tbpv1;
																		tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																		if (tbpv1 == NULL)
																		{
																			pwv = 1;
																			break;
																		}
																	}

																	if (pwv == 0)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}

																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
																else
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
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
									symbol_t *tgs1;
									tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
									if (tgs1 != NULL)
									{
										int32_t gwv = 0;
										symbol_t *tg1;
										for (tg1 = tgs1->begin;tg1 != tgs1->end;tg1 = tg1->next)
										{
											if (symbol_check_type(tg1, SYMBOL_GENERIC))
											{
												symbol_t *tgv1;
												tgv1 = syntax_only_with(tg1, SYMBOL_VALUE);
												if (tgv1 != NULL)
												{
													gwv = 1;
													break;
												}
											}
										}
										if (gwv == 0)
										{
											symbol_t *tb1;
											for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
											{
												if (symbol_check_type(tb1, SYMBOL_FUNCTION))
												{
													symbol_t *tbk1;
													tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
													if (tbk1 != NULL)
													{
														if (syntax_idstrcmp(tbk1, "constructor") == 0)
														{
															symbol_t *obps1;
															obps1 = syntax_only_with(o1, SYMBOL_PARAMETERS);
															if (obps1 != NULL)
															{
																symbol_t *tbps1;
																tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																if (tbps1 != NULL)
																{
																	int32_t r2;
																	r2 = syntax_equal_psps(program, obps1, tbps1);
																	if (r2 == -1)
																	{
																		return -1;
																	}
																	else 
																	if (r2 == 1)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}
																	else
																	{
																		itable_t *it2;
																		it2 = table_rpush(frame, target);
																		if (it2 == NULL)
																		{
																			fprintf(stderr, "unable to allocate memory\n");
																			return -1;
																		}
																	}
																}
																else
																{
																	int32_t pwv = 0;
																	symbol_t *obp1;
																	for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																	{
																		symbol_t *obpv1;
																		obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																		if (obpv1 == NULL)
																		{
																			pwv = 1;
																			break;
																		}
																	}

																	if (pwv == 0)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}

																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
															}
															else
															{
																symbol_t *tbps1;
																tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
																if (tbps1 != NULL)
																{
																	int32_t pwv = 0;
																	symbol_t *tbp1;
																	for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																	{
																		symbol_t *tbpv1;
																		tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																		if (tbpv1 == NULL)
																		{
																			pwv = 1;
																			break;
																		}
																	}

																	if (pwv == 0)
																	{
																		syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																			ok1->declaration->position.line, ok1->declaration->position.column);
																		return -1;
																	}

																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
																else
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
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
										symbol_t *tb1;
										for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
										{
											if (symbol_check_type(tb1, SYMBOL_FUNCTION))
											{
												symbol_t *tbk1;
												tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
												if (tbk1 != NULL)
												{
													if (syntax_idstrcmp(tbk1, "constructor") == 0)
													{
														symbol_t *obps1;
														obps1 = syntax_only_with(o1, SYMBOL_PARAMETERS);
														if (obps1 != NULL)
														{
															symbol_t *tbps1;
															tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
															if (tbps1 != NULL)
															{
																int32_t r2;
																r2 = syntax_equal_psps(program, obps1, tbps1);
																if (r2 == -1)
																{
																	return -1;
																}
																else 
																if (r2 == 1)
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
																	return -1;
																}
																else
																{
																	itable_t *it2;
																	it2 = table_rpush(frame, target);
																	if (it2 == NULL)
																	{
																		fprintf(stderr, "unable to allocate memory\n");
																		return -1;
																	}
																}
															}
															else
															{
																int32_t pwv = 0;
																symbol_t *obp1;
																for (obp1 = obps1->begin;obp1 != obps1->end;obp1 = obp1->next)
																{
																	symbol_t *obpv1;
																	obpv1 = syntax_only_with(obp1, SYMBOL_VALUE);
																	if (obpv1 == NULL)
																	{
																		pwv = 1;
																		break;
																	}
																}

																if (pwv == 0)
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
																	return -1;
																}

																itable_t *it2;
																it2 = table_rpush(frame, target);
																if (it2 == NULL)
																{
																	fprintf(stderr, "unable to allocate memory\n");
																	return -1;
																}
															}
														}
														else
														{
															symbol_t *tbps1;
															tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
															if (tbps1 != NULL)
															{
																int32_t pwv = 0;
																symbol_t *tbp1;
																for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																{
																	symbol_t *tbpv1;
																	tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																	if (tbpv1 == NULL)
																	{
																		pwv = 1;
																		break;
																	}
																}

																if (pwv == 0)
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
																	return -1;
																}

																itable_t *it2;
																it2 = table_rpush(frame, target);
																if (it2 == NULL)
																{
																	fprintf(stderr, "unable to allocate memory\n");
																	return -1;
																}
															}
															else
															{
																syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																	ok1->declaration->position.line, ok1->declaration->position.column);
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
							if (symbol_check_type(o1, SYMBOL_PROPERTY))
							{
								symbol_t *tgs1;
								tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
								if (tgs1 != NULL)
								{
									int32_t gwv = 0;
									symbol_t *tg1;
									for (tg1 = tgs1->begin;tg1 != tgs1->end;tg1 = tg1->next)
									{
										if (symbol_check_type(tg1, SYMBOL_GENERIC))
										{
											symbol_t *tgv1;
											tgv1 = syntax_only_with(tg1, SYMBOL_VALUE);
											if (tgv1 != NULL)
											{
												gwv = 1;
												break;
											}
										}
									}
									if (gwv == 0)
									{
										symbol_t *tb1;
										for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
										{
											if (symbol_check_type(tb1, SYMBOL_FUNCTION))
											{
												symbol_t *tbk1;
												tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
												if (tbk1 != NULL)
												{
													if (syntax_idstrcmp(tbk1, "constructor") == 0)
													{
														symbol_t *tbps1;
														tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
														if (tbps1 != NULL)
														{
															symbol_t *tbps1;
															tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
															if (tbps1 != NULL)
															{
																int32_t pwv = 0;
																symbol_t *tbp1;
																for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
																{
																	symbol_t *tbpv1;
																	tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																	if (tbpv1 == NULL)
																	{
																		pwv = 1;
																		break;
																	}
																}

																if (pwv == 0)
																{
																	syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																		ok1->declaration->position.line, ok1->declaration->position.column);
																	return -1;
																}

																itable_t *it2;
																it2 = table_rpush(frame, target);
																if (it2 == NULL)
																{
																	fprintf(stderr, "unable to allocate memory\n");
																	return -1;
																}
															}
															else
															{
																syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																	ok1->declaration->position.line, ok1->declaration->position.column);
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
									symbol_t *tb1;
									for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
									{
										if (symbol_check_type(tb1, SYMBOL_FUNCTION))
										{
											symbol_t *tbk1;
											tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
											if (tbk1 != NULL)
											{
												if (syntax_idstrcmp(tbk1, "constructor") == 0)
												{
													symbol_t *tbps1;
													tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
													if (tbps1 != NULL)
													{
														int32_t pwv = 0;
														symbol_t *tbp1;
														for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
														{
															symbol_t *tbpv1;
															tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
															if (tbpv1 == NULL)
															{
																pwv = 1;
																break;
															}
														}

														if (pwv == 0)
														{
															syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																ok1->declaration->position.line, ok1->declaration->position.column);
															return -1;
														}

														itable_t *it2;
														it2 = table_rpush(frame, target);
														if (it2 == NULL)
														{
															fprintf(stderr, "unable to allocate memory\n");
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
							if (symbol_check_type(o1, SYMBOL_ENUM))
							{
								symbol_t *tgs1;
								tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
								if (tgs1 != NULL)
								{
									int32_t gwv = 0;
									symbol_t *tg1;
									for (tg1 = tgs1->begin;tg1 != tgs1->end;tg1 = tg1->next)
									{
										if (symbol_check_type(tg1, SYMBOL_GENERIC))
										{
											symbol_t *tgv1;
											tgv1 = syntax_only_with(tg1, SYMBOL_VALUE);
											if (tgv1 != NULL)
											{
												gwv = 1;
												break;
											}
										}
									}
									if (gwv == 0)
									{
										symbol_t *tb1;
										for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
										{
											if (symbol_check_type(tb1, SYMBOL_FUNCTION))
											{
												symbol_t *tbk1;
												tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
												if (tbk1 != NULL)
												{
													if (syntax_idstrcmp(tbk1, "constructor") == 0)
													{
														symbol_t *tbps1;
														tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
														if (tbps1 != NULL)
														{
															int32_t pwv = 0;
															symbol_t *tbp1;
															for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
															{
																symbol_t *tbpv1;
																tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																if (tbpv1 == NULL)
																{
																	pwv = 1;
																	break;
																}
															}

															if (pwv == 0)
															{
																syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																	ok1->declaration->position.line, ok1->declaration->position.column);
																return -1;
															}

															itable_t *it2;
															it2 = table_rpush(frame, target);
															if (it2 == NULL)
															{
																fprintf(stderr, "unable to allocate memory\n");
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
									symbol_t *tb1;
									for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
									{
										if (symbol_check_type(tb1, SYMBOL_FUNCTION))
										{
											symbol_t *tbk1;
											tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
											if (tbk1 != NULL)
											{
												if (syntax_idstrcmp(tbk1, "constructor") == 0)
												{
													symbol_t *tbps1;
													tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
													if (tbps1 != NULL)
													{
														int32_t pwv = 0;
														symbol_t *tbp1;
														for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
														{
															symbol_t *tbpv1;
															tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
															if (tbpv1 == NULL)
															{
																pwv = 1;
																break;
															}
														}

														if (pwv == 0)
														{
															syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																ok1->declaration->position.line, ok1->declaration->position.column);
															return -1;
														}

														itable_t *it2;
														it2 = table_rpush(frame, target);
														if (it2 == NULL)
														{
															fprintf(stderr, "unable to allocate memory\n");
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
							if (symbol_check_type(o1, SYMBOL_VAR))
							{
								symbol_t *tgs1;
								tgs1 = syntax_only_with(target, SYMBOL_GENERICS);
								if (tgs1 != NULL)
								{
									int32_t gwv = 0;
									symbol_t *tg1;
									for (tg1 = tgs1->begin;tg1 != tgs1->end;tg1 = tg1->next)
									{
										if (symbol_check_type(tg1, SYMBOL_GENERIC))
										{
											symbol_t *tgv1;
											tgv1 = syntax_only_with(tg1, SYMBOL_VALUE);
											if (tgv1 != NULL)
											{
												gwv = 1;
												break;
											}
										}
									}
									if (gwv == 0)
									{
										symbol_t *tb1;
										for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
										{
											if (symbol_check_type(tb1, SYMBOL_FUNCTION))
											{
												symbol_t *tbk1;
												tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
												if (tbk1 != NULL)
												{
													if (syntax_idstrcmp(tbk1, "constructor") == 0)
													{
														symbol_t *tbps1;
														tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
														if (tbps1 != NULL)
														{
															int32_t pwv = 0;
															symbol_t *tbp1;
															for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
															{
																symbol_t *tbpv1;
																tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
																if (tbpv1 == NULL)
																{
																	pwv = 1;
																	break;
																}
															}

															if (pwv == 0)
															{
																syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																	ok1->declaration->position.line, ok1->declaration->position.column);
																return -1;
															}

															itable_t *it2;
															it2 = table_rpush(frame, target);
															if (it2 == NULL)
															{
																fprintf(stderr, "unable to allocate memory\n");
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
									symbol_t *tb1;
									for (tb1 = target->begin;tb1 != target->end;tb1 = tb1->next)
									{
										if (symbol_check_type(tb1, SYMBOL_FUNCTION))
										{
											symbol_t *tbk1;
											tbk1 = syntax_extract_with(tb1, SYMBOL_KEY);
											if (tbk1 != NULL)
											{
												if (syntax_idstrcmp(tbk1, "constructor") == 0)
												{
													symbol_t *tbps1;
													tbps1 = syntax_only_with(tb1, SYMBOL_PARAMETERS);
													if (tbps1 != NULL)
													{
														int32_t pwv = 0;
														symbol_t *tbp1;
														for (tbp1 = tbps1->begin;tbp1 != tbps1->end;tbp1 = tbp1->next)
														{
															symbol_t *tbpv1;
															tbpv1 = syntax_only_with(tbp1, SYMBOL_VALUE);
															if (tbpv1 == NULL)
															{
																pwv = 1;
																break;
															}
														}

														if (pwv == 0)
														{
															syntax_error(program, tk1, "defination repeated, another defination in %lld:%lld",
																ok1->declaration->position.line, ok1->declaration->position.column);
															return -1;
														}

														itable_t *it2;
														it2 = table_rpush(frame, target);
														if (it2 == NULL)
														{
															fprintf(stderr, "unable to allocate memory\n");
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
	}

	table_t *frame2;
	frame2 = table_create();
	if (frame2 == NULL)
	{
		fprintf(stderr, "unable to allocate memory\n");
		return -1;
	}
	frame2->parent = frame;

	ilist_t *il1;
	il1 = list_rpush(scope, target);
	if (il1 == NULL)
	{
		fprintf(stderr, "unable to alloc memory\n");
		return -1;
	}

	symbol_t *a;
	for (a = target->begin;a != target->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			int32_t result;
			result = syntax_generics(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PROPERTY))
		{
			int32_t result;
			result = syntax_property(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			int32_t result;
			result = syntax_function(program, scope, frame2, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	table_destroy(frame2);
	list_unlink(scope, il1);
	return 1;
}

static int32_t
syntax_field(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
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
			syntax_error(program, target, "field without parent");
			return -1;
		}
	}
	else
	{
		syntax_error(program, target, "field without key");
		return -1;
	}

	return 1;
}

static int32_t
syntax_fields(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_FIELD))
		{
			int32_t result;
			result = syntax_field(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak)
			{
				symbol_t *parent = target->parent;
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
					syntax_error(program, target, "fields without parent");
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
syntax_import(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for(a = target->begin; a != target->end; a = a->next)
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

				ilist_t *r1;
				r1 = list_rpush(program->imports, root);
				if (r1 == NULL)
				{
					fprintf(stderr, "unable to allocate memory\n");
					return -1;
				}

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

				if(!symbol_link(target, target->end, root))
				{
					return -1;
				}

				continue;
			}
			else
			{
				syntax_error(program, target, "unable to access path");
				return -1;	
			}
		}

		if (symbol_check_type(a, SYMBOL_FIELDS))
		{
			int32_t result;
			result = syntax_fields(program, scope, frame, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	symbol_set_flag(target, SYMBOL_FLAG_SYNTAX);

	return 1;
}


static int32_t
syntax_module(program_t *program, list_t *scope, table_t *frame, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for(a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_IMPORT))
		{
			int32_t result;
			result = syntax_import(program, scope, frame, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(program, scope, frame, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(program, scope, frame, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_VAR))
		{
			int32_t result;
			result = syntax_var(program, scope, frame, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			int32_t result;
			result = syntax_function(program, scope, frame, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
	}

	symbol_set_flag(target, SYMBOL_FLAG_SYNTAX);
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
			list_t *response = list_create();
			if (response == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			list_t *scope = list_create();
			if (scope == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			table_t *frame = table_create();
			if (frame == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}
			

			int32_t result;
			result = syntax_module(program, scope, frame, a, response, SYNTAX_FLAG_NONE);
			if(result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}


