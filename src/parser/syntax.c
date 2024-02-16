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
syntax_type_of(graph_t *graph, symbol_t *t1);

static int32_t
syntax_ga(graph_t *graph, symbol_t *g1, symbol_t *a1)
{
	symbol_t *gt_1;
	gt_1 = syntax_extract_with(g1, SYMBOL_TYPE);
	if (gt_1)
	{
		symbol_t *gtr_1;
		gtr_1 = syntax_type_of(graph, gt_1);
		if (gtr_1)
		{
			symbol_t *an_1;
			an_1 = syntax_extract_with(a1, SYMBOL_KEY);
			if (an_1)
			{
				symbol_t *anr_1;
				anr_1 = syntax_type_of(graph, an_1);
				if (anr_1)
				{
					return (gtr_1 == anr_1) ? 1 : -1;
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
	return 1;
}

static int32_t
syntax_gsas(graph_t *graph, symbol_t *gs1, symbol_t *as1)
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
					result = syntax_ga(graph, a, b);
					if (result == 1)
					{
						return 1;
					}
					else 
					if (result == 0)
					{
						return 0;
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
syntax_type_of_in_scope(graph_t *graph, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route)
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
				if (syntax_comparison_id(ak, t1))
				{
					symbol_t *gs;
					gs = syntax_only_with(a, SYMBOL_GENERICS);
					if (gs)
					{
						if (arguments)
						{
							if (syntax_gsas(graph, gs, arguments))
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
				syntax_error(graph, a, "does not include the key field");
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
						if (syntax_comparison_id(bk, t1) && (bk != t1) && !arguments)
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
						syntax_error(graph, a, "private access");
						return NULL;
					}
				}
			}
		}
		return a;
	}

	if (base->parent && (route == (route & SYNTAX_ROUTE_NONE)))
	{
		return syntax_type_of_in_scope(graph, base->parent, t1, arguments, route);
	}

	return NULL;
}

static symbol_t *
syntax_type_of_by_arguments(graph_t *graph, symbol_t *base, symbol_t *t1, symbol_t *arguments, int32_t route)
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
				r1 = syntax_type_of_by_arguments(graph, base, left, NULL, route);
				if (r1)
				{
					symbol_t *r2;
					r2 = syntax_type_of_by_arguments(graph, r1, right, arguments, SYNTAX_ROUTE_FORWARD);
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
				r1 = syntax_type_of_by_arguments(graph, base, key, arguments1, route);
				if (r1)
				{
					return r1;
				}
				else
				{
					syntax_error(graph, key, "field not found");
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
			syntax_error(graph, t1, "attribute does not include the key field");
			return NULL;
		}
	}
	else
	if (symbol_check_type(t1, SYMBOL_ID))
	{
		symbol_t *r;
		r = syntax_type_of_in_scope(graph, base, t1, arguments, route);
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
		syntax_error(graph, t1, "the reference is not a routable");
		return NULL;
	}
}

static symbol_t *
syntax_type_of(graph_t *graph, symbol_t *t1)
{
	return syntax_type_of_by_arguments(graph, t1->parent, t1, NULL, SYNTAX_ROUTE_NONE);
}

static int32_t
syntax_gg(graph_t *graph, symbol_t *g1, symbol_t *g2)
{
	symbol_t *gt_1;
	gt_1 = syntax_extract_with(g1, SYMBOL_TYPE);
	if (gt_1)
	{
		symbol_t *gtr_1;
		gtr_1 = syntax_type_of(graph, gt_1);
		if (gtr_1)
		{
			symbol_t *gt_2;
			gt_2 = syntax_extract_with(g2, SYMBOL_TYPE);
			if (gt_2)
			{
				symbol_t *gtr_2;
				gtr_2 = syntax_type_of(graph, gt_2);
				if (gtr_2)
				{
					return (gtr_1 == gtr_2) ? 1 : -1;
				}
				else
				{
					syntax_error(graph, gt_2, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				return 1;
			}
		}
		else
		{
			syntax_error(graph, gt_1, "refrerence of this type not found");
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_gsgs(graph_t *graph, symbol_t *gs1, symbol_t *gs2)
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
					result = syntax_gg(graph, a, b);
					if (result == 1)
					{
						return 1;
					}
					else 
					if (result == 0)
					{
						return 0;
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
syntax_pp(graph_t *graph, symbol_t *p1, symbol_t *p2)
{
	symbol_t *pt1;
	pt1 = syntax_extract_with(p1, SYMBOL_TYPE);
	if (pt1)
	{
		symbol_t *ptr1;
		ptr1 = syntax_type_of(graph, pt1);
		if (ptr1)
		{
			symbol_t *pt2;
			pt2 = syntax_extract_with(p2, SYMBOL_TYPE);
			if (pt2)
			{
				symbol_t *ptr2;
				ptr2 = syntax_type_of(graph, pt2);
				if (ptr2)
				{
					return (ptr1 == ptr2) ? 1 : -1;
				}
				else
				{
					syntax_error(graph, pt2, "refrerence of this type not found");
					return 0;
				}
			}
			else
			{
				return 1;
			}
		}
		else
		{
			syntax_error(graph, pt1, "refrerence of this type not found");
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_psps(graph_t *graph, symbol_t *ps1, symbol_t *ps2)
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
					if (syntax_pp(graph, a, b))
					{
						break;
					}
					return -1;
				}
			}
			
			if (ps2_cnt < ps1_cnt)
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
				value = syntax_extract_with(b, SYMBOL_VALUE);
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
syntax_gs_v(graph_t *graph, symbol_t *gs)
{
	symbol_t *a;
	for (a = gs->begin;a != gs->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_GENERIC))
		{
			symbol_t *av;
			av = syntax_extract_with(a, SYMBOL_VALUE);
			if (!av)
			{
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_ps_v(graph_t *graph, symbol_t *ps)
{
	symbol_t *a;
	for (a = ps->begin;a != ps->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PARAMETER))
		{
			symbol_t *av;
			av = syntax_extract_with(a, SYMBOL_VALUE);
			if (!av)
			{
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_unique(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *s)
{
	symbol_t *a;
	for (a = root->begin;(a != root->end) && (a != subroot);a = a->next)
	{
		if (symbol_check_type(s, SYMBOL_CLASS))
		{
			if (symbol_check_type(a, SYMBOL_CLASS))
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_gsgs(graph, sgs, ags);
									if (result == 1)
									{
										syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
											ak->declaration->position.line, ak->declaration->position.column);
										return 0;
									}
									else
									if (result == 0)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(graph, sk, "the class is redefined1, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
							else
							{
								syntax_error(graph, sk, "the class is redefined2, previous at %lld:%lld", 
									ak->declaration->position.line, ak->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}
			else
			if (symbol_check_type(a, SYMBOL_FUNCTION))
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_gsgs(graph, sgs, ags);
									if (result == 1)
									{
										syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
											ak->declaration->position.line, ak->declaration->position.column);
										return 0;
									}
									else
									if (result == 0)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(graph, sk, "the class is redefined1, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
							else
							{
								syntax_error(graph, sk, "the class is redefined2, previous at %lld:%lld", 
									ak->declaration->position.line, ak->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}
			else
			if (symbol_check_type(a, SYMBOL_GENERICS))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			if (symbol_check_type(a, SYMBOL_HERITAGES))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			if (symbol_check_type(a, SYMBOL_PARAMETERS))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								if (syntax_gs_v(graph, sgs))
								{
									syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
						}
					}
				}
				continue;
			}
		}
		else
		if (symbol_check_type(s, SYMBOL_FUNCTION))
		{
			if (symbol_check_type(a, SYMBOL_CLASS))
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_gsgs(graph, sgs, ags);
									if (result == 1)
									{
										syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
											ak->declaration->position.line, ak->declaration->position.column);
										return 0;
									}
									else
									if (result == 0)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(graph, sk, "the class is redefined1, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
							else
							{
								syntax_error(graph, sk, "the class is redefined2, previous at %lld:%lld", 
									ak->declaration->position.line, ak->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}
			else
			if (symbol_check_type(a, SYMBOL_FUNCTION))
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_gsgs(graph, sgs, ags);
									if (result == 1)
									{
										symbol_t *sps;
										sps = syntax_only_with(s, SYMBOL_PARAMETERS);
										if (sps)
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												result = syntax_psps(graph, sps, aps);
												if (result == 1)
												{
													syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
												else
												if (result == 0)
												{
													return 0;
												}
											}
											else
											{
												if (syntax_ps_v(graph, sps))
												{
													syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
											}
										}
										else
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												if (syntax_ps_v(graph, aps))
												{
													syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
											}
											else
											{
												syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
													ak->declaration->position.line, ak->declaration->position.column);
												return 0;
											}
										}
									}
									else
									if (result == 0)
									{
										return 0;
									}
								}
								else
								{
									if (syntax_gs_v(graph, sgs))
									{
										symbol_t *sps;
										sps = syntax_only_with(s, SYMBOL_PARAMETERS);
										if (sps)
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t result;
												result = syntax_psps(graph, sps, aps);
												if (result == 1)
												{
													syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
												else
												if (result == 0)
												{
													return 0;
												}
											}
											else
											{
												if (syntax_ps_v(graph, sps))
												{
													syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
											}
										}
										else
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												if (syntax_ps_v(graph, aps))
												{
													syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
											}
											else
											{
												syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
													ak->declaration->position.line, ak->declaration->position.column);
												return 0;
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
									if (syntax_gs_v(graph, ags))
									{
										symbol_t *sps;
										sps = syntax_only_with(s, SYMBOL_PARAMETERS);
										if (sps)
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												int32_t result;
												result = syntax_psps(graph, sps, aps);
												if (result == 1)
												{
													syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
												else
												if (result == 0)
												{
													return 0;
												}
											}
											else
											{
												if (syntax_ps_v(graph, sps))
												{
													syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
											}
										}
										else
										{
											symbol_t *aps;
											aps = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (aps)
											{
												if (syntax_ps_v(graph, aps))
												{
													syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
														ak->declaration->position.line, ak->declaration->position.column);
													return 0;
												}
											}
											else
											{
												syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
													ak->declaration->position.line, ak->declaration->position.column);
												return 0;
											}
										}
									}
								}
								else
								{
									symbol_t *sps;
									sps = syntax_only_with(s, SYMBOL_PARAMETERS);
									if (sps)
									{
										symbol_t *aps;
										aps = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (aps)
										{
											int32_t result;
											result = syntax_psps(graph, sps, aps);
											if (result == 1)
											{
												syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
													ak->declaration->position.line, ak->declaration->position.column);
												return 0;
											}
											else
											if (result == 0)
											{
												return 0;
											}
										}
										else
										{
											if (syntax_ps_v(graph, sps))
											{
												syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
													ak->declaration->position.line, ak->declaration->position.column);
												return 0;
											}
										}
									}
									else
									{
										symbol_t *aps;
										aps = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (aps)
										{
											if (syntax_ps_v(graph, aps))
											{
												syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
													ak->declaration->position.line, ak->declaration->position.column);
												return 0;
											}
										}
										else
										{
											syntax_error(graph, sk, "the method is redefined, previous at %lld:%lld", 
												ak->declaration->position.line, ak->declaration->position.column);
											return 0;
										}
									}
								}
							}
						}
					}
				}
				continue;
			}
			else
			if (symbol_check_type(a, SYMBOL_GENERICS))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			if (symbol_check_type(a, SYMBOL_HERITAGES))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			if (symbol_check_type(a, SYMBOL_PARAMETERS))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								if (syntax_gs_v(graph, sgs))
								{
									syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
						}
					}
				}
				continue;
			}
			continue;
		}
		else
		{
			if (symbol_check_type(a, SYMBOL_CLASS))
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_gsgs(graph, sgs, ags);
									if (result == 1)
									{
										syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
											ak->declaration->position.line, ak->declaration->position.column);
										return 0;
									}
									else
									if (result == 0)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(graph, sk, "the class is redefined1, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
							else
							{
								syntax_error(graph, sk, "the class is redefined2, previous at %lld:%lld", 
									ak->declaration->position.line, ak->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}
			else
			if (symbol_check_type(a, SYMBOL_FUNCTION))
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_gsgs(graph, sgs, ags);
									if (result == 1)
									{
										syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
											ak->declaration->position.line, ak->declaration->position.column);
										return 0;
									}
									else
									if (result == 0)
									{
										return 0;
									}
								}
								else
								{
									syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
							else
							{
								syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
									ak->declaration->position.line, ak->declaration->position.column);
								return 0;
							}
						}
					}
				}
				continue;
			}
			else
			if (symbol_check_type(a, SYMBOL_GENERICS))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			if (symbol_check_type(a, SYMBOL_HERITAGES))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			if (symbol_check_type(a, SYMBOL_PARAMETERS))
			{
				int32_t result;
				result = syntax_unique(graph, a, NULL, s);
				if (!result)
				{
					return 0;
				}
			}
			else
			{
				symbol_t *sk;
				sk = syntax_extract_with(s, SYMBOL_KEY);
				if (sk)
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak)
					{
						if (syntax_comparison_id(sk, ak) && (a != s))
						{
							symbol_t *sgs;
							sgs = syntax_only_with(s, SYMBOL_GENERICS);
							if (sgs)
							{
								if (syntax_gs_v(graph, sgs))
								{
									syntax_error(graph, sk, "the class is redefined, previous at %lld:%lld", 
										ak->declaration->position.line, ak->declaration->position.column);
									return 0;
								}
							}
						}
					}
				}
				continue;
			}
			continue;		
		}
	}

	if (root->parent && (subroot != NULL))
	{
		return syntax_unique(graph, root->parent, root, s);
	}

	return 1;
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
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_if(graph_t *graph, symbol_t *current)
{
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}

	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
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
		if (symbol_check_type(a, SYMBOL_BLOCK))
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
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(graph, a);
			if(!result)
			{
				return 0;
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
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
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
					result = syntax_var(graph, b);
					if(!result)
					{
						return 0;
					}
					continue;
				}
				if (symbol_check_type(a, SYMBOL_ASSIGN))
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
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			int32_t result;
			result = syntax_expression(graph, a);
			if(!result)
			{
				return 0;
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
					result = syntax_assign(graph, b);
					if(!result)
					{
						return 0;
					}
					continue;
				}
			}
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
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
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
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
					result = syntax_var(graph, b);
					if(!result)
					{
						return 0;
					}
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_EXPRESSION))
		{
			int32_t result;
			result = syntax_expression(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
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

	if (symbol_check_type(current, SYMBOL_BLOCK))
	{
		result = syntax_block(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_IF))
	{
		result = syntax_if(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_TRY))
	{
		result = syntax_try(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_FOR))
	{
		result = syntax_for(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_FORIN))
	{
		result = syntax_forin(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_FUNCTION))
	{
		result = syntax_function(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_CONTINUE))
	{
		result = syntax_continue(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_BREAK))
	{
		result = syntax_break(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_RETURN))
	{
		result = syntax_return(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_THROW))
	{
		result = syntax_throw(graph, current);
	}
	else if (symbol_check_type(current, SYMBOL_VAR))
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
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}
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
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}
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
syntax_member(graph_t *graph, symbol_t *current)
{
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_enum(graph_t *graph, symbol_t *current)
{
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}

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
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_function(graph_t *graph, symbol_t *current)
{
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}
	
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			int32_t result;
			result = syntax_generics(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
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
syntax_class(graph_t *graph, symbol_t *current)
{
	int32_t result;
	result = syntax_unique(graph, current->parent, current, current);
	if (result == 0)
	{
		return 0;
	}

	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_HERITAGES))
		{
			int32_t result;
			result = syntax_heritages(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			int32_t result;
			result = syntax_parameters(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			int32_t result;
			result = syntax_generics(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PROPERTY))
		{
			int32_t result;
			result = syntax_property(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
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

	return 1;
}

static int32_t
syntax_field(graph_t *graph, symbol_t *current)
{
	symbol_t *fk;
	fk = syntax_only_with(current, SYMBOL_KEY);
	if (fk)
	{
		symbol_t *ft;
		ft = syntax_only_with(current, SYMBOL_TYPE);
		if (ft)
		{
			int32_t result = 1;
			//result = syntax_field_match(graph, fk, ft);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			if (symbol_check_type(fk, SYMBOL_ID))
			{
				int32_t result = 1;
				//result = syntax_unique(graph, fk);
				if (!result)
				{
					return 0;
				} 
				return 1;
			}
			else
			{
				syntax_error(graph, fk, "key is invalid");
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_fields(graph_t *graph, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_FIELD))
		{
			int32_t result;
			result = syntax_field(graph, a);
			if (!result)
			{
				return 0;
			}
			continue;
		}
	}
	return 1;
}

static int32_t
syntax_import(graph_t *graph, symbol_t *import)
{
	symbol_t *a;
	for(a = import->begin; a != import->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_FIELDS))
		{
			int32_t result;
			result = syntax_fields(graph, a);
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
		if (symbol_check_type(a, SYMBOL_IMPORT))
		{
			int32_t result;
			result = syntax_import(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_VAR))
		{
			int32_t result;
			result = syntax_var(graph, a);
			if(!result)
			{
				return 0;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
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
		if (symbol_check_type(a, SYMBOL_MODULE))
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


