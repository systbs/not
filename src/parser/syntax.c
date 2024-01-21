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
	SYNTAX_ROUTE_FORWARD 	= 1 << 1
} syntax_route_type_t;

static error_t *
syntax_error(graph_t *graph, symbol_t *current, const char *format, ...)
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

	if (list_rpush(graph->errors, (uint64_t)error))
	{
		return NULL;
	}

	return error;
}


static int32_t
syntax_function(graph_t *graph, symbol_t *current);

static int32_t
syntax_block(graph_t *graph, symbol_t *current);

static int32_t
syntax_assign(graph_t *graph, symbol_t *current);

static int32_t
syntax_import(graph_t *graph, symbol_t *current);

static int32_t
syntax_generics(graph_t *graph, symbol_t *current);

static int32_t
syntax_generic(graph_t *graph, symbol_t *current);

static int32_t
syntax_fields(graph_t *graph, symbol_t *current);

static int32_t
syntax_field(graph_t *graph, symbol_t *current);




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
syntax_unique(graph_t *graph, symbol_t *s1, symbol_t *s2);

static int32_t
syntax_unique_gsas(graph_t *graph, symbol_t *gs1, symbol_t *as1);

static int32_t
syntax_unique_gsgs(graph_t *graph, symbol_t *gs1, symbol_t *gs2);

static int32_t
syntax_unique_asas(graph_t *graph, symbol_t *as1, symbol_t *as2);

static int32_t
syntax_unique_psps(graph_t *graph, symbol_t *ps1, symbol_t *ps2);




static int32_t
syntax_comparison_id(symbol_t *id1, symbol_t *id2)
{
	node_t *nid1 = id1->declaration;
	node_basic_t *nbid1 = (node_basic_t *)nid1->value;

	node_t *nid2 = id2->declaration;
	node_basic_t *nbid2 = (node_basic_t *)nid2->value;

	//printf("%s %s\n", nbid1->value, nbid2->value);

	return (strncmp(nbid1->value, nbid2->value, 
		max(strlen(nbid1->value), strlen(nbid2->value))) == 0);
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
syntax_reference_in_scope(graph_t *graph, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route)
{
	symbol_t *a;
	for (a = base->begin;(a != base->end);a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_CLASS))
		{
			symbol_t *an;
			an = syntax_extract_with(a, SYMBOL_TYPE_NAME);
			if (an)
			{
				symbol_t *tn1;
				tn1 = syntax_extract_with(t1, SYMBOL_TYPE_NAME);
				if (tn1)
				{
					int32_t r;
					r = syntax_comparison_id(an, tn1);
					if (r)
					{
						symbol_t *gs;
						gs = syntax_only_with(a, SYMBOL_TYPE_GENERICS);
						if (gs)
						{
							if (arguments)
							{
								if (syntax_unique_gsas(graph, gs, arguments))
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
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
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
					syntax_error(graph, t1, "does not include the name field");
					return NULL;
				}
			}
			else
			{
				syntax_error(graph, a, "does not include the name field");
				return NULL;
			}
		}
		
		if (symbol_check_type(a, SYMBOL_TYPE_TYPE))
		{
			symbol_t *an;
			an = syntax_extract_with(a, SYMBOL_TYPE_NAME);
			if (an)
			{
				symbol_t *tn1;
				tn1 = syntax_extract_with(t1, SYMBOL_TYPE_NAME);
				if (tn1)
				{
					int32_t r;
					r = syntax_comparison_id(an, tn1);
					if (r)
					{
						symbol_t *gs;
						gs = syntax_only_with(a, SYMBOL_TYPE_GENERICS);
						if (gs)
						{
							if (arguments)
							{
								if (syntax_unique_gsas(graph, gs, arguments))
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
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
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
					syntax_error(graph, t1, "does not include the name field");
					return NULL;
				}
			}
			else
			{
				syntax_error(graph, a, "does not include the name field");
				return NULL;
			}
		}

		continue;
region_access:
		if ((route == (route & SYNTAX_ROUTE_FORWARD)))
		{
			if (!syntax_in_backward(base, a))
			{
				if (symbol_check_type(a, SYMBOL_TYPE_CLASS))
				{
					node_class_t *class = a->declaration->value;
					if ((class->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
					{
						syntax_error(graph, a, "private access");
						return NULL;
					}
				}

				if (symbol_check_type(a, SYMBOL_TYPE_TYPE))
				{
					node_type_t *type = a->declaration->value;
					if ((type->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
					{
						syntax_error(graph, a, "private access");
						return NULL;
					}
				}
			}
		}
	}

	if (base->parent && (route == (route & SYNTAX_ROUTE_NONE)))
	{
		return syntax_reference_in_scope(graph, base->parent, t1, arguments, route);
	}

	return NULL;
}

static symbol_t *
syntax_reference_by_arguments(graph_t *graph, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route)
{
	if (symbol_check_type(t1, SYMBOL_TYPE_ATTR))
	{
		symbol_t *left;
		left = syntax_extract_with(t1, SYMBOL_TYPE_LEFT);
		if (left)
		{
			symbol_t *right;
			right = syntax_extract_with(t1, SYMBOL_TYPE_RIGHT);
			if (right)
			{
				symbol_t *r1;
				r1 = syntax_reference_by_arguments(graph, base, left, NULL, route);
				if (r1)
				{
					symbol_t *r2;
					r2 = syntax_reference_by_arguments(graph, r1, right, arguments, SYNTAX_ROUTE_FORWARD);
					if (r2)
					{
						return r2;
					}
					else
					{
						syntax_error(graph, right, "field not found in (%lld:%lld)",
							r1->declaration->position.line, r1->declaration->position.column);
						return NULL;
					}
				}
				else
				{
					syntax_error(graph, left, "field not found");
					return NULL;
				}
			}
			else
			{
				syntax_error(graph, t1, "attribute does not include the right field");
				return NULL;
			}
		}
		else
		{
			syntax_error(graph, t1, "attribute does not include the left field");
			return NULL;
		}
	}
	else 
	if (symbol_check_type(t1, SYMBOL_TYPE_COMPOSITE))
	{
		symbol_t *name;
		name = syntax_extract_with(t1, SYMBOL_TYPE_NAME);
		if (name)
		{
			symbol_t *arguments1;
			arguments1 = syntax_extract_with(t1, SYMBOL_TYPE_ARGUMENTS);
			if (arguments1)
			{
				symbol_t *r1;
				r1 = syntax_reference_by_arguments(graph, base, name, arguments1, route);
				if (r1)
				{
					return r1;
				}
				else
				{
					syntax_error(graph, name, "field not found");
					return NULL;
				}
			}
			else
			{
				syntax_error(graph, t1, "attribute does not include the arguments field");
				return NULL;
			}
		}
		else
		{
			syntax_error(graph, t1, "attribute does not include the name field");
			return NULL;
		}
	}
	else
	if (symbol_check_type(t1, SYMBOL_TYPE_ID))
	{
		symbol_t *r;
		r = syntax_reference_in_scope(graph, base, t1, arguments, route);
		if (r)
		{
			return r;
		}
		else
		{
			syntax_error(graph, t1, "reference not found");
			return NULL;
		}
	}
	else
	{
		syntax_error(graph, t1, "the reference is not a routable");
		return NULL;
	}
}

static symbol_t *
syntax_reference(graph_t *graph, symbol_t *t1)
{
	return syntax_reference_by_arguments(graph, t1->parent, t1, NULL, SYNTAX_ROUTE_NONE);
}



static int32_t
syntax_unique_ga(graph_t *graph, symbol_t *g1, symbol_t *a1)
{
	symbol_t *gt_1;
	gt_1 = syntax_extract_with(g1, SYMBOL_TYPE_TYPE);
	if (gt_1)
	{
		symbol_t *gtr_1;
		gtr_1 = syntax_reference(graph, gt_1);
		if (gtr_1)
		{
			symbol_t *an_1;
			an_1 = syntax_extract_with(a1, SYMBOL_TYPE_NAME);
			if (an_1)
			{
				symbol_t *anr_1;
				anr_1 = syntax_reference(graph, an_1);
				if (anr_1)
				{
					return syntax_unique(graph, gtr_1, anr_1);
				}
				else
				{
					syntax_error(graph, an_1, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				syntax_error(graph, a1, "refrerence of this type not found");
				return 0;
			}
		}
		else
		{
			syntax_error(graph, g1, "refrerence of this type not found");
			return 0;
		}
	}
	else
	{
		symbol_t *gv_1;
		gv_1 = syntax_extract_with(g1, SYMBOL_TYPE_VALUE);
		if (gv_1)
		{
			symbol_t *gvr_1;
			gvr_1 = syntax_reference(graph, gv_1);
			if (gvr_1)
			{
				symbol_t *an_1;
				an_1 = syntax_extract_with(a1, SYMBOL_TYPE_NAME);
				if (an_1)
				{
					symbol_t *anr_1;
					anr_1 = syntax_reference(graph, an_1);
					if (anr_1)
					{
						return syntax_unique(graph, gvr_1, anr_1);
					}
					else
					{
						syntax_error(graph, an_1, "refrerence of this type not found");
						return 0;
					}
				}
				else
				{
					syntax_error(graph, a1, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				syntax_error(graph, gv_1, "refrerence of this type not found");
				return 0;
			}
		}
		else
		{
			return 1;
		}
	}
	return 1;
}

static int32_t
syntax_unique_gsas(graph_t *graph, symbol_t *gs1, symbol_t *as1)
{
	uint64_t gs1_cnt = 0;
	uint64_t as1_cnt = 0;

	symbol_t *a;
	for (a = gs1->begin;a != gs1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_GENERIC))
		{
			gs1_cnt += 1;
			as1_cnt = 0;

			symbol_t *b;
			for (b = as1->begin;b != as1->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_TYPE_ARGUMENT))
				{
					as1_cnt += 1;
					if (as1_cnt < gs1_cnt)
					{
						continue;
					}
					if (syntax_unique_ga(graph, a, b))
					{
						break;
					}
					return -1;
				}
			}
			
			if (as1_cnt < gs1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(a, SYMBOL_TYPE_VALUE);
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
		if (symbol_check_type(b, SYMBOL_TYPE_ARGUMENT))
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

static int32_t
syntax_unique_gg(graph_t *graph, symbol_t *g1, symbol_t *g2)
{
	symbol_t *gt_1;
	gt_1 = syntax_extract_with(g1, SYMBOL_TYPE_TYPE);
	if (gt_1)
	{
		symbol_t *gtr_1;
		gtr_1 = syntax_reference(graph, gt_1);
		if (gtr_1)
		{
			symbol_t *gt_2;
			gt_2 = syntax_extract_with(g2, SYMBOL_TYPE_TYPE);
			if (gt_2)
			{
				symbol_t *gtr_2;
				gtr_2 = syntax_reference(graph, gt_2);
				if (gtr_2)
				{
					return syntax_unique(graph, gtr_1, gtr_2);
				}
				else
				{
					syntax_error(graph, gt_2, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				symbol_t *gv_2;
				gv_2 = syntax_extract_with(g2, SYMBOL_TYPE_VALUE);
				if (gv_2)
				{
					symbol_t *gtr_2;
					gtr_2 = syntax_reference(graph, gv_2);
					if (gtr_2)
					{
						return syntax_unique(graph, gtr_1, gtr_2);
					}
					else
					{
						syntax_error(graph, gv_2, "refrerence of this type not found");
						return 0;
					}
				}
			}
		}
		else
		{
			syntax_error(graph, gt_1, "refrerence of this type not found");
			return 0;
		}
	}
	else
	{
		symbol_t *gt_2;
		gt_2 = syntax_extract_with(g2, SYMBOL_TYPE_TYPE);
		if (gt_2)
		{
			return syntax_unique_gg(graph, g2, g1);
		}
	}
	return 1;
}

static int32_t
syntax_unique_gsgs(graph_t *graph, symbol_t *gs1, symbol_t *gs2)
{
	uint64_t gs1_cnt = 0;
	uint64_t gs2_cnt = 0;

	symbol_t *a;
	for (a = gs1->begin;a != gs1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_GENERIC))
		{
			gs1_cnt += 1;
			gs2_cnt = 0;

			symbol_t *b;
			for (b = gs2->begin;b != gs2->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
				{
					gs2_cnt += 1;
					if (gs2_cnt < gs1_cnt)
					{
						continue;
					}
					if (syntax_unique_gg(graph, a, b))
					{
						break;
					}
					return -1;
				}
			}
			
			if (gs2_cnt < gs1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(a, SYMBOL_TYPE_VALUE);
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
		if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
		{
			gs2_cnt += 1;
			if (gs2_cnt > gs1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(b, SYMBOL_TYPE_VALUE);
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
syntax_unique_pp(graph_t *graph, symbol_t *p1, symbol_t *p2)
{
	symbol_t *pt1;
	pt1 = syntax_extract_with(p1, SYMBOL_TYPE_TYPE);
	if (pt1)
	{
		symbol_t *ptr1;
		ptr1 = syntax_reference(graph, pt1);
		if (ptr1)
		{
			symbol_t *pt2;
			pt2 = syntax_extract_with(p2, SYMBOL_TYPE_TYPE);
			if (pt2)
			{
				symbol_t *ptr2;
				ptr2 = syntax_reference(graph, p2);
				if (ptr2)
				{
					return syntax_unique(graph, ptr1, ptr2);
				}
				else
				{
					syntax_error(graph, pt2, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				symbol_t *pv2;
				pv2 = syntax_extract_with(p2, SYMBOL_TYPE_VALUE);
				if (pv2)
				{
					symbol_t *pvr2;
					pvr2 = syntax_reference(graph, pv2);
					if (pvr2)
					{
						return syntax_unique(graph, ptr1, pvr2);
					}
					else
					{
						syntax_error(graph, pv2, "refrerence of this type not found");
						return 0;
					}
				}
			}
		}
		else
		{
			syntax_error(graph, pt1, "refrerence of this type not found");
			return 0;
		}
	}
	else
	{
		symbol_t *pt2;
		pt2 = syntax_extract_with(p2, SYMBOL_TYPE_TYPE);
		if (pt2)
		{
			return syntax_unique_gg(graph, p2, p1);
		}
	}
	return 1;
}

static int32_t
syntax_unique_psps(graph_t *graph, symbol_t *ps1, symbol_t *ps2)
{
	uint64_t ps1_cnt = 0;
	uint64_t ps2_cnt = 0;

	symbol_t *a;
	for (a = ps1->begin;a != ps1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_PARAMETER))
		{
			ps1_cnt += 1;
			ps2_cnt = 0;

			symbol_t *b;
			for (b = ps2->begin;b != ps2->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_TYPE_PARAMETER))
				{
					ps2_cnt += 1;
					if (ps2_cnt < ps1_cnt)
					{
						continue;
					}
					if (syntax_unique_pp(graph, a, b))
					{
						break;
					}
					return -1;
				}
			}
			
			if (ps2_cnt < ps1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(a, SYMBOL_TYPE_VALUE);
				if (!value)
				{
					return -1;
				}
			}
		}
	}

	ps2_cnt = 0;
	symbol_t *b;
	for (b = ps2->begin;b != ps2->end;b = b->next)
	{
		if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
		{
			ps2_cnt += 1;
			if (ps2_cnt > ps1_cnt)
			{
				symbol_t *value;
				value = syntax_extract_with(b, SYMBOL_TYPE_VALUE);
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
syntax_unique_aa(graph_t *graph, symbol_t *a1, symbol_t *a2)
{
	symbol_t *an1;
	an1 = syntax_extract_with(a1, SYMBOL_TYPE_NAME);
	if (an1)
	{
		symbol_t *anr1;
		anr1 = syntax_reference(graph, an1);
		if (anr1)
		{
			symbol_t *an2;
			an2 = syntax_extract_with(a2, SYMBOL_TYPE_NAME);
			if (an2)
			{
				symbol_t *anr2;
				anr2 = syntax_reference(graph, an2);
				if (anr2)
				{
					return syntax_unique(graph, anr1, anr2);
				}
				else
				{
					syntax_error(graph, an2, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				syntax_error(graph, a2, "refrerence of this type not found");
				return 0;
			}
		}
		else
		{
			syntax_error(graph, a1, "refrerence of this type not found");
			return 0;
		}
	}
	else
	{
		syntax_error(graph, a1, "argument without type");
		return 0;
	}
	return 1;
}

static int32_t
syntax_unique_asas(graph_t *graph, symbol_t *as1, symbol_t *as2)
{
	uint64_t as1_cnt = 0;
	uint64_t as2_cnt = 0;

	symbol_t *a;
	for (a = as1->begin;a != as1->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_ARGUMENT))
		{
			as1_cnt += 1;
			as2_cnt = 0;

			symbol_t *b;
			for (b = as2->begin;b != as2->end;b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_TYPE_ARGUMENT))
				{
					as2_cnt += 1;
					if (as2_cnt < as1_cnt)
					{
						continue;
					}
					if (syntax_unique_aa(graph, a, b))
					{
						break;
					}
					return -1;
				}
			}
		}
	}

	as2_cnt = 0;
	symbol_t *b;
	for (b = as2->begin;b != as2->end;b = b->next)
	{
		if (symbol_check_type(b, SYMBOL_TYPE_ARGUMENT))
		{
			as2_cnt += 1;
			if (as2_cnt > as1_cnt)
			{
				return -1;
			}
		}
	}

	return 1;
}

static int32_t
syntax_unique(graph_t *graph, symbol_t *s1, symbol_t *s2)
{
	symbol_t *cn;
	cn = syntax_extract_with(s1, SYMBOL_TYPE_NAME);
	if (cn)
	{
		symbol_t *tn;
		tn = syntax_extract_with(s2, SYMBOL_TYPE_NAME);
		if (tn)
		{
			if (symbol_check_type(s1, SYMBOL_TYPE_CLASS))
			{
				if (symbol_check_type(s2, SYMBOL_TYPE_CLASS))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 1;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_TYPE))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 1;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_METHOD))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 1;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FUNCTION))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 1;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FIELD))
				{
					if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *ntn;
						ntn = syntax_extract_with(tn, SYMBOL_TYPE_NAME);
						if (ntn)
						{
							symbol_t *astn;
							astn = syntax_extract_with(tn, SYMBOL_TYPE_ARGUMENTS);
							if (astn)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									return syntax_unique_gsas(graph, gsc, astn);
								}
								return -1;
							}
							else
							{
								syntax_error(graph, tn, "does not include the arguments field");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						if (symbol_check_type(tn, SYMBOL_TYPE_ID))
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									symbol_t *b;
									for (b = gsc->begin;b != gsc->end;b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
										{
											symbol_t *bv;
											bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
											if (!bv)
											{
												return -1;
											}
										}
									}
								}
								return 1;
							}
						}
						else
						{
							syntax_error(graph, tn, "the name not an id");
							return 0;
						}
					}
				}
				else
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *b;
							for (b = gsc->begin;b != gsc->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
						}
						return 1;
					}
					return -1;
				}
			}
			else
			if (symbol_check_type(s1, SYMBOL_TYPE_TYPE))
			{
				if (symbol_check_type(s2, SYMBOL_TYPE_CLASS))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_TYPE))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_METHOD))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FUNCTION))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FIELD))
				{
					if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *ntn;
						ntn = syntax_extract_with(tn, SYMBOL_TYPE_NAME);
						if (ntn)
						{
							symbol_t *astn;
							astn = syntax_extract_with(tn, SYMBOL_TYPE_ARGUMENTS);
							if (astn)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									return syntax_unique_gsas(graph, gsc, astn);
								}
								return -1;
							}
							else
							{
								syntax_error(graph, tn, "does not include the arguments field");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						if (symbol_check_type(tn, SYMBOL_TYPE_ID))
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									symbol_t *b;
									for (b = gsc->begin;b != gsc->end;b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
										{
											symbol_t *bv;
											bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
											if (!bv)
											{
												return -1;
											}
										}
									}
								}
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "the name not an id");
							return 0;
						}
					}
				}
				else
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *b;
							for (b = gsc->begin;b != gsc->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
						}
						return 1;
					}
					return -1;
				}
			}
			else
			if (symbol_check_type(s1, SYMBOL_TYPE_METHOD))
			{
				if (symbol_check_type(s2, SYMBOL_TYPE_CLASS))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_TYPE))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_METHOD))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								r = syntax_unique_gsgs(graph, gsc, gst);
								if (r > 0)
								{
									symbol_t *psc;
									psc = syntax_extract_with(s1, SYMBOL_TYPE_PARAMETERS);
									if (psc)
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											return syntax_unique_psps(graph, psc, pst);
										}
										return -1;
									}
									else
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											if (symbol_count(pst) > 0)
											{
												return -1;
											}
										}
										return 1;
									}
								}
								return r;
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FUNCTION))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								r = syntax_unique_gsgs(graph, gsc, gst);
								if (r > 0)
								{
									symbol_t *psc;
									psc = syntax_extract_with(s1, SYMBOL_TYPE_PARAMETERS);
									if (psc)
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											return syntax_unique_psps(graph, psc, pst);
										}
										return -1;
									}
									else
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											if (symbol_count(pst) > 0)
											{
												return -1;
											}
										}
										return 1;
									}
								}
								return r;
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FIELD))
				{
					if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *ntn;
						ntn = syntax_extract_with(tn, SYMBOL_TYPE_NAME);
						if (ntn)
						{
							symbol_t *astn;
							astn = syntax_extract_with(tn, SYMBOL_TYPE_ARGUMENTS);
							if (astn)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									return syntax_unique_gsas(graph, gsc, astn);
								}
								return -1;
							}
							else
							{
								syntax_error(graph, tn, "does not include the arguments field");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						if (symbol_check_type(tn, SYMBOL_TYPE_ID))
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									symbol_t *b;
									for (b = gsc->begin;b != gsc->end;b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
										{
											symbol_t *bv;
											bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
											if (!bv)
											{
												return -1;
											}
										}
									}
								}
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "the name not an id");
							return 0;
						}
					}
				}
				else
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *b;
							for (b = gsc->begin;b != gsc->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
						}
						return 1;
					}
					return -1;
				}
			}
			else
			if (symbol_check_type(s1, SYMBOL_TYPE_FUNCTION))
			{
				if (symbol_check_type(s2, SYMBOL_TYPE_CLASS))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_TYPE))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								return syntax_unique_gsgs(graph, gsc, gst);
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_METHOD))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								r = syntax_unique_gsgs(graph, gsc, gst);
								if (r > 0)
								{
									symbol_t *psc;
									psc = syntax_extract_with(s1, SYMBOL_TYPE_PARAMETERS);
									if (psc)
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											return syntax_unique_psps(graph, psc, pst);
										}
										return -1;
									}
									else
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											if (symbol_count(pst) > 0)
											{
												return -1;
											}
										}
										return 1;
									}
								}
								return r;
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FUNCTION))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								r = syntax_unique_gsgs(graph, gsc, gst);
								if (r > 0)
								{
									symbol_t *psc;
									psc = syntax_extract_with(s1, SYMBOL_TYPE_PARAMETERS);
									if (psc)
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											return syntax_unique_psps(graph, psc, pst);
										}
										return -1;
									}
									else
									{
										symbol_t *pst;
										pst = syntax_extract_with(s2, SYMBOL_TYPE_PARAMETERS);
										if (pst)
										{
											if (symbol_count(pst) > 0)
											{
												return -1;
											}
										}
										return 1;
									}
								}
								return r;
							}
							else
							{
								symbol_t *b;
								for (b = gsc->begin;b != gsc->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
								return 0;
							}
						}
						else
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 0;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FIELD))
				{
					if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *ntn;
						ntn = syntax_extract_with(tn, SYMBOL_TYPE_NAME);
						if (ntn)
						{
							symbol_t *astn;
							astn = syntax_extract_with(tn, SYMBOL_TYPE_ARGUMENTS);
							if (astn)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									return syntax_unique_gsas(graph, gsc, astn);
								}
								return -1;
							}
							else
							{
								syntax_error(graph, tn, "does not include the arguments field");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						if (symbol_check_type(tn, SYMBOL_TYPE_ID))
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								symbol_t *gsc;
								gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
								if (gsc)
								{
									symbol_t *b;
									for (b = gsc->begin;b != gsc->end;b = b->next)
									{
										if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
										{
											symbol_t *bv;
											bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
											if (!bv)
											{
												return -1;
											}
										}
									}
								}
								return 0;
							}
						}
						else
						{
							syntax_error(graph, tn, "the name not an id");
							return 0;
						}
					}
				}
				else
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *b;
							for (b = gsc->begin;b != gsc->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
						}
						return 1;
					}
					return -1;
				}
			}
			else
			if (symbol_check_type(s1, SYMBOL_TYPE_FIELD))
			{
				if (symbol_check_type(s2, SYMBOL_TYPE_FIELD))
				{
					if (symbol_check_type(cn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *cnn;
						cnn = syntax_extract_with(cn, SYMBOL_TYPE_NAME);
						if (cnn)
						{
							if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
							{
								symbol_t *tnn;
								tnn = syntax_extract_with(tn, SYMBOL_TYPE_NAME);
								if (tnn)
								{
									if (symbol_check_type(cnn, SYMBOL_TYPE_ID))
									{
										if (symbol_check_type(tnn, SYMBOL_TYPE_ID))
										{
											int32_t r;
											r = syntax_comparison_id(cn, tn);
											if (r)
											{
												symbol_t *cnna;
												cnna = syntax_only_with(cnn, SYMBOL_TYPE_ARGUMENTS);
												if (cnna)
												{
													symbol_t *tnna;
													tnna = syntax_only_with(tnn, SYMBOL_TYPE_ARGUMENTS);
													if (tnna)
													{
														return syntax_unique_asas(graph, cnna, tnna);
													}
													else
													{
														syntax_error(graph, tnna, "missing arguments");
														return 0;
													}
												}
												else
												{
													syntax_error(graph, cnna, "missing arguments");
													return 0;
												}
											}
											return -1;
										}
										else
										{
											syntax_error(graph, tnn, "the name not an id");
											return 0;
										}
									}
									else
									{
										syntax_error(graph, cnn, "the name not an id");
										return 0;
									}
								}
								else
								{
									syntax_error(graph, tn, "does not include the name field");
									return 0;
								}
							}
							else
							{
								return -1;
							}
						}
						else
						{
							syntax_error(graph, cn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
						{
							return -1;
						}
						else
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								return 1;
							}
							return -1;
						}
					}
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_CLASS))
				{
					if (symbol_check_type(cn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *cnn;
						cnn = syntax_extract_with(cn, SYMBOL_TYPE_NAME);
						if (cnn)
						{
							if (symbol_check_type(cnn, SYMBOL_TYPE_ID))
							{
								int32_t r;
								r = syntax_comparison_id(cnn, tn);
								if (r)
								{
									symbol_t *cnna;
									cnna = syntax_only_with(cnn, SYMBOL_TYPE_ARGUMENTS);
									if (cnna)
									{
										symbol_t *gst;
										gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
										if (gst)
										{
											return syntax_unique_gsas(graph, gst, cnna);
										}
										else
										{
											return -1;
										}
									}
									else
									{
										syntax_error(graph, cnna, "missing arguments");
										return 0;
									}
								}
								return -1;
							}
							else
							{
								syntax_error(graph, cnn, "the name not an id");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, cn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						int32_t r;
						r = syntax_comparison_id(cn, tn);
						if (r)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
						return -1;
					}
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_TYPE))
				{
					if (symbol_check_type(cn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *cnn;
						cnn = syntax_extract_with(cn, SYMBOL_TYPE_NAME);
						if (cnn)
						{
							if (symbol_check_type(cnn, SYMBOL_TYPE_ID))
							{
								int32_t r;
								r = syntax_comparison_id(cnn, tn);
								if (r)
								{
									symbol_t *cnna;
									cnna = syntax_only_with(cnn, SYMBOL_TYPE_ARGUMENTS);
									if (cnna)
									{
										symbol_t *gst;
										gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
										if (gst)
										{
											return syntax_unique_gsas(graph, gst, cnna);
										}
										else
										{
											return -1;
										}
									}
									else
									{
										syntax_error(graph, cnna, "missing arguments");
										return 0;
									}
								}
								return -1;
							}
							else
							{
								syntax_error(graph, cnn, "the name not an id");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, cn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						int32_t r;
						r = syntax_comparison_id(cn, tn);
						if (r)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
						return -1;
					}
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_METHOD))
				{
					if (symbol_check_type(cn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *cnn;
						cnn = syntax_extract_with(cn, SYMBOL_TYPE_NAME);
						if (cnn)
						{
							if (symbol_check_type(cnn, SYMBOL_TYPE_ID))
							{
								int32_t r;
								r = syntax_comparison_id(cnn, tn);
								if (r)
								{
									symbol_t *cnna;
									cnna = syntax_only_with(cnn, SYMBOL_TYPE_ARGUMENTS);
									if (cnna)
									{
										symbol_t *gst;
										gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
										if (gst)
										{
											return syntax_unique_gsas(graph, gst, cnna);
										}
										else
										{
											return -1;
										}
									}
									else
									{
										syntax_error(graph, cnna, "missing arguments");
										return 0;
									}
								}
								return -1;
							}
							else
							{
								syntax_error(graph, cnn, "the name not an id");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, cn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						int32_t r;
						r = syntax_comparison_id(cn, tn);
						if (r)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
						return -1;
					}
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FUNCTION))
				{
					if (symbol_check_type(cn, SYMBOL_TYPE_COMPOSITE))
					{
						symbol_t *cnn;
						cnn = syntax_extract_with(cn, SYMBOL_TYPE_NAME);
						if (cnn)
						{
							if (symbol_check_type(cnn, SYMBOL_TYPE_ID))
							{
								int32_t r;
								r = syntax_comparison_id(cnn, tn);
								if (r)
								{
									symbol_t *cnna;
									cnna = syntax_only_with(cnn, SYMBOL_TYPE_ARGUMENTS);
									if (cnna)
									{
										symbol_t *gst;
										gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
										if (gst)
										{
											return syntax_unique_gsas(graph, gst, cnna);
										}
										else
										{
											return -1;
										}
									}
									else
									{
										syntax_error(graph, cnna, "missing arguments");
										return 0;
									}
								}
								return -1;
							}
							else
							{
								syntax_error(graph, cnn, "the name not an id");
								return 0;
							}
						}
						else
						{
							syntax_error(graph, cn, "does not include the name field");
							return 0;
						}
					}
					else
					{
						int32_t r;
						r = syntax_comparison_id(cn, tn);
						if (r)
						{
							symbol_t *gst;
							gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
							if (gst)
							{
								symbol_t *b;
								for (b = gst->begin;b != gst->end;b = b->next)
								{
									if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
									{
										symbol_t *bv;
										bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
										if (!bv)
										{
											return -1;
										}
									}
								}
							}
							return 1;
						}
						return -1;
					}
				}
				else
				{
					if (symbol_check_type(cn, SYMBOL_TYPE_COMPOSITE))
					{
						return -1;
					}
					else
					{
						if (symbol_check_type(cn, SYMBOL_TYPE_ID))
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								return 1;
							}
							return -1;
						}
						else
						{
							syntax_error(graph, cn, "the name not an id");
							return 0;
						}
					}
				}
			}
			else
			{
				if (symbol_check_type(s2, SYMBOL_TYPE_CLASS))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gst;
						gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
						if (gst)
						{
							symbol_t *b;
							for (b = gst->begin;b != gst->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
							return 1;
						}
						else
						{
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_TYPE))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gst;
						gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
						if (gst)
						{
							symbol_t *b;
							for (b = gst->begin;b != gst->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
							return 1;
						}
						else
						{
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_METHOD))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gst;
						gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
						if (gst)
						{
							symbol_t *b;
							for (b = gst->begin;b != gst->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
							return 1;
						}
						else
						{
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FUNCTION))
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gst;
						gst = syntax_only_with(s2, SYMBOL_TYPE_GENERICS);
						if (gst)
						{
							symbol_t *b;
							for (b = gst->begin;b != gst->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
							return 1;
						}
						else
						{
							return 1;
						}
					}
					return -1;
				}
				else
				if (symbol_check_type(s2, SYMBOL_TYPE_FIELD))
				{
					if (symbol_check_type(tn, SYMBOL_TYPE_COMPOSITE))
					{
						return -1;
					}
					else
					{
						if (symbol_check_type(tn, SYMBOL_TYPE_ID))
						{
							int32_t r;
							r = syntax_comparison_id(cn, tn);
							if (r)
							{
								return 1;
							}
							return -1;
						}
						else
						{
							syntax_error(graph, tn, "the name not an id");
							return 0;
						}
					}
				}
				else
				{
					int32_t r;
					r = syntax_comparison_id(cn, tn);
					if (r)
					{
						symbol_t *gsc;
						gsc = syntax_only_with(s1, SYMBOL_TYPE_GENERICS);
						if (gsc)
						{
							symbol_t *b;
							for (b = gsc->begin;b != gsc->end;b = b->next)
							{
								if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
								{
									symbol_t *bv;
									bv = syntax_only_with(b, SYMBOL_TYPE_VALUE);
									if (!bv)
									{
										return -1;
									}
								}
							}
						}
						return 1;
					}
					return -1;
				}
			}
		}
		else
		{
			syntax_error(graph, s2, "does not include the name field");
			return 0;
		}
	}
	else
	{
		syntax_error(graph, s1, "does not include the name field");
		return 0;
	}
}



static int32_t
syntax_expression(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_assign(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_return(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(graph, a);
		if(!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_continue(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(graph, a);
		if(!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_break(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(graph, a);
		if(!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_throw(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_expression(graph, a);
		if(!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_var(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_if(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_CONDITION))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_ELSE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_TYPE_IF))
				{
					int32_t result;
					result = syntax_if(graph, b);
					if(!result)
					{
						return 0;
					}
				}
				else
				{
					int32_t result;
					result = syntax_block(graph, b);
					if(!result)
					{
						return 0;
					}
				}
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_catch(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_try(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_CATCHS))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(b, SYMBOL_TYPE_CATCH))
				{
					int32_t result;
					result = syntax_catch(graph, b);
					if(!result)
					{
						return 0;
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
syntax_for(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_TYPE_VAR))
				{
					int32_t result;
					result = syntax_var(graph, b);
					if(!result)
					{
						return 0;
					}
					continue;
				}
				if (symbol_check_type(a, SYMBOL_TYPE_ASSIGN))
				{
					int32_t result;
					result = syntax_assign(graph, b);
					if(!result)
					{
						return 0;
					}
					continue;
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_CONDITION))
		{
			int32_t result;
			result = syntax_expression(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_INCREMENTOR))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_TYPE_ASSIGN))
				{
					int32_t result;
					result = syntax_assign(graph, b);
					if(!result)
					{
						return 0;
					}
					continue;
				}
			}
		}
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_forin(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_TYPE_VAR))
				{
					int32_t result;
					result = syntax_var(graph, b);
					if(!result)
					{
						return 0;
					}
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_EXPRESSION))
		{
			int32_t result;
			result = syntax_expression(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_statement(graph_t *graph, symbol_t *current)
{
	int32_t result = 1;

	if (symbol_check_type(current, SYMBOL_TYPE_BLOCK))
	{
		result = syntax_block(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_IF))
	{
		result = syntax_if(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_TRY))
	{
		result = syntax_try(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_FOR))
	{
		result = syntax_for(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_FORIN))
	{
		result = syntax_forin(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_FUNCTION))
	{
		result = syntax_function(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_CONTINUE))
	{
		result = syntax_continue(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_BREAK))
	{
		result = syntax_break(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_RETURN))
	{
		result = syntax_return(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_THROW))
	{
		result = syntax_throw(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TYPE_VAR))
	{
		result = syntax_var(graph, current);
	}
	else
	{
		result = syntax_assign(graph, current);
	}

	return result;
}

static int32_t
syntax_block(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end);a = a->next)
	{
		int32_t result;
		result = syntax_statement(graph, a);
		if(!result)
		{
			return 0;
		}
	}
	return 1;
}



static int32_t
syntax_generic(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_generics(graph_t *graph, symbol_t *current)
{
	
	return 1;
}

static int32_t
syntax_parameter(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_parameters(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_parameter(graph, a);
		if(!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_heritage(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_heritages(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_heritage(graph, a);
		if(!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_method(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_GENERICS))
		{
			int32_t result;
			result = syntax_generics(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_member(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_enum(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		int32_t result;
		result = syntax_member(graph, a);
		if(!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_property(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_class(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_GENERICS))
		{
			int32_t result;
			result = syntax_generics(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_PROPERTY))
		{
			int32_t result;
			result = syntax_property(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_CLASS))
		{
			int32_t result;
			result = syntax_class(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_ENUM))
		{
			int32_t result;
			result = syntax_enum(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_METHOD))
		{
			int32_t result;
			result = syntax_method(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_type(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_GENERICS))
		{
			int32_t result;
			result = syntax_generics(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_function(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_GENERICS))
		{
			int32_t result;
			result = syntax_generics(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}

	return 1;
}

static int32_t
syntax_field(graph_t *graph, symbol_t *current)
{
	return 1;
}

static int32_t
syntax_fields(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		
	}
	return 1;
}

static int32_t
syntax_import_name(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *name)
{
	symbol_t *na;
	na = syntax_only_with(name, SYMBOL_TYPE_ID);
	if (na)
	{
		symbol_t *b;
		for(b = root->begin; (b != root->end) && (b != subroot); b = b->next)
		{
			if (symbol_check_type(b, SYMBOL_TYPE_IMPORT))
			{
				symbol_t *nb;
				nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
				if (nb)
				{
					if (syntax_comparison_id(na, nb))
					{
						syntax_error(graph, na, "already defined, previous at %lld:%lld", 
							nb->declaration->position.line, nb->declaration->position.column);
						return 0;
					}
				}

				symbol_t *fsb;
				fsb = syntax_only_with(b, SYMBOL_TYPE_FIELDS);
				if (fsb)
				{
					symbol_t *c;
					for(c = fsb->begin; c != fsb->end; c = c->next)
					{
						if (symbol_check_type(c, SYMBOL_TYPE_FIELD))
						{
							symbol_t *nc;
							nc = syntax_extract_with(c, SYMBOL_TYPE_NAME);
							if (nc)
							{
								if (symbol_check_type(nc, SYMBOL_TYPE_ID))
								{
									if (syntax_comparison_id(na, nc))
									{
										syntax_error(graph, na, "already defined, previous at %lld:%lld", 
											nc->declaration->position.line, nc->declaration->position.column);
										return 0;
									}
								}
							}
						}
					}
				}
				continue;
			}

			if (symbol_check_type(b, SYMBOL_TYPE_CLASS))
			{
				symbol_t *nb;
				nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
				if (nb)
				{
					if (syntax_comparison_id(na, nb))
					{
						syntax_error(graph, na, "already defined, previous at %lld:%lld", 
							nb->declaration->position.line, nb->declaration->position.column);
						return 0;
					}
				}

				symbol_t *gsb;
				gsb = syntax_only_with(b, SYMBOL_TYPE_GENERICS);
				if (gsb)
				{
					symbol_t *c;
					for(c = gsb->begin; c != gsb->end; c = c->next)
					{
						if (symbol_check_type(c, SYMBOL_TYPE_GENERIC))
						{
							symbol_t *vc;
							vc = syntax_extract_with(c, SYMBOL_TYPE_VALUE);
							if (vc)
							{
								syntax_error(graph, na, "already defined, previous at %lld:%lld", 
									nb->declaration->position.line, nb->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}

			if (symbol_check_type(b, SYMBOL_TYPE_TYPE))
			{
				symbol_t *nb;
				nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
				if (nb)
				{
					if (syntax_comparison_id(na, nb))
					{
						syntax_error(graph, na, "already defined, previous at %lld:%lld", 
							nb->declaration->position.line, nb->declaration->position.column);
						return 0;
					}
				}

				symbol_t *gsb;
				gsb = syntax_only_with(b, SYMBOL_TYPE_GENERICS);
				if (gsb)
				{
					symbol_t *c;
					for(c = gsb->begin; c != gsb->end; c = c->next)
					{
						if (symbol_check_type(c, SYMBOL_TYPE_GENERIC))
						{
							symbol_t *vc;
							vc = syntax_extract_with(c, SYMBOL_TYPE_VALUE);
							if (vc)
							{
								syntax_error(graph, na, "already defined, previous at %lld:%lld", 
									nb->declaration->position.line, nb->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}

			if (symbol_check_type(b, SYMBOL_TYPE_FUNCTION))
			{
				symbol_t *nb;
				nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
				if (nb)
				{
					if (syntax_comparison_id(na, nb))
					{
						syntax_error(graph, na, "already defined, previous at %lld:%lld", 
							nb->declaration->position.line, nb->declaration->position.column);
						return 0;
					}
				}

				symbol_t *gsb;
				gsb = syntax_only_with(b, SYMBOL_TYPE_GENERICS);
				if (gsb)
				{
					symbol_t *c;
					for(c = gsb->begin; c != gsb->end; c = c->next)
					{
						if (symbol_check_type(c, SYMBOL_TYPE_GENERIC))
						{
							symbol_t *vc;
							vc = syntax_extract_with(c, SYMBOL_TYPE_VALUE);
							if (vc)
							{
								syntax_error(graph, na, "already defined, previous at %lld:%lld", 
									nb->declaration->position.line, nb->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}

			if (symbol_check_type(b, SYMBOL_TYPE_VAR))
			{
				symbol_t *nb;
				nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
				if (nb)
				{
					if (syntax_comparison_id(na, nb))
					{
						syntax_error(graph, na, "already defined, previous at %lld:%lld", 
							nb->declaration->position.line, nb->declaration->position.column);
						return 0;
					}
				}
				continue;
			}
		}

		if (root->parent)
		{
			return syntax_import_name(graph, root->parent, root, name);
		}

		return 1;
	}

	syntax_error(graph, name, "import name not an id");
	return 0;
}

static int32_t
syntax_import_generics(graph_t *graph, symbol_t *gs)
{
	symbol_t *b;
	for (b = gs->begin;(b != gs->end); b = b->next)
	{
		symbol_t *nb;
		nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
		if (nb)
		{
			symbol_t *c;
			for (c = gs->begin;(b != c) && (c != gs->end); c = c->next)
			{
				symbol_t *nc;
				nc = syntax_extract_with(c, SYMBOL_TYPE_NAME);
				if (nc)
				{
					if (syntax_comparison_id(nb, nc))
					{
						syntax_error(graph, nb, "already defined, previous at %lld:%lld", 
							nc->declaration->position.line, nc->declaration->position.column);
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

static int32_t
syntax_field_single(graph_t *graph, symbol_t *import, symbol_t *name)
{
	if (symbol_check_type(name, SYMBOL_TYPE_COMPOSITE))
	{
		symbol_t *nn;
		nn = syntax_only_with(name, SYMBOL_TYPE_NAME);
		if (nn)
		{
			if (symbol_check_type(nn, SYMBOL_TYPE_COMPOSITE))
			{
				syntax_error(graph, nn, "not a valid alias for field");
				return 0;
			}

			int32_t result;
			result = syntax_field_single(graph, import, nn);
			if (result)
			{
				symbol_t *as;
				as = syntax_only_with(name, SYMBOL_TYPE_ARGUMENTS);
				if (as)
				{
					symbol_t *a;
					for (a = as->begin; a != as->end; a = a->next)
					{
						if (symbol_check_type(a, SYMBOL_TYPE_ARGUMENT))
						{
							symbol_t *va;
							va = syntax_extract_with(a, SYMBOL_TYPE_VALUE);
							if (va)
							{

							}
							else
							{
								symbol_t *na;
								na = syntax_extract_with(a, SYMBOL_TYPE_NAME);
								if (na)
								{
									if (symbol_check_type(na, SYMBOL_TYPE_ID))
									{
										symbol_t *gs;
										gs = syntax_only_with(import, SYMBOL_TYPE_GENERICS);
										if (gs)
										{
											symbol_t *b;
											for (b = gs->begin; b != gs->end; b = b->next)
											{
												if (symbol_check_type(b, SYMBOL_TYPE_GENERIC))
												{
													symbol_t *nb;
													nb = syntax_extract_with(b, SYMBOL_TYPE_NAME);
													if (nb)
													{
														if (syntax_comparison_id(na, nb))
														{

														}
													}
												}
											}
										}
									}
									else
									{
										syntax_error(graph, a, "argument not an type");
										return 0;
									}
								}

							}
						}
					}
				}

				return 1;
			}
			
			return 0;
		}
		syntax_error(graph, name, "composite without name for field!");
		return 0;
	}

	if (symbol_check_type(name, SYMBOL_TYPE_ATTR))
	{
		syntax_error(graph, name, "attribute is used in alias of field");
		return 0;
	}

	if (symbol_check_type(name, SYMBOL_TYPE_ID))
	{
		return 1;
	}

	syntax_error(graph, name, "not a valid alias for field");
	return 0;
}

static int32_t
syntax_import_fields(graph_t *graph, symbol_t *import, symbol_t *fields)
{
	symbol_t *a;
	for(a = fields->begin; a != fields->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_FIELD))
		{
			symbol_t *tf;
			tf = syntax_only_with(a, SYMBOL_TYPE_TYPE);
			if (tf)
			{
				
			}
			else
			{
				symbol_t *nf;
				nf = syntax_only_with(a, SYMBOL_TYPE_NAME);
				if (nf)
				{
					int32_t result;
					result = syntax_field_single(graph, import, nf);
					if (!result)
					{
						return 0;
					}
					continue;
				}
				
			}
		}
	}
}

static int32_t
syntax_import(graph_t *graph, symbol_t *import)
{
	symbol_t *a;
	for(a = import->begin; a != import->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_NAME))
		{
			int32_t result;
			result = syntax_import_name(graph, import->parent, import, a);
			if (!result)
			{
				return 0;
			}
			continue;
		}

		if (symbol_check_type(a, SYMBOL_TYPE_GENERICS))
		{
			int32_t result;
			result = syntax_import_generics(graph, a);
			if (!result)
			{
				return 0;
			}
			continue;
		}

		if (symbol_check_type(a, SYMBOL_TYPE_FIELDS))
		{
			int32_t result;
			result = syntax_import_fields(graph, import, a);
			if (!result)
			{
				return 0;
			}
			continue;
		}
	}

	symbol_set_flag(import, SYMBOL_FLAG_SYNTAX);

	return 1;
}


static int32_t
syntax_module(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_IMPORT))
		{
			int32_t result;
			result = syntax_import(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_CLASS))
		{
			int32_t result;
			result = syntax_class(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_ENUM))
		{
			int32_t result;
			result = syntax_enum(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_TYPE))
		{
			int32_t result;
			result = syntax_type(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_VAR))
		{
			int32_t result;
			result = syntax_var(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_TYPE_FUNCTION))
		{
			int32_t result;
			result = syntax_function(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
	}

	symbol_set_flag(current, SYMBOL_FLAG_SYNTAX);
	return 1;
}

int32_t
syntax_run(graph_t *graph)
{
	symbol_t *symbol = (symbol_t *)graph->symbol;

	symbol_t *a;
	for(a = symbol->begin; a != symbol->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_TYPE_MODULE))
		{
			int32_t result;
			result = syntax_module(graph, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return 1;
}


