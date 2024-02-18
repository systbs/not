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


typedef enum syntax_access {
	SYNTAX_ACCESS_PUBLIC 				= 1 << 0,
	SYNTAX_ACCESS_PRIVATE 			= 1 << 1,
	SYNTAX_ACCESS_PROTECTED 		= 1 << 2,
	SYNTAX_ACCESS_BAN_PRIVATE 	= 1 << 3,
	SYNTAX_ACCESS_BAN_PROTECTED = 1 << 4
} syntax_access_t;

typedef enum syntax_route_type {
	SYNTAX_ROUTE_NONE 			= 1 << 0,
	SYNTAX_ROUTE_FORWARD 		= 1 << 1
} syntax_route_type_t;

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
syntax_comparison_id(symbol_t *id1, symbol_t *id2)
{
	node_t *nid1 = id1->declaration;
	node_basic_t *nbid1 = (node_basic_t *)nid1->value;

	node_t *nid2 = id2->declaration;
	node_basic_t *nbid2 = (node_basic_t *)nid2->value;

	//printf("%s %s\n", nbid1->value, nbid2->value);

	return (strncmp(nbid1->value, nbid2->value, 
		max(strlen(nbid1->value), strlen(nbid2->value))) == 0) ? 1 : -1;
}



static symbol_t *
syntax_type_of(program_t *program, symbol_t *t1);

static int32_t
syntax_ga(program_t *program, symbol_t *g1, symbol_t *a1)
{
	symbol_t *gt_1;
	gt_1 = syntax_extract_with(g1, SYMBOL_TYPE);
	if (gt_1)
	{
		symbol_t *gtr_1;
		gtr_1 = syntax_type_of(program, gt_1);
		if (gtr_1)
		{
			symbol_t *an_1;
			an_1 = syntax_extract_with(a1, SYMBOL_KEY);
			if (an_1)
			{
				symbol_t *anr_1;
				anr_1 = syntax_type_of(program, an_1);
				if (anr_1)
				{
					return (gtr_1 == anr_1) ? 1 : -1;
				}
				else
				{
					syntax_error(program, an_1, "refrerence of this type not found");
					return -1;
				}
			}
			else
			{
				syntax_error(program, a1, "refrerence of this type not found");
				return -1;
			}
		}
		else
		{
			syntax_error(program, g1, "refrerence of this type not found");
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_gsas(program_t *program, symbol_t *gs1, symbol_t *as1)
{
	uint64_t gs1_cnt = 0;
	uint64_t as1_cnt = 0;

	symbol_t *a;
	for (a = gs1->begin;a != gs1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_GENERIC))
		{
			gs1_cnt += 1;
			as1_cnt = 0;

			symbol_t *b;
			for (b = as1->begin;b != as1->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_ARGUMENT))
				{
					as1_cnt += 1;
					if (as1_cnt < gs1_cnt)
					{
						continue;
					}
					int32_t result;
					result = syntax_ga(program, a, b);
					if (result == 1)
					{
						return 1;
					}
					else 
					if (result == 0)
					{
						return -1;
					}
					return -1;
				}
			}
			
			if (as1_cnt < gs1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(a, SYMBOL_VALUE);
				if (!value)
				{
					return -1;
				}
			}
		}
	}

	as1_cnt = 0;
	symbol_t *b;
	for (b = as1->begin;b != as1->end;b = b->next)
	{
		if (symbol_check_type(b, SYMBOL_ARGUMENT))
		{
			as1_cnt += 1;
			if (as1_cnt > gs1_cnt)
			{
				return -1;
			}
		}
	}

	return 1;
}

static symbol_t *
syntax_in_backward(symbol_t *t1, symbol_t *t2)
{
	if (t1 == t2)
	{
		return t2;
	}
	if (t2->parent)
	{
		return syntax_in_backward(t1, t2->parent);
	}
	return NULL;
}

static symbol_t *
syntax_type_of_in_scope(program_t *program, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route)
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
				if (syntax_comparison_id(ak, t1) == 1)
				{
					symbol_t *gs;
					gs = syntax_only_with(a, SYMBOL_GENERICS);
					if (gs)
					{
						if (arguments)
						{
							if (syntax_gsas(program, gs, arguments))
							{
								goto region_access;
							}
							continue;
						}
						else
						{
							int32_t no_match = 0;
							symbol_t *b;
							for (b = gs->begin;b != gs->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_VALUE);
									if (!bv)
									{
										no_match = 1;
										break;
									}
								}
							}
							if (no_match)
							{
								goto region_access;
							}
						}
					}
					else
					{
						if (arguments)
						{
							continue;
						}
						goto region_access;
					}
				}
				continue;
			}
			else
			{
				syntax_error(program, a, "does not include the key field");
				return NULL;
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
						if ((syntax_comparison_id(bk, t1) == 1) && (bk != t1) && !arguments)
						{
							return b;
						}
					}
				}
			}
			continue;
		}
		
		continue;
region_access:
		if ((route == (route & SYNTAX_ROUTE_FORWARD)))
		{
			if (!syntax_in_backward(base, a))
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					node_class_t *class = a->declaration->value;
					if ((class->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
					{
						syntax_error(program, a, "private access");
						return NULL;
					}
				}
			}
		}
		return a;
	}

	if (base->parent && (route == (route & SYNTAX_ROUTE_NONE)))
	{
		return syntax_type_of_in_scope(program, base->parent, t1, arguments, route);
	}

	return NULL;
}

static symbol_t *
syntax_type_of_by_arguments(program_t *program, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route)
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
				symbol_t *r1;
				r1 = syntax_type_of_by_arguments(program, base, left, NULL, route);
				if (r1)
				{
					symbol_t *r2;
					r2 = syntax_type_of_by_arguments(program, r1, right, arguments, SYNTAX_ROUTE_FORWARD);
					if (r2)
					{
						return r2;
					}
					else
					{
						syntax_error(program, right, "field not found in (%lld:%lld)",
							r1->declaration->position.line, r1->declaration->position.column);
						return NULL;
					}
				}
				else
				{
					syntax_error(program, left, "field not found");
					return NULL;
				}
			}
			else
			{
				syntax_error(program, t1, "attribute does not include the right field");
				return NULL;
			}
		}
		else
		{
			syntax_error(program, t1, "attribute does not include the left field");
			return NULL;
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
				symbol_t *r1;
				r1 = syntax_type_of_by_arguments(program, base, key, arguments1, route);
				if (r1)
				{
					return r1;
				}
				else
				{
					syntax_error(program, key, "field not found");
					return NULL;
				}
			}
			else
			{
				syntax_error(program, t1, "attribute does not include the arguments field");
				return NULL;
			}
		}
		else
		{
			syntax_error(program, t1, "attribute does not include the key field");
			return NULL;
		}
	}
	else
	if (symbol_check_type(t1, SYMBOL_ID))
	{
		symbol_t *r;
		r = syntax_type_of_in_scope(program, base, t1, arguments, route);
		if (r)
		{
			return r;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		syntax_error(program, t1, "the reference is not a routable");
		return NULL;
	}
}

static symbol_t *
syntax_type_of(program_t *program, symbol_t *t1)
{
	return syntax_type_of_by_arguments(program, t1->parent, t1, NULL, SYNTAX_ROUTE_NONE);
}



static int32_t
syntax_gg(program_t *program, symbol_t *g1, symbol_t *g2)
{
	symbol_t *gt_1;
	gt_1 = syntax_extract_with(g1, SYMBOL_TYPE);
	if (gt_1)
	{
		symbol_t *gtr_1;
		gtr_1 = syntax_type_of(program, gt_1);
		if (gtr_1)
		{
			symbol_t *gt_2;
			gt_2 = syntax_extract_with(g2, SYMBOL_TYPE);
			if (gt_2)
			{
				symbol_t *gtr_2;
				gtr_2 = syntax_type_of(program, gt_2);
				if (gtr_2)
				{
					return (gtr_1 == gtr_2) ? 1 : 0;
				}
				else
				{
					syntax_error(program, gt_2, "refrerence of this type not found");
					return -1;
				}
			}
			else
			{
				return 1;
			}
		}
		else
		{
			syntax_error(program, gt_1, "refrerence of this type not found");
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_gsgs(program_t *program, symbol_t *gs1, symbol_t *gs2)
{
	uint64_t gs1_cnt = 0;
	uint64_t gs2_cnt = 0;

	symbol_t *a;
	for (a = gs1->begin;a != gs1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_GENERIC))
		{
			gs1_cnt += 1;
			gs2_cnt = 0;

			symbol_t *b;
			for (b = gs2->begin;b != gs2->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_GENERIC))
				{
					gs2_cnt += 1;
					if (gs2_cnt < gs1_cnt)
					{
						continue;
					}
					int32_t result;
					result = syntax_gg(program, a, b);
					if (result == 1)
					{
						return 1;
					}
					else 
					if (result == 0)
					{
						return -1;
					}
					return -1;
				}
			}
			
			if (gs2_cnt < gs1_cnt)
			{
				symbol_t *value;
				value = syntax_only_with(a, SYMBOL_VALUE);
				if (!value)
				{
					return -1;
				}
			}
		}
	}

	gs2_cnt = 0;
	symbol_t *b;
	for (b = gs2->begin;b != gs2->end;b = b->next)
	{
		if (symbol_check_type(b, SYMBOL_GENERIC))
		{
			gs2_cnt += 1;
			if (gs2_cnt > gs1_cnt)
			{
				symbol_t *value;
				value = syntax_only_with(b, SYMBOL_VALUE);
				if (!value)
				{
					return -1;
				}
			}
		}
	}

	return 1;
}

static int32_t
syntax_tt(program_t *program, symbol_t *t1, symbol_t *t2)
{
	if (symbol_check_type(t1, SYMBOL_AND))
	{
		int32_t rl = 0;

		symbol_t *left;
		left = syntax_extract_with(t1, SYMBOL_LEFT);
		if (left)
		{
			rl = syntax_tt(program, left, t2);
			if (rl == -1)
			{
				return -1;
			}
		}

		int32_t rr = 0;

		symbol_t *right;
		right = syntax_extract_with(t1, SYMBOL_RIGHT);
		if (right)
		{
			rr = syntax_tt(program, right, t2);
			if (rr == -1)
			{
				return -1;
			}
		}

		return rl & rr;
	}
	else
	if (symbol_check_type(t1, SYMBOL_OR))
	{
		int32_t rl = 0;

		symbol_t *left;
		left = syntax_extract_with(t1, SYMBOL_LEFT);
		if (left)
		{
			rl = syntax_tt(program, left, t2);
			if (rl == -1)
			{
				return -1;
			}
		}

		int32_t rr = 0;

		symbol_t *right;
		right = syntax_extract_with(t1, SYMBOL_RIGHT);
		if (right)
		{
			rr = syntax_tt(program, right, t2);
			if (rr == -1)
			{
				return -1;
			}
		}

		return rl & rr;
	}
	else
	{
		if (symbol_check_type(t2, SYMBOL_AND))
		{
			int32_t rl = 0;

			symbol_t *left;
			left = syntax_extract_with(t2, SYMBOL_LEFT);
			if (left)
			{
				rl = syntax_tt(program, t1, left);
				if (rl == -1)
				{
					return -1;
				}
			}

			int32_t rr = 0;

			symbol_t *right;
			right = syntax_extract_with(t2, SYMBOL_RIGHT);
			if (right)
			{
				rr = syntax_tt(program, t1, right);
				if (rr == -1)
				{
					return -1;
				}
			}

			return rl & rr;
		}
		else
		if (symbol_check_type(t2, SYMBOL_OR))
		{
			int32_t rl = 0;

			symbol_t *left;
			left = syntax_extract_with(t2, SYMBOL_LEFT);
			if (left)
			{
				rl = syntax_tt(program, t1, left);
				if (rl == -1)
				{
					return -1;
				}
			}

			int32_t rr = 0;

			symbol_t *right;
			right = syntax_extract_with(t2, SYMBOL_RIGHT);
			if (right)
			{
				rr = syntax_tt(program, t1, right);
				if (rr == -1)
				{
					return -1;
				}
			}

			return rl | rr;
		}
		else
		{
			symbol_t *tt1;
			tt1 = syntax_type_of(program, t1);
			if (tt1)
			{
				symbol_t *tt2;
				tt2 = syntax_type_of(program, t2);
				if (tt2)
				{
					return tt1 == tt2 ? 1 : 0;
				}
				else
				{
					syntax_error(program, t2, "refrerence of this type not found");
					return -1;
				}
			}
			else
			{
				syntax_error(program, t1, "refrerence of this type not found");
				return -1;
			}
		}
	}
	return -1;
}

static int32_t
syntax_pp(program_t *program, symbol_t *p1, symbol_t *p2)
{
	symbol_t *pt1;
	pt1 = syntax_extract_with(p1, SYMBOL_TYPE);
	if (pt1)
	{
		symbol_t *pt2;
		pt2 = syntax_extract_with(p2, SYMBOL_TYPE);
		if (pt2)
		{
			return syntax_tt(program, pt1, pt2);
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 1;
	}
}

static int32_t
syntax_psps(program_t *program, symbol_t *ps1, symbol_t *ps2)
{
	uint64_t ps1_cnt = 0;
	uint64_t ps2_cnt = 0;

	symbol_t *a;
	for (a = ps1->begin;a != ps1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PARAMETER))
		{
			ps1_cnt += 1;
			ps2_cnt = 0;

			symbol_t *b;
			for (b = ps2->begin;b != ps2->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_PARAMETER))
				{
					ps2_cnt += 1;
					if (ps2_cnt < ps1_cnt)
					{
						continue;
					}
					int32_t result;
					result = syntax_pp(program, a, b);
					if (result == 0)
					{
						return 0;
					}
					else 
					if (result == -1)
					{
						return -1;
					}
					break;
				}
			}
			
			if (ps2_cnt < ps1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(a, SYMBOL_VALUE);
				if (!value)
				{
					return 0;
				}
			}
		}
	}

	ps2_cnt = 0;
	symbol_t *b;
	for (b = ps2->begin;b != ps2->end;b = b->next)
	{
		if (symbol_check_type(b, SYMBOL_GENERIC))
		{
			ps2_cnt += 1;
			if (ps2_cnt > ps1_cnt)
			{
				symbol_t *value;
				value = syntax_only_with(b, SYMBOL_VALUE);
				if (!value)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}

static int32_t
syntax_aa(program_t *program, symbol_t *a1, symbol_t *a2)
{
	symbol_t *an1;
	an1 = syntax_extract_with(a1, SYMBOL_KEY);
	if (an1)
	{
		symbol_t *an2;
		an2 = syntax_extract_with(a2, SYMBOL_KEY);
		if (an2)
		{
			return 1;
		}
		else
		{
			syntax_error(program, a2, "argument without key");
			return -1;
		}
	}
	else
	{
		syntax_error(program, a1, "argument without key");
		return -1;
	}
	return 1;
}

static int32_t
syntax_asas(program_t *program, symbol_t *as1, symbol_t *as2)
{
	uint64_t as1_cnt = 0;
	uint64_t as2_cnt = 0;

	symbol_t *a;
	for (a = as1->begin;a != as1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_ARGUMENT))
		{
			as1_cnt += 1;
			as2_cnt = 0;

			symbol_t *b;
			for (b = as2->begin;b != as2->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_ARGUMENT))
				{
					as2_cnt += 1;
					if (as2_cnt < as1_cnt)
					{
						continue;
					}
					int32_t result;
					result = syntax_aa(program, a, b);
					if (result == 0)
					{
						return 0;
					}
					else 
					if (result == -1)
					{
						return -1;
					}
					break;
				}
			}

			if (as2_cnt < as1_cnt)
			{
				symbol_t *value;
				value = syntax_only_with(a, SYMBOL_VALUE);
				if (!value)
				{
					return 0;
				}
			}

		}
	}

	as2_cnt = 0;
	symbol_t *b;
	for (b = as2->begin;b != as2->end;b = b->next)
	{
		if (symbol_check_type(b, SYMBOL_ARGUMENT))
		{
			as2_cnt += 1;
			if (as2_cnt > as1_cnt)
			{
				return 0;
			}
		}
	}

	return 1;
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
							if (syntax_comparison_id(ak, bk))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
							if (syntax_comparison_id(ak, bk) == 1)
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
									result = syntax_gsgs(program, cgs, ags);
									if (result == 1)
									{
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											int32_t no_value2 = -1;
											symbol_t *b;
											for (b = cps->begin;(b != cps->end);b = b->next)
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
									if (result == -1)
									{
										return -1;
									}
									else
									{
										continue;
									}
								}
								else
								{
									int32_t no_value = -1;
									symbol_t *b;
									for (b = cgs->begin;(b != cgs->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_GENERIC))
										{
											symbol_t *cgv;
											cgv = syntax_only_with(b, SYMBOL_VALUE);
											if (!cgv)
											{
												no_value = 1;
												break;
											}
										}
									}
									if (no_value == -1)
									{
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											int32_t no_value2 = -1;
											for (b = cps->begin;(b != cps->end);b = b->next)
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
							}
							else
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
											agv = syntax_only_with(b, SYMBOL_VALUE);
											if (!agv)
											{
												no_value = 1;
												break;
											}
										}
									}
									if (no_value == -1)
									{
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											int32_t no_value2 = -1;
											for (b = cps->begin;(b != cps->end);b = b->next)
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
									symbol_t *cps;
									cps = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (cps)
									{
										int32_t no_value2 = -1;
										symbol_t *b;
										for (b = cps->begin;(b != cps->end);b = b->next)
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
									result = syntax_gsgs(program, cgs, ags);
									if (result == 1)
									{
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t result;
												result = syntax_psps(program, cps, aps);
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
												if (result == -1)
												{
													return -1;
												}
												else
												{
													continue;
												}
											}
											else
											{
												int32_t no_value = -1;
												symbol_t *b;
												for (b = cps->begin;(b != cps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *cpv;
														cpv = syntax_only_with(b, SYMBOL_VALUE);
														if (!cpv)
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
										}
										else
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t no_value = -1;
												symbol_t *b;
												for (b = aps->begin;(b != aps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *apv;
														apv = syntax_only_with(b, SYMBOL_VALUE);
														if (!apv)
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
									}
									else
									if (result == -1)
									{
										return -1;
									}
									else
									{
										continue;
									}
								}
								else
								{
									int32_t no_value = -1;
									symbol_t *b;
									for (b = cgs->begin;(b != cgs->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_GENERIC))
										{
											symbol_t *cgv;
											cgv = syntax_only_with(b, SYMBOL_VALUE);
											if (!cgv)
											{
												no_value = 1;
												break;
											}
										}
									}
									if (no_value == -1)
									{
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t result;
												result = syntax_psps(program, cps, aps);
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
												if (result == -1)
												{
													return -1;
												}
												else
												{
													continue;
												}
											}
											else
											{
												int32_t no_value = -1;
												for (b = cps->begin;(b != cps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *cpv;
														cpv = syntax_only_with(b, SYMBOL_VALUE);
														if (!cpv)
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
										}
										else
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t no_value = -1;
												for (b = aps->begin;(b != aps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *apv;
														apv = syntax_only_with(b, SYMBOL_VALUE);
														if (!apv)
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
									}
									else
									{
										continue;
									}
								}
							}
							else
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
											agv = syntax_only_with(b, SYMBOL_VALUE);
											if (!agv)
											{
												no_value = 1;
												break;
											}
										}
									}
									if (no_value == -1)
									{
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t result;
												result = syntax_psps(program, cps, aps);
												if (result == 1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
														ak->declaration->position.line, ak->declaration->position.column);
													return -1;
												}
												if (result == -1)
												{
													return -1;
												}
												else
												{
													continue;
												}
											}
											else
											{
												int32_t no_value = -1;
												for (b = cps->begin;(b != cps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *cpv;
														cpv = syntax_only_with(b, SYMBOL_VALUE);
														if (!cpv)
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
										}
										else
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t no_value = -1;
												for (b = aps->begin;(b != aps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *apv;
														apv = syntax_only_with(b, SYMBOL_VALUE);
														if (!apv)
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
									}
									else
									{
										continue;
									}
								}
								else
								{
									symbol_t *cps;
									cps = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (cps)
									{
										symbol_t *aps;
										aps = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (aps)
										{
											int32_t result;
											result = syntax_psps(program, cps, aps);
											if (result == 1)
											{
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													ak->declaration->position.line, ak->declaration->position.column);
												return -1;
											}
											else
											if (result == -1)
											{
												return -1;
											}
											else
											{
												continue;
											}
										}
										else
										{
											int32_t no_value = -1;
											symbol_t *b;
											for (b = cps->begin;(b != cps->end);b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_PARAMETER))
												{
													symbol_t *cpv;
													cpv = syntax_only_with(b, SYMBOL_VALUE);
													if (!cpv)
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
									}
									else
									{
										symbol_t *aps;
										aps = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (aps)
										{
											int32_t no_value = -1;
											symbol_t *b;
											for (b = aps->begin;(b != aps->end);b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_PARAMETER))
												{
													symbol_t *apv;
													apv = syntax_only_with(b, SYMBOL_VALUE);
													if (!apv)
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
						{
							symbol_t *cgs;
							cgs = syntax_extract_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = cgs->begin;(b != cgs->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_extract_with(b, SYMBOL_VALUE);
										if (!cgv)
										{
											no_value = 1;
											break;
										}
									}
								}
								if (no_value == -1)
								{
									symbol_t *cps;
									cps = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (cps)
									{
										int32_t no_value2 = -1;
										for (b = cps->begin;(b != cps->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_PARAMETER))
											{
												symbol_t *cpv;
												cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
								symbol_t *cps;
								cps = syntax_only_with(current, SYMBOL_PARAMETERS);
								if (cps)
								{
									int32_t no_value2 = -1;
									symbol_t *b;
									for (b = cps->begin;(b != cps->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_PARAMETER))
										{
											symbol_t *cpv;
											cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
						{
							symbol_t *cgs;
							cgs = syntax_extract_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = cgs->begin;(b != cgs->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_extract_with(b, SYMBOL_VALUE);
										if (!cgv)
										{
											no_value = 1;
											break;
										}
									}
								}
								if (no_value == -1)
								{
									symbol_t *cps;
									cps = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (cps)
									{
										int32_t no_value2 = -1;
										for (b = cps->begin;(b != cps->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_PARAMETER))
											{
												symbol_t *cpv;
												cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
								symbol_t *cps;
								cps = syntax_only_with(current, SYMBOL_PARAMETERS);
								if (cps)
								{
									int32_t no_value2 = -1;
									symbol_t *b;
									for (b = cps->begin;(b != cps->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_PARAMETER))
										{
											symbol_t *cpv;
											cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
						{
							symbol_t *cgs;
							cgs = syntax_extract_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = cgs->begin;(b != cgs->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_extract_with(b, SYMBOL_VALUE);
										if (!cgv)
										{
											no_value = 1;
											break;
										}
									}
								}
								if (no_value == -1)
								{
									symbol_t *cps;
									cps = syntax_only_with(current, SYMBOL_PARAMETERS);
									if (cps)
									{
										int32_t no_value2 = -1;
										for (b = cps->begin;(b != cps->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_PARAMETER))
											{
												symbol_t *cpv;
												cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
								symbol_t *cps;
								cps = syntax_only_with(current, SYMBOL_PARAMETERS);
								if (cps)
								{
									int32_t no_value2 = -1;
									symbol_t *b;
									for (b = cps->begin;(b != cps->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_PARAMETER))
										{
											symbol_t *cpv;
											cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
								{
									symbol_t *cgs;
									cgs = syntax_extract_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t no_value = -1;
										symbol_t *b;
										for (b = cgs->begin;(b != cgs->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_extract_with(b, SYMBOL_VALUE);
												if (!cgv)
												{
													no_value = 1;
													break;
												}
											}
										}
										if (no_value == -1)
										{
											symbol_t *cps;
											cps = syntax_only_with(current, SYMBOL_PARAMETERS);
											if (cps)
											{
												int32_t no_value2 = -1;
												for (b = cps->begin;(b != cps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *cpv;
														cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													dk->declaration->position.line, dk->declaration->position.column);
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
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											int32_t no_value2 = -1;
											symbol_t *b;
											for (b = cps->begin;(b != cps->end);b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_PARAMETER))
												{
													symbol_t *cpv;
													cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												dk->declaration->position.line, dk->declaration->position.column);
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
								{
									symbol_t *cgs;
									cgs = syntax_extract_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t no_value = -1;
										symbol_t *b;
										for (b = cgs->begin;(b != cgs->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_extract_with(b, SYMBOL_VALUE);
												if (!cgv)
												{
													no_value = 1;
													break;
												}
											}
										}
										if (no_value == -1)
										{
											symbol_t *cps;
											cps = syntax_only_with(current, SYMBOL_PARAMETERS);
											if (cps)
											{
												int32_t no_value2 = -1;
												for (b = cps->begin;(b != cps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *cpv;
														cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													dk->declaration->position.line, dk->declaration->position.column);
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
										symbol_t *cps;
										cps = syntax_only_with(current, SYMBOL_PARAMETERS);
										if (cps)
										{
											int32_t no_value2 = -1;
											symbol_t *b;
											for (b = cps->begin;(b != cps->end);b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_PARAMETER))
												{
													symbol_t *cpv;
													cpv = syntax_extract_with(b, SYMBOL_VALUE);
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
											syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
												dk->declaration->position.line, dk->declaration->position.column);
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
									result = syntax_gsgs(program, cgs, ags);
									if (result == 1)
									{
										syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
											ak->declaration->position.line, ak->declaration->position.column);
										return -1;
									}
									else
									if (result == -1)
									{
										return -1;
									}
								}
								else
								{
									int32_t no_value = -1;
									symbol_t *b;
									for (b = cgs->begin;(b != cgs->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(b, SYMBOL_VALUE);
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
								}
							}
							else
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
											agv = syntax_only_with(b, SYMBOL_VALUE);
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
									result = syntax_gsgs(program, cgs, ags);
									if (result == 1)
									{
										symbol_t *aps;
										aps = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (aps)
										{
											int32_t no_value = -1;
											symbol_t *b;
											for (b = aps->begin;(b != aps->end);b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_PARAMETER))
												{
													symbol_t *agv;
													agv = syntax_only_with(b, SYMBOL_VALUE);
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
									if (result == -1)
									{
										return -1;
									}
								}
								else
								{
									int32_t no_value = -1;
									symbol_t *b;
									for (b = cgs->begin;(b != cgs->end);b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(b, SYMBOL_VALUE);
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
							}
							else
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
											agv = syntax_only_with(b, SYMBOL_VALUE);
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
											int32_t no_value = -1;
											symbol_t *b;
											for (b = aps->begin;(b != aps->end);b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_PARAMETER))
												{
													symbol_t *agv;
													agv = syntax_only_with(b, SYMBOL_VALUE);
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
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t no_value = -1;
												symbol_t *b;
												for (b = aps->begin;(b != aps->end);b = b->next)
												{
													if (symbol_check_type(b, SYMBOL_PARAMETER))
													{
														symbol_t *agv;
														agv = syntax_only_with(b, SYMBOL_VALUE);
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
									}
								}
								else
								{
									symbol_t *aps;
									aps = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (aps)
									{
										int32_t no_value = -1;
										symbol_t *b;
										for (b = aps->begin;(b != aps->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_PARAMETER))
											{
												symbol_t *agv;
												agv = syntax_only_with(b, SYMBOL_VALUE);
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
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = cgs->begin;(b != cgs->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(b, SYMBOL_VALUE);
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
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = cgs->begin;(b != cgs->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(b, SYMBOL_VALUE);
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
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
						{
							symbol_t *cgs;
							cgs = syntax_only_with(current, SYMBOL_GENERICS);
							if (cgs)
							{
								int32_t no_value = -1;
								symbol_t *b;
								for (b = cgs->begin;(b != cgs->end);b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_GENERIC))
									{
										symbol_t *agv;
										agv = syntax_only_with(b, SYMBOL_VALUE);
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
								{
									symbol_t *cgs;
									cgs = syntax_only_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t no_value = -1;
										symbol_t *b;
										for (b = cgs->begin;(b != cgs->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_GENERIC))
											{
												symbol_t *agv;
												agv = syntax_only_with(b, SYMBOL_VALUE);
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
								if ((syntax_comparison_id(ck, dk) == 1) && (current != a))
								{
									symbol_t *cgs;
									cgs = syntax_only_with(current, SYMBOL_GENERICS);
									if (cgs)
									{
										int32_t no_value = -1;
										symbol_t *b;
										for (b = cgs->begin;(b != cgs->end);b = b->next)
										{
											if (symbol_check_type(b, SYMBOL_GENERIC))
											{
												symbol_t *agv;
												agv = syntax_only_with(b, SYMBOL_VALUE);
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
								if ((syntax_comparison_id(ck, ak) == 1) && (current != a))
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
										if (syntax_comparison_id(ck, akk) == 1)
										{
											symbol_t *aas;
											aas = syntax_only_with(ak, SYMBOL_ARGUMENTS);
											if (aas)
											{
												int32_t no_value = -1;
												symbol_t *b;
												for (b = aas->begin;(b != aas->end);b = b->next)
												{
													symbol_t *bv;
													bv = syntax_only_with(b, SYMBOL_VALUE);
													if (!bv)
													{
														no_value = 1;
														break;
													}
												}
												if (no_value == -1)
												{
													syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
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
												syntax_error(program, ck, "defination repeated, another defination in %lld:%lld",
													akk->declaration->position.line, akk->declaration->position.column);
												return -1;
											}
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
										if (syntax_comparison_id(ak, ckk) == 1)
										{
											symbol_t *cas;
											cas = syntax_only_with(ck, SYMBOL_ARGUMENTS);
											if (cas)
											{
												int32_t no_value = -1;
												symbol_t *b;
												for (b = cas->begin;(b != cas->end);b = b->next)
												{
													symbol_t *bv;
													bv = syntax_only_with(b, SYMBOL_VALUE);
													if (!bv)
													{
														no_value = 1;
														break;
													}
												}
												if (no_value == -1)
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
												syntax_error(program, ckk, "defination repeated, another defination in %lld:%lld",
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
										if (syntax_comparison_id(akk, ckk) == 1)
										{
											symbol_t *cas;
											cas = syntax_only_with(ck, SYMBOL_ARGUMENTS);
											if (cas)
											{
												symbol_t *aas;
												aas = syntax_only_with(ak, SYMBOL_ARGUMENTS);
												if (aas)
												{
													int32_t result;
													result = syntax_asas(program, cas, aas);
													if (result == 1)
													{
														syntax_error(program, ckk, "defination repeated, another defination in %lld:%lld",
															akk->declaration->position.line, akk->declaration->position.column);
														return -1;
													}
													else
													if (result == -1)
													{
														return -1;
													}
													else
													{
														continue;
													}
												}
												else
												{
													syntax_error(program, ak, "composite without arguments");
													return -1;
												}
											}
											else
											{
												syntax_error(program, ck, "composite without arguments");
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
										if (syntax_comparison_id(ak, bk) == 1)
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
syntax_import(program_t *program, symbol_t *import)
{
	symbol_t *a;
	for(a = import->begin; a != import->end; a = a->next)
	{
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

	symbol_set_flag(import, SYMBOL_FLAG_SYNTAX);

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


