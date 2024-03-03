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
syntax_function(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_block(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_import(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_generics(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_generic(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_fields(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_field(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_expression(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_assign(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);

static int32_t
syntax_parameters(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag);


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

static void
printfn(symbol_t *id1)
{
	node_t *nid1 = id1->declaration;
	node_basic_t *nbid1 = (node_basic_t *)nid1->value;

	printf("---->   %s\n", nbid1->value);
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

static void
syntax_xchg(symbol_t *a, symbol_t *b)
{
	symbol_t c = *a;
	*a = *b;
	*b = c;
}

static int32_t
syntax_match_gsast(program_t *program, list_t *scope, symbol_t *gs1, symbol_t *as1, uint64_t flag)
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
							list_t *response1 = list_create();
							if (response1 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}
							int32_t r1;
							r1 = syntax_expression(program, scope, av1, response1, SYNTAX_FLAG_NONE);
							if (r1 == 1)
							{
								list_t *response2 = list_create();
								if (response2 == NULL)
								{
									fprintf(stderr, "unable to allocate memory\n");
									return -1;
								}
								int32_t r2;
								r2 = syntax_expression(program, scope, gt1, response2, SYNTAX_FLAG_NONE);
								if (r2 == 1)
								{
									ilist_t *a1;
									for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
									{
										symbol_t *av2 = (symbol_t *)a1->value;
										if (av2 != NULL)
										{
											ilist_t *a2;
											for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
											{
												symbol_t *av3 = (symbol_t *)a2->value;
												if (av3 != NULL)
												{
													if (av2->id != av3->id)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}
											}
										}
									}
									
								}
								else
								if (r2 == -1)
								{
									return -1;
								}
								else
								{
									syntax_error(program, gt1, "reference not found");
									return -1;
								}
								list_destroy(response2);
							}
							else
							if (r1 == -1)
							{
								return -1;
							}
							else
							{
								syntax_error(program, gt1, "reference not found");
								return -1;
							}
							list_destroy(response1);
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
syntax_match_psasv(program_t *program, list_t *scope, symbol_t *gs1, symbol_t *as1, uint64_t flag)
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
							list_t *response1 = list_create();
							if (response1 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}
							int32_t r1;
							r1 = syntax_expression(program, scope, av1, response1, SYNTAX_FLAG_NONE);
							if (r1 == 1)
							{
								list_t *response2 = list_create();
								if (response2 == NULL)
								{
									fprintf(stderr, "unable to allocate memory\n");
									return -1;
								}
								int32_t r2;
								r2 = syntax_expression(program, scope, gt1, response2, SYNTAX_FLAG_NONE);
								if (r2 == 1)
								{
									ilist_t *a1;
									for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
									{
										symbol_t *av2 = (symbol_t *)a1->value;
										if (av2 != NULL)
										{
											ilist_t *a2;
											for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
											{
												symbol_t *av3 = (symbol_t *)a2->value;
												if (av3 != NULL)
												{
													if (av2->id != av3->id)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}
											}
										}
									}
									
								}
								else
								if (r2 == -1)
								{
									return -1;
								}
								else
								{
									syntax_error(program, gt1, "reference not found");
									return -1;
								}
								list_destroy(response2);
							}
							else
							if (r1 == -1)
							{
								return -1;
							}
							else
							{
								syntax_error(program, gt1, "reference not found");
								return -1;
							}
							list_destroy(response1);
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
syntax_match_pst(program_t *program, symbol_t *ps1, symbol_t *s1)
{
	return 1;
}

static int32_t
syntax_equal_gsgs(program_t *program, list_t *scope, symbol_t *gs1, symbol_t *gs2, uint64_t flag)
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
							list_t *response1 = list_create();
							if (response1 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}
							int32_t r1;
							r1 = syntax_expression(program, scope, gt1, response1, SYNTAX_FLAG_NONE);
							if (r1 == 1)
							{
								list_t *response2 = list_create();
								if (response2 == NULL)
								{
									fprintf(stderr, "unable to allocate memory\n");
									return -1;
								}
								int32_t r2;
								r2 = syntax_expression(program, scope, gt2, response2, SYNTAX_FLAG_NONE);
								if (r2 == 1)
								{
									ilist_t *a1;
									for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
									{
										symbol_t *av1 = (symbol_t *)a1->value;
										if (av1 != NULL)
										{
											ilist_t *a2;
											for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
											{
												symbol_t *av2 = (symbol_t *)a2->value;
												if (av2 != NULL)
												{
													if (av1->id != av2->id)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}
											}
										}
									}
									
								}
								else
								if (r2 == -1)
								{
									return -1;
								}
								else
								{
									syntax_error(program, gt2, "reference not found");
									return -1;
								}
								list_destroy(response2);
							}
							else
							if (r1 == -1)
							{
								return -1;
							}
							else
							{
								syntax_error(program, gt1, "reference not found");
								return -1;
							}
							list_destroy(response1);
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
syntax_equal_psps(program_t *program, list_t *scope, symbol_t *ps1, symbol_t *ps2, uint64_t flag)
{
	int32_t changed = 0;
	symbol_t *p1;
	uint64_t pcnt1;
	region_type:
	pcnt1 = 0;
	for (p1 = ps1->begin;p1 != ps1->end;p1 = p1->next)
	{
		if (symbol_check_type(p1, SYMBOL_GENERIC))
		{
			pcnt1 += 1;
			uint64_t pcnt2 = 0;
			symbol_t *p2;
			for (p2 = ps2->begin;p2 != ps2->end;p2 = p2->next)
			{
				if (symbol_check_type(p2, SYMBOL_GENERIC))
				{
					pcnt2 += 1;
					if (pcnt2 < pcnt1)
					{
						continue;
					}
					symbol_t *pt1;
					pt1 = syntax_extract_with(p1, SYMBOL_TYPE);
					if (pt1)
					{
						symbol_t *pt2;
						pt2 = syntax_extract_with(p2, SYMBOL_TYPE);
						if (pt2)
						{
							list_t *response1 = list_create();
							if (response1 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}
							int32_t r1;
							r1 = syntax_expression(program, scope, pt1, response1, SYNTAX_FLAG_NONE);
							if (r1 == 1)
							{
								list_t *response2 = list_create();
								if (response2 == NULL)
								{
									fprintf(stderr, "unable to allocate memory\n");
									return -1;
								}
								int32_t r2;
								r2 = syntax_expression(program, scope, pt2, response2, SYNTAX_FLAG_NONE);
								if (r2 == 1)
								{
									ilist_t *a1;
									for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
									{
										symbol_t *av1 = (symbol_t *)a1->value;
										if (av1 != NULL)
										{
											ilist_t *a2;
											for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
											{
												symbol_t *av2 = (symbol_t *)a2->value;
												if (av2 != NULL)
												{
													if (av1->id != av2->id)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}
											}
										}
									}
									
								}
								else
								if (r2 == -1)
								{
									return -1;
								}
								else
								{
									syntax_error(program, pt2, "reference not found");
									return -1;
								}
								list_destroy(response2);
							}
							else
							if (r1 == -1)
							{
								return -1;
							}
							else
							{
								syntax_error(program, pt1, "reference not found");
								return -1;
							}
							list_destroy(response1);
						}
					}
				}
			}

			if (pcnt2 < pcnt1)
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
syntax_subset(program_t *program, list_t *scope, symbol_t *t1, symbol_t *t2, uint64_t flag)
{
	if (symbol_check_type(t1, SYMBOL_CLASS))
	{
		if (symbol_check_type(t2, SYMBOL_CLASS))
		{
			if (t1->id == t2->id)
			{
				return 1;
			}
			else
			{
				symbol_t *hs1;
				hs1 = syntax_only_with(t1, SYMBOL_HERITAGES);
				if (hs1 != NULL)
				{
					symbol_t *h1;
					for (h1 = hs1->begin;h1 != hs1->end;h1 = h1->next)
					{
						if (symbol_check_type(h1, SYMBOL_HERITAGE))
						{
							symbol_t *ht1;
							ht1 = syntax_extract_with(t1, SYMBOL_TYPE);
							if (ht1 != NULL)
							{
								list_t *response1 = list_create();
								if (response1 == NULL)
								{
									fprintf(stderr, "unable to allocate memory\n");
									return -1;
								}

								int32_t r1;
								r1 = syntax_expression(program, scope, ht1, response1, flag);
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
									ilist_t *a1;
									for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
									{
										symbol_t *s1 = (symbol_t *)a1->value;
										
										int32_t r2;
										r2 = syntax_subset(program, scope, s1, t2, flag);
										if (r2 == -1)
										{
											return -1;
										}
										else
										if (r2 == 1)
										{
											list_destroy(response1);
											return 1;
										}
									}
									return 0;
								}
								list_destroy(response1);
							}
							
						}
					}
				}
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	if (symbol_check_type(t1, SYMBOL_GENERIC))
	{
		if (symbol_check_type(t2, SYMBOL_GENERIC))
		{
			if (t1->id == t2->id)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

static int32_t
syntax_validation_type(program_t *program, symbol_t *target)
{
	if (symbol_check_type(target, SYMBOL_ATTR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			return syntax_validation_type(program, left);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_COMPOSITE))
	{
		symbol_t *key;
		key = syntax_extract_with(target, SYMBOL_KEY);
		if (key != NULL)
		{
			return syntax_validation_type(program, key);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_TYPEOF))
	{
		return 1;
	}
	else
	if (symbol_check_type(target, SYMBOL_ID))
	{
		return 1;
	}

	syntax_error(program, target, "not a valid type");
	return -1;
}


static int32_t
syntax_id(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *subroot = root->end;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_PROPERTY))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_HERITAGES))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				symbol_t *bk;
				bk = syntax_extract_with(b, SYMBOL_KEY);
				if (bk != NULL)
				{
					if (syntax_idcmp(bk, target) == 0)
					{
						ilist_t *r1;
						r1 = list_rpush(response, b);
						if (r1 == NULL)
						{
							fprintf(stderr, "unable to allocate memory\n");
							return -1;
						}
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				symbol_t *bk;
				bk = syntax_extract_with(b, SYMBOL_KEY);
				if (bk != NULL)
				{
					if (syntax_idcmp(bk, target) == 0)
					{
						ilist_t *r1;
						r1 = list_rpush(response, b);
						if (r1 == NULL)
						{
							fprintf(stderr, "unable to allocate memory\n");
							return -1;
						}
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				symbol_t *bk;
				bk = syntax_extract_with(b, SYMBOL_KEY);
				if (bk != NULL)
				{
					if (syntax_idcmp(bk, target) == 0)
					{
						ilist_t *r1;
						r1 = list_rpush(response, b);
						if (r1 == NULL)
						{
							fprintf(stderr, "unable to allocate memory\n");
							return -1;
						}
					}
				}
			}
		}

		if (a == subroot)
		{
			continue;
		}

		if (symbol_check_type(a, SYMBOL_VAR))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_IF))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_FOR))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
		else
		if (symbol_check_type(a, SYMBOL_FORIN))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idcmp(ak, target) == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		subroot = root;
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_number(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idstrcmp(ak, "Int") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_string(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idstrcmp(ak, "String") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_char(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idstrcmp(ak, "Char") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_null(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idstrcmp(ak, "Null") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_true(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idstrcmp(ak, "Boolean") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_false(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *root = target->parent;
	symbol_t *a;
	region_start_found:
	for (a = root->begin;a != root->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
			{
				if (syntax_idstrcmp(ak, "Boolean") == 0)
				{
					ilist_t *r1;
					r1 = list_rpush(response, a);
					if (r1 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
				}
			}
		}
	}

	if (root->parent != NULL)
	{
		root = root->parent;
		goto region_start_found;
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_primary(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_ID))
	{
		return syntax_id(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_NUMBER))
	{
		return syntax_number(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_STRING))
	{
		return syntax_string(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_CHAR))
	{
		return syntax_char(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_NULL))
	{
		return syntax_null(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_TRUE))
	{
		return syntax_true(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_FALSE))
	{
		return syntax_false(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_PARENTHESIS))
	{
		return syntax_expression(program, scope, syntax_only(target), response, flag);
	}
	else
	{
		return 1;
	}
}

static int32_t
syntax_hierarchy(program_t *program, list_t *scope, symbol_t *base, symbol_t *target, list_t *response, uint64_t flag)
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
					ilist_t *r1;
					r1 = list_rpush(response, a1);
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
					r1 = syntax_expression(program, scope, ht1, &response1, flag);
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
									r2 = syntax_hierarchy(program, scope, brs1, target, response, flag);
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
syntax_attribute(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *left;
	left = syntax_extract_with(target, SYMBOL_LEFT);
	if (left != NULL)
	{
		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, left, response1, flag);
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
			for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
			{
				symbol_t *s1 = (symbol_t *)a1->value;
				if (symbol_check_type(s1, SYMBOL_CLASS))
				{
					symbol_t *right;
					right = syntax_extract_with(target, SYMBOL_RIGHT);
					if (right != NULL)
					{
						list_t *response2 = list_create();
						if (response2 == NULL)
						{
							fprintf(stderr, "unable to allocate memory\n");
							return -1;
						}

						int32_t r3;
						r3 = syntax_hierarchy(program, scope, s1, right, response2, flag);
						if (r3 == -1)
						{
							return -1;
						}

						ilist_t *a2;
						for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
						{
							ilist_t *r4;
							r4 = list_rpush(response, a2->value);
							if (r4 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}
						}
						list_destroy(response2);
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
											ilist_t *r2;
											r2 = list_rpush(response, m1);
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
										ilist_t *r2;
										r2 = list_rpush(response, m1);
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
				{
					syntax_error(program, s1, "not a route");
					return -1;
				}
			}

			list_destroy(response1);
		}
	}

	if (list_count(response) > 0)
	{
		return 1;
	}

	return 0;
}

static int32_t
syntax_composite(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *key;
	key = syntax_extract_with(target, SYMBOL_KEY);
	if (key)
	{
		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, key, response1, flag);
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
			for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
			{
				symbol_t *s1 = (symbol_t *)a1->value;
				if (symbol_check_type(s1, SYMBOL_CLASS))
				{
					symbol_t *gs1;
					gs1 = syntax_only_with(s1, SYMBOL_GENERICS);
					if (gs1 != NULL)
					{
						symbol_t *arguments;
						arguments = syntax_extract_with(target, SYMBOL_ARGUMENTS);
						if (arguments != NULL)
						{
							int32_t r2;
							r2 = syntax_match_gsast(program, scope, gs1, arguments, flag);
							if (r2 == -1)
							{
								return -1;
							}
							else
							if (r2 == 1)
							{
								ilist_t *r3;
								r3 = list_rpush(response, s1);
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
					if (gs != NULL)
					{
						symbol_t *arguments;
						arguments = syntax_extract_with(target, SYMBOL_ARGUMENTS);
						if (arguments != NULL)
						{
							int32_t r2;
							r2 = syntax_match_gsast(program, scope, gs, arguments, flag);
							if (r2 == 1)
							{
								ilist_t *r3;
								r3 = list_rpush(response, s1);
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

		list_destroy(response1);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_call(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *key;
	key = syntax_extract_with(target, SYMBOL_KEY);
	if (key)
	{
		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, key, response1, flag);
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
			for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
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
											r2 = syntax_match_psasv(program, scope, ps1, arguments, flag);
											if (r2 == 1)
											{
												ilist_t *r3;
												r3 = list_rpush(response, s1);
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
							r2 = syntax_match_psasv(program, scope, ps1, arguments, flag);
							if (r2 == 1)
							{
								ilist_t *r3;
								r3 = list_rpush(response, s1);
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

		list_destroy(response1);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_postfix(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_ATTR))
	{
		return syntax_attribute(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_COMPOSITE))
	{
		return syntax_composite(program, scope, target, response, flag);
	}
	else
	if (symbol_check_type(target, SYMBOL_CALL))
	{
		return syntax_call(program, scope, target, response, flag);
	}
	else
	{
		return syntax_primary(program, scope, target, response, flag);
	}
}

static int32_t
syntax_prefix(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_TILDE))
	{
		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right)
		{
			return syntax_prefix(program, scope, right, response, flag);
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
			return syntax_prefix(program, scope, right, response, flag);
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
			return syntax_prefix(program, scope, right, response, flag);
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
			return syntax_prefix(program, scope, right, response, flag);
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
			return syntax_prefix(program, scope, right, response, flag);
		}
		return 0;
	}
	else
	if (symbol_check_type(target, SYMBOL_SIZEOF))
	{
		return syntax_number(program, scope, target, response, flag);
	}
	else
	{
		return syntax_postfix(program, scope, target, response, flag);
	}
}

static int32_t
syntax_operator(program_t *program, list_t *scope, symbol_t *left, symbol_t *right, char *operator, list_t *response, uint64_t flag)
{
	uint64_t cnt1 = list_count(response);

	if (symbol_check_type(left, SYMBOL_CLASS))
	{
		symbol_t *b1;
		for (b1 = left->begin;b1 != left->end;b1 = b1->next)
		{
			if (symbol_check_type(b1, SYMBOL_FUNCTION))
			{
				symbol_t *bk1;
				bk1 = syntax_extract_with(b1, SYMBOL_KEY);
				if (bk1 != NULL)
				{
					if (syntax_idstrcmp(bk1, operator) == 0)
					{
						symbol_t *bps1;
						bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
						if (bps1 != NULL)
						{
							if (symbol_check_type(right, SYMBOL_CLASS))
							{
								int32_t r1;
								r1 = syntax_match_pst(program, bps1, right);
								if (r1 == -1)
								{
									return -1;
								}
								else
								if (r1 == 1)
								{
									int32_t r2;
									r2 = syntax_function(program, scope, b1, response, flag);
									if (r2 == -1)
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

		if ((list_count(response) - cnt1) < 1)
		{
			symbol_t *hs1 = syntax_only_with(left, SYMBOL_HERITAGES);
			if (hs1 != NULL)
			{
				symbol_t *h1;
				for (h1 = hs1->begin;h1 != hs1->end;h1 = h1->next)
				{
					if (symbol_check_type(h1, SYMBOL_HERITAGE))
					{
						symbol_t *ht1 = syntax_only_with(h1, SYMBOL_HERITAGES);
						if (ht1 != NULL)
						{
							list_t *response1 = list_create();
							if (response1 == NULL)
							{
								fprintf(stderr, "unable to allocate memory\n");
								return -1;
							}

							int32_t r1;
							r1 = syntax_expression(program, scope, ht1, response1, flag);
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
								ilist_t *a1;
								for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
								{
									symbol_t *s1 = (symbol_t *)a1->value;
									if (s1 != NULL)
									{
										int32_t r2;
										r2 = syntax_operator(program, scope, s1, right, operator, response, flag);
										if (r2 == -1)
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

			if ((list_count(response) - cnt1) < 1)
			{
				syntax_error(program, left, "operator %s not defined for this type", operator);
				return -1;
			}
		}
	}
	else
	if (symbol_check_type(left, SYMBOL_GENERIC))
	{
		symbol_t *st1;
		st1 = syntax_extract_with(left, SYMBOL_TYPE);
		if (st1 != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}
			
			int32_t r1;
			r1 = syntax_expression(program, scope, st1, response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
			else
			if (r1 == 0)
			{
				syntax_error(program, st1, "reference not found");
				return -1;
			}
			else
			{
				ilist_t *a1;
				for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
				{
					symbol_t *s1 = (symbol_t *)a1->value;
					if (s1 != NULL)
					{
						int32_t r2;
						r2 = syntax_operator(program, scope, s1, right, operator, response, flag);
						if (r2 == -1)
						{
							return -1;
						}
					}
				}
			}
		}
		else
		{
			syntax_error(program, left, "generic without type");
			return -1;
		}
	}
	else
	{
		syntax_error(program, left, "no class for this variable");
		return -1;
	}

	if ((list_count(response) - cnt1) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_power(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_POW))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "**", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_prefix(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_multiplicative(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_MUL))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "*", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_DIV))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "/", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_MOD))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "%", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_EPI))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "\\", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_prefix(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_addative(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_PLUS))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "+", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_MINUS))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "-", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_multiplicative(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_shifting(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_SHL))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "<<", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_SHR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, ">>", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_addative(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_relational(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_LT))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "<", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_GT))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, ">", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_LE))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "<=", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_GE))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, ">=", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_shifting(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_equality(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_EQ))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "==", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_NEQ))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "!=", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_relational(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_bitwise_and(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_AND))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "&", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_equality(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_bitwise_xor(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_XOR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "^", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_bitwise_and(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_bitwise_or(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_OR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "|", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_bitwise_xor(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_logical_and(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_LAND))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "&&", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_bitwise_or(program, scope, target, response, flag);
	}
	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_logical_or(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_LOR))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "||", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_logical_and(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_conditional(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_CONDITIONAL))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			int32_t r1;
			r1 = syntax_expression(program, scope, left, response, flag);
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
		}

		symbol_t *right;
		right = syntax_extract_with(target, SYMBOL_RIGHT);
		if (right != NULL)
		{
			int32_t r2;
			r2 = syntax_expression(program, scope, right, response, flag);
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
	else
	{
		return syntax_logical_or(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}

static int32_t
syntax_expression(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	return syntax_conditional(program, scope, target, response, flag);
}

static int32_t
syntax_assign(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	if (symbol_check_type(target, SYMBOL_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;

							ilist_t *a2;
							for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
							{
								symbol_t *s2 = (symbol_t *)a2->value;
								if (s2 != NULL)
								{
									int32_t r4;
									r4 = syntax_operator(program, scope, s1, s2, "=", response, flag);
									if (r4 == -1)
									{
										return -1;
									}
								}
							}				
						}
					}

					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_ADD_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "+", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '+' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}		
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_SUB_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "-", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '-' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}		
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_MUL_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "*", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '*' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}			
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_DIV_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "/", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '/' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}			
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_MOD_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "%", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '%%' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}		
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_AND_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "&", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '&' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}			
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_OR_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, "|", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '+' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}
							}			
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_SHL_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, ">>", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '<<' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}	
							}			
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	if (symbol_check_type(target, SYMBOL_SHR_ASSIGN))
	{
		symbol_t *left;
		left = syntax_extract_with(target, SYMBOL_LEFT);
		if (left != NULL)
		{
			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r1;
			r1 = syntax_expression(program, scope, left, response1, flag);
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
				if (right != NULL)
				{
					list_t *response2 = list_create();
					if (response2 == NULL)
					{
						fprintf(stderr, "unable to allocate memory\n");
						return -1;
					}
			
					int32_t r3;
					r3 = syntax_expression(program, scope, right, response2, flag);
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
						for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
						{
							symbol_t *s1 = (symbol_t *)a1->value;
							if (s1 != NULL)
							{
								ilist_t *a2;
								for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
								{
									symbol_t *s2 = (symbol_t *)a2->value;
									if (s2 != NULL)
									{
										list_t *response3 = list_create();
										if (response3 == NULL)
										{
											fprintf(stderr, "unable to allocate memory\n");
											return -1;
										}

										int32_t r4;
										r4 = syntax_operator(program, scope, s1, s2, ">>", response3, flag);
										if (r4 == -1)
										{
											return -1;
										}
										else
										if (r4 == 0)
										{
											syntax_error(program, left, "operator '>>' not defined");
											return -1;
										}
										else
										{
											ilist_t *a3;
											for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
											{
												symbol_t *s3 = (symbol_t *)a2->value;
												if (s3 != NULL)
												{
													int32_t r5;
													r5 = syntax_operator(program, scope, s1, s3, "=", response, flag);
													if (r5 == -1)
													{
														return -1;
													}
												}
											}
										}
										list_destroy(response3);
									}
								}	
							}			
						}
					}
					list_destroy(response2);
				}
			}
			list_destroy(response1);
		}
	}
	else
	{
		return syntax_expression(program, scope, target, response, flag);
	}

	if (list_count(response) > 0)
	{
		return 1;
	}
	return 0;
}





static int32_t
syntax_return(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *t1 = syntax_only(target);
	if (t1 != NULL)
	{
		int32_t r1;
		r1 = syntax_expression(program, scope, t1, response, flag);
		if (r1 == -1)
		{
			return -1;
		}
		return r1;
	}
	return 1;
}

static int32_t
syntax_continue(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *t1 = syntax_only(target);
	if (t1 != NULL)
	{
		list_t *response1 = list_create();
		if (response1 != NULL)
		{
			int32_t r1;
			r1 = syntax_expression(program, scope, t1, response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
		}
		list_destroy(response1);
	}
	return 1;
}

static int32_t
syntax_break(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *t1 = syntax_only(target);
	if (t1 != NULL)
	{
		list_t *response1 = list_create();
		if (response1 != NULL)
		{
			int32_t r1;
			r1 = syntax_expression(program, scope, t1, response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
		}
		list_destroy(response1);
	}
	return 1;
}

static int32_t
syntax_throw(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *t1 = syntax_only(target);
	if (t1 != NULL)
	{
		list_t *response1 = list_create();
		if (response1 != NULL)
		{
			int32_t r1;
			r1 = syntax_expression(program, scope, t1, response1, flag);
			if (r1 == -1)
			{
				return -1;
			}
		}
		list_destroy(response1);
	}
	return 1;
}

static int32_t
syntax_var(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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

	symbol_t *left;
	left = syntax_extract_with(target, SYMBOL_TYPE);
	if (left != NULL)
	{
		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, left, response1, flag);
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
			for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
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
							if (bk1 != NULL)
							{
								if (syntax_idstrcmp(bk1, "=") == 0)
								{
									symbol_t *bps1;
									bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
									if (bps1 != NULL)
									{
										symbol_t *right;
										right = syntax_extract_with(target, SYMBOL_VALUE);
										if (right != NULL)
										{
											list_t *response2 = list_create();
											if (response2 == NULL)
											{
												fprintf(stderr, "unable to allocate memory\n");
												return -1;
											}

											int32_t r3;
											r3 = syntax_expression(program, scope, right, response2, flag);
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
												for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
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
															int32_t r5;
															r5 = syntax_function(program, scope, b1, response, flag);
															if (r5 == -1)
															{
																return -1;
															}
															return r5;
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

	symbol_t *tv1 = syntax_extract_with(target, SYMBOL_VALUE);
	if (tv1 != NULL)
	{
		int32_t r1;
		r1 = syntax_expression(program, scope, tv1, response, flag);
		if (r1 == -1)
		{
			return -1;
		}
		return r1;
	}

	return 0;
}

static int32_t
syntax_if(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_expression(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, a, response, flag);
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
					result = syntax_if(program, scope, b, response, flag);
					if (result == -1)
					{
						return -1;
					}
				}
				else
				{
					int32_t result;
					result = syntax_block(program, scope, b, response, flag);
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
syntax_catch(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_parameters(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		else
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, a, response, flag);
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
syntax_try(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, a, response, flag);
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
					result = syntax_catch(program, scope, b, response, flag);
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
syntax_for(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					list_t *response1 = list_create();
					int32_t result;
					result = syntax_var(program, scope, b, response1, flag);
					if (result == -1)
					{
						return -1;
					}
					list_destroy(response1);
					continue;
				}
				if (symbol_check_type(a, SYMBOL_ASSIGN))
				{
					list_t *response1 = list_create();
					int32_t result;
					result = syntax_assign(program, scope, b, response1, flag);
					if (result == -1)
					{
						return -1;
					}
					list_destroy(response1);
					continue;
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CONDITION))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_expression(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_INCREMENTOR))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_type(a, SYMBOL_ASSIGN))
				{
					list_t *response1 = list_create();
					int32_t result;
					result = syntax_assign(program, scope, b, response1, flag);
					if (result == -1)
					{
						return -1;
					}
					list_destroy(response1);
					continue;
				}
			}
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, a, response, flag);
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
syntax_forin(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					list_t *response1 = list_create();
					int32_t result;
					result = syntax_var(program, scope, b, response1, flag);
					if (result == -1)
					{
						return -1;
					}
					list_destroy(response1);
				}
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_EXPRESSION))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_expression(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, a, response, flag);
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
syntax_statement(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	int32_t result = 1;

	if (symbol_check_type(target, SYMBOL_BLOCK))
	{
		result = syntax_block(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_IF))
	{
		result = syntax_if(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_TRY))
	{
		result = syntax_try(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_FOR))
	{
		result = syntax_for(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_FORIN))
	{
		result = syntax_forin(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_FUNCTION))
	{
		result = syntax_function(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_CONTINUE))
	{
		result = syntax_continue(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_BREAK))
	{
		result = syntax_break(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_RETURN))
	{
		result = syntax_return(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_THROW))
	{
		result = syntax_throw(program, scope, target, response, flag);
	}
	else 
	if (symbol_check_type(target, SYMBOL_VAR))
	{
		result = syntax_var(program, scope, target, response, flag);
	}
	else
	{
		/*
		result = syntax_assign(program, scope, target, response, flag);
		*/
	}

	return result;
}

static int32_t
syntax_block(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end);a = a->next)
	{
		int32_t result;
		result = syntax_statement(program, scope, a, response, flag);
		if (result == -1)
		{
			return -1;
		}
		return result;
	}
	return 1;
}



static int32_t
syntax_generic(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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

	symbol_t *tv1 = syntax_extract_with(target, SYMBOL_VALUE);
	if (tv1 != NULL)
	{
		int32_t r0;
		r0 = syntax_validation_type(program, tv1);
		if (r0 == -1)
		{
			return -1;
		}

		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r1;
		r1 = syntax_expression(program, scope, tv1, response1, flag);
		if (r1 == -1)
		{
			return -1;
		}
		else
		if (r1 == 0)
		{
			syntax_error(program, tv1, "reference not found");
			return -1;
		}
		else
		{
			symbol_t *tt1 = syntax_extract_with(target, SYMBOL_TYPE);
			if (tt1 != NULL)
			{
				int32_t r2;
				r2 = syntax_validation_type(program, tt1);
				if (r2 == -1)
				{
					return -1;
				}

				list_t *response2 = list_create();
				if (response2 == NULL)
				{
					fprintf(stderr, "unable to allocate memory\n");
					return -1;
				}

				int32_t r3;
				r3 = syntax_expression(program, scope, tt1, response2, flag);
				if (r3 == -1)
				{
					return -1;
				}
				else
				if (r3 == 0)
				{
					syntax_error(program, tt1, "reference not found");
					return -1;
				}
				else
				{
					ilist_t *a1;
					for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
					{
						symbol_t *av1 = (symbol_t *)a1->value;
						if (av1 != NULL)
						{
							ilist_t *b1;
							for (b1 = response2->begin;b1 != response2->end;b1 = b1->next)
							{
								symbol_t *bv1 = (symbol_t *)b1->value;
								if (bv1 != NULL)
								{
									int32_t r4;
									r4 = syntax_subset(program, scope, av1, bv1, flag);
									if (r4 == -1)
									{
										return -1;
									}
									else 
									if (r4 == 0)
									{
										syntax_error(program, tv1, "not a subset of %lld:%lld", 
											tt1->declaration->position.line, tt1->declaration->position.column);
										return -1;
									}
								}
							}
						}
					}
				}
				list_destroy(response2);
			}
		}
		list_destroy(response1);
	}
	else
	{
		symbol_t *tt1 = syntax_extract_with(target, SYMBOL_TYPE);
		if (tt1 != NULL)
		{
			int32_t r1;
			r1 = syntax_validation_type(program, tt1);
			if (r1 == -1)
			{
				return -1;
			}

			list_t *response1 = list_create();
			if (response1 == NULL)
			{
				fprintf(stderr, "unable to allocate memory\n");
				return -1;
			}

			int32_t r2;
			r2 = syntax_expression(program, scope, tt1, response1, flag);
			if (r2 == -1)
			{
				return -1;
			}
			else
			if (r2 == 0)
			{
				syntax_error(program, tt1, "reference not found");
				return -1;
			}
			list_destroy(response1);
		}
	}
	
	return 1;
}

static int32_t
syntax_generics(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_generic(program, scope, a, response, flag);
		if (result == -1)
		{
			return -1;
		}
	}

	return 1;
}

static int32_t
syntax_parameter(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
	
	symbol_t *tt1 = syntax_extract_with(target, SYMBOL_TYPE);
	if (tt1 != NULL)
	{
		int32_t r1;
		r1 = syntax_validation_type(program, tt1);
		if (r1 == -1)
		{
			return -1;
		}

		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r2;
		r2 = syntax_expression(program, scope, tt1, response1, flag);
		if (r2 == -1)
		{
			return -1;
		}
		else
		if (r2 == 0)
		{
			syntax_error(program, tt1, "reference not found");
			return -1;
		}
		list_destroy(response1);
	}

	return 1;
}

static int32_t
syntax_parameters(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_parameter(program, scope, a, response, flag);
		if (result == -1)
		{
			return -1;
		}

		symbol_t *ak;
		ak = syntax_extract_with(a, SYMBOL_KEY);
		if (ak != NULL)
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
syntax_heritage(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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

	
	symbol_t *tt1 = syntax_extract_with(target, SYMBOL_TYPE);
	if (tt1 != NULL)
	{
		int32_t r1;
		r1 = syntax_validation_type(program, tt1);
		if (r1 == -1)
		{
			return -1;
		}

		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r2;
		r2 = syntax_expression(program, scope, tt1, response1, flag);
		if (r2 == -1)
		{
			return -1;
		}
		else
		if (r2 == 0)
		{
			syntax_error(program, tt1, "reference not found");
			return -1;
		}
		list_destroy(response1);
	}
	

	return 1;
}

static int32_t
syntax_heritages(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_heritage(program, scope, a, response, flag);
		if (result == -1)
		{
			return -1;
		}

		symbol_t *ak;
		ak = syntax_extract_with(a, SYMBOL_KEY);
		if (ak != NULL)
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
syntax_member(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
syntax_members(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		int32_t result;
		result = syntax_member(program, scope, a, response, flag);
		if (result == -1)
		{
			return -1;
		}
	}
	return 1;
}

static int32_t
syntax_enum(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
													if (bps2 != NULL)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (bpv2 == NULL)
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
												if (bps2 != NULL)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (bpv2 == NULL)
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
					if (ak != NULL)
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
									if (bps2 != NULL)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (bpv2 == NULL)
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
								if (bps2 != NULL)
								{
									int32_t parameter_without_value = 0;
									symbol_t *bp2;
									for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
									{
										symbol_t *bpv2;
										bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
										if (bpv2 == NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_members(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
		}
	}

	return 1;
}

static int32_t
syntax_property(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
													if (bps2 != NULL)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (bpv2 == NULL)
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
												if (bps2 != NULL)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (bpv2 == NULL)
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
					if (ak != NULL)
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
									if (bps2 != NULL)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (bpv2 == NULL)
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
								if (bps2 != NULL)
								{
									int32_t parameter_without_value = 0;
									symbol_t *bp2;
									for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
									{
										symbol_t *bpv2;
										bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
										if (bpv2 == NULL)
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
					if (ak != NULL)
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
					if (ak != NULL)
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

	symbol_t *tt1 = syntax_extract_with(target, SYMBOL_TYPE);
	if (tt1 != NULL)
	{
		int32_t r1;
		r1 = syntax_validation_type(program, tt1);
		if (r1 == -1)
		{
			return -1;
		}

		list_t *response1 = list_create();
		if (response1 == NULL)
		{
			fprintf(stderr, "unable to allocate memory\n");
			return -1;
		}

		int32_t r2;
		r2 = syntax_expression(program, scope, tt1, response1, flag);
		if (r2 == -1)
		{
			return -1;
		}
		else
		if (r2 == 0)
		{
			syntax_error(program, tt1, "reference not found");
			return -1;
		}
		list_destroy(response1);
	}

	return 1;
}

static int32_t
syntax_function(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, scope, cgs, ags, flag);
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
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
											if (cgv == NULL)
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
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
													if (bps1 != NULL)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
														if (bps2 != NULL)
														{
															int32_t result;
															result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																if (bpv1 == NULL)
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
														if (bps2 != NULL)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (bpv2 == NULL)
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
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, scope, cgs, ags, flag);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *bps1;
										bps1 = syntax_only_with(target, SYMBOL_PARAMETERS);
										if (bps1 != NULL)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2 != NULL)
											{
												int32_t result;
												result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
													if (bpv1 == NULL)
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
											if (bps2 != NULL)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (bpv2 == NULL)
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
											if (cgv == NULL)
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
										if (bps1 != NULL)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2 != NULL)
											{
												int32_t result;
												result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
													if (bpv1 == NULL)
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
											if (bps2 != NULL)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (bpv2 == NULL)
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
										if (bps1 != NULL)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
											if (bps2 != NULL)
											{
												int32_t result;
												result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
													if (bpv1 == NULL)
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
											if (bps2 != NULL)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (bpv2 == NULL)
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
									if (bps1 != NULL)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2 != NULL)
										{
											int32_t result;
											result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
												if (bpv1 == NULL)
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
										if (bps2 != NULL)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (bpv2 == NULL)
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
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (cgv == NULL)
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
									if (bps1 != NULL)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2 != NULL)
										{
											int32_t result;
											result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
												if (bpv1 == NULL)
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
										if (bps2 != NULL)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (bpv2 == NULL)
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
								if (bps1 != NULL)
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2 != NULL)
									{
										int32_t result;
										result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
											if (bpv1 == NULL)
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
									if (bps2 != NULL)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (bpv2 == NULL)
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
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (cgv == NULL)
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
									if (bps1 != NULL)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2 != NULL)
										{
											int32_t result;
											result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
												if (bpv1 == NULL)
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
										if (bps2 != NULL)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (bpv2 == NULL)
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
								if (bps1 != NULL)
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2 != NULL)
									{
										int32_t result;
										result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
											if (bpv1 == NULL)
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
									if (bps2 != NULL)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (bpv2 == NULL)
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
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (cgv == NULL)
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
									if (bps1 != NULL)
									{
										symbol_t *bps2;
										bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
										if (bps2 != NULL)
										{
											int32_t result;
											result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
												if (bpv1 == NULL)
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
										if (bps2 != NULL)
										{
											int32_t parameter_without_value = 0;
											symbol_t *bp2;
											for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
											{
												symbol_t *bpv2;
												bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
												if (bpv2 == NULL)
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
								if (bps1 != NULL)
								{
									symbol_t *bps2;
									bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
									if (bps2 != NULL)
									{
										int32_t result;
										result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
											if (bpv1 == NULL)
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
									if (bps2 != NULL)
									{
										int32_t parameter_without_value = 0;
										symbol_t *bp2;
										for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
										{
											symbol_t *bpv2;
											bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
											if (bpv2 == NULL)
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
									if (cgs != NULL)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (cgv == NULL)
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
											if (bps1 != NULL)
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
												if (bps2 != NULL)
												{
													int32_t result;
													result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
														if (bpv1 == NULL)
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
												if (bps2 != NULL)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (bpv2 == NULL)
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
										if (bps1 != NULL)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
											if (bps2 != NULL)
											{
												int32_t result;
												result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
													if (bpv1 == NULL)
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
											if (bps2 != NULL)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (bpv2 == NULL)
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
									if (cgs != NULL)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (cgv == NULL)
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
											if (bps1 != NULL)
											{
												symbol_t *bps2;
												bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
												if (bps2 != NULL)
												{
													int32_t result;
													result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
														if (bpv1 == NULL)
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
												if (bps2 != NULL)
												{
													int32_t parameter_without_value = 0;
													symbol_t *bp2;
													for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
													{
														symbol_t *bpv2;
														bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
														if (bpv2 == NULL)
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
										if (bps1 != NULL)
										{
											symbol_t *bps2;
											bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
											if (bps2 != NULL)
											{
												int32_t result;
												result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
													if (bpv1 == NULL)
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
											if (bps2 != NULL)
											{
												int32_t parameter_without_value = 0;
												symbol_t *bp2;
												for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
												{
													symbol_t *bpv2;
													bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
													if (bpv2 == NULL)
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

	symbol_t *a;
	for (a = target->begin;a != target->end;a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_parameters(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		else
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_generics(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		else
		if (symbol_check_type(a, SYMBOL_BLOCK))
		{
			int32_t result;
			result = syntax_block(program, scope, a, response, flag);
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
syntax_class(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *ck;
	ck = syntax_extract_with(target, SYMBOL_KEY);
	if (ck)
	{
		symbol_t *root = target->parent;
		if (root != NULL)
		{
			symbol_t *a;
			for (a = root->begin;(a != root->end) && (a != target);a = a->next)
			{
				if (symbol_check_type(a, SYMBOL_CLASS))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags != NULL)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, scope, cgs, ags, flag);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *b1;
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
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
																		if (bps1 != NULL)
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2 != NULL)
																			{
																				int32_t result;
																				result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																				for (bp1 = bps1->begin;bp1 != bps1->end;bp1 = bp1->next)
																				{
																					symbol_t *bpv1;
																					bpv1 = syntax_only_with(bp1, SYMBOL_VALUE);
																					if (bpv1 == NULL)
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
																			if (bps2 != NULL)
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp2;
																				for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																				{
																					symbol_t *bpv2;
																					bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																					if (bpv2 == NULL)
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
											if (cgv == NULL)
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
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
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
																if (bk2 != NULL)
																{
																	if (syntax_idstrcmp(bk2, "constructor") == 0)
																	{
																		symbol_t *bps1;
																		bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																		if (bps1 != NULL)
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2 != NULL)
																			{
																				int32_t result;
																				result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																					if (bpv1 == NULL)
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
																			if (bps2 != NULL)
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp2;
																				for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																				{
																					symbol_t *bpv2;
																					bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																					if (bpv2 == NULL)
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
											if (agv == NULL)
											{
												generic_without_value = 1;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b1;
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
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
																if (bk2 != NULL)
																{
																	if (syntax_idstrcmp(bk2, "constructor") == 0)
																	{
																		symbol_t *bps1;
																		bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																		if (bps1 != NULL)
																		{
																			symbol_t *bps2;
																			bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																			if (bps2 != NULL)
																			{
																				int32_t result;
																				result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																					if (bpv1 == NULL)
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
																			if (bps2 != NULL)
																			{
																				int32_t parameter_without_value = 0;
																				symbol_t *bp2;
																				for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																				{
																					symbol_t *bpv2;
																					bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																					if (bpv2 != NULL)
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
									for (b1 = target->begin;b1 != target->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
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
															if (bk2 != NULL)
															{
																if (syntax_idstrcmp(bk2, "constructor") == 0)
																{
																	symbol_t *bps1;
																	bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
																	if (bps1 != NULL)
																	{
																		symbol_t *bps2;
																		bps2 = syntax_only_with(b2, SYMBOL_PARAMETERS);
																		if (bps2 != NULL)
																		{
																			int32_t result;
																			result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																				if (bpv1 == NULL)
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
																		if (bps2 != NULL)
																		{
																			int32_t parameter_without_value = 0;
																			symbol_t *bp2;
																			for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																			{
																				symbol_t *bpv2;
																				bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																				if (bpv2 == NULL)
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
					}
				}
				else
				if (symbol_check_type(a, SYMBOL_FUNCTION))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								symbol_t *ags;
								ags = syntax_only_with(a, SYMBOL_GENERICS);
								if (ags != NULL)
								{
									int32_t result;
									result = syntax_equal_gsgs(program, scope, cgs, ags, flag);
									if (result == -1)
									{
										return -1;
									}
									else
									if (result == 1)
									{
										symbol_t *b1;
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
											if (cgv == NULL)
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
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
								if (ags != NULL)
								{
									int32_t generic_without_value = 0;
									symbol_t *ag;
									for (ag = ags->begin;ag != ags->end;ag = ag->next)
									{
										if (symbol_check_type(ag, SYMBOL_GENERIC))
										{
											symbol_t *agv;
											agv = syntax_only_with(ag, SYMBOL_VALUE);
											if (agv == NULL)
											{
												generic_without_value = 1;
											}
										}
									}
									if (generic_without_value == 0)
									{
										// check parameters
										symbol_t *b1;
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
									for (b1 = target->begin;b1 != target->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2 != NULL)
														{
															int32_t result;
															result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																if (bpv1 == NULL)
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
														if (bps2 != NULL)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (bpv2 == NULL)
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
				else
				if (symbol_check_type(a, SYMBOL_PROPERTY))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (cgv == NULL)
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
									for (b1 = target->begin;b1 != target->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2 != NULL)
														{
															int32_t result;
															result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																if (bpv1 == NULL)
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
														if (bps2 != NULL)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (bpv2 == NULL)
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
								for (b1 = target->begin;b1 != target->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1 != NULL)
										{
											if (syntax_idstrcmp(bk1, "constructor") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1 != NULL)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2 != NULL)
													{
														int32_t result;
														result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
															if (bpv1 == NULL)
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
													if (bps2 != NULL)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (bpv2 == NULL)
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
				else
				if (symbol_check_type(a, SYMBOL_VAR))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (cgv == NULL)
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
									for (b1 = target->begin;b1 != target->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2 != NULL)
														{
															int32_t result;
															result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																if (bpv1 == NULL)
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
														if (bps2 != NULL)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (bpv2 == NULL)
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
								for (b1 = target->begin;b1 != target->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1 != NULL)
										{
											if (syntax_idstrcmp(bk1, "constructor") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1 != NULL)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2 != NULL)
													{
														int32_t result;
														result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
															if (bpv1 == NULL)
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
													if (bps2 != NULL)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (bpv2 == NULL)
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
				else
				if (symbol_check_type(a, SYMBOL_ENUM))
				{
					symbol_t *ak;
					ak = syntax_extract_with(a, SYMBOL_KEY);
					if (ak != NULL)
					{
						if (syntax_idcmp(ck, ak) == 0)
						{
							symbol_t *cgs;
							cgs = syntax_only_with(target, SYMBOL_GENERICS);
							if (cgs != NULL)
							{
								int32_t generic_without_value = 0;
								symbol_t *cg;
								for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
								{
									if (symbol_check_type(cg, SYMBOL_GENERIC))
									{
										symbol_t *cgv;
										cgv = syntax_only_with(cg, SYMBOL_VALUE);
										if (cgv == NULL)
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
									for (b1 = target->begin;b1 != target->end;b1 = b1->next)
									{
										if (symbol_check_type(b1, SYMBOL_FUNCTION))
										{
											symbol_t *bk1;
											bk1 = syntax_extract_with(b1, SYMBOL_KEY);
											if (bk1 != NULL)
											{
												if (syntax_idstrcmp(bk1, "constructor") == 0)
												{
													symbol_t *bps1;
													bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
													if (bps1 != NULL)
													{
														symbol_t *bps2;
														bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
														if (bps2 != NULL)
														{
															int32_t result;
															result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																if (bpv1 == NULL)
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
														if (bps2 != NULL)
														{
															int32_t parameter_without_value = 0;
															symbol_t *bp2;
															for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
															{
																symbol_t *bpv2;
																bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																if (bpv2 == NULL)
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
								for (b1 = target->begin;b1 != target->end;b1 = b1->next)
								{
									if (symbol_check_type(b1, SYMBOL_FUNCTION))
									{
										symbol_t *bk1;
										bk1 = syntax_extract_with(b1, SYMBOL_KEY);
										if (bk1 != NULL)
										{
											if (syntax_idstrcmp(bk1, "constructor") == 0)
											{
												symbol_t *bps1;
												bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
												if (bps1 != NULL)
												{
													symbol_t *bps2;
													bps2 = syntax_only_with(a, SYMBOL_PARAMETERS);
													if (bps2 != NULL)
													{
														int32_t result;
														result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
															if (bpv1 == NULL)
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
													if (bps2 != NULL)
													{
														int32_t parameter_without_value = 0;
														symbol_t *bp2;
														for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
														{
															symbol_t *bpv2;
															bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
															if (bpv2 == NULL)
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
									if (cgs != NULL)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (cgv == NULL)
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
											for (b1 = target->begin;b1 != target->end;b1 = b1->next)
											{
												if (symbol_check_type(b1, SYMBOL_FUNCTION))
												{
													symbol_t *bk1;
													bk1 = syntax_extract_with(b1, SYMBOL_KEY);
													if (bk1 != NULL)
													{
														if (syntax_idstrcmp(bk1, "constructor") == 0)
														{
															symbol_t *bps1;
															bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
															if (bps1 != NULL)
															{
																symbol_t *bps2;
																bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
																if (bps2 != NULL)
																{
																	int32_t result;
																	result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																		if (bpv1 == NULL)
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
																if (bps2 != NULL)
																{
																	int32_t parameter_without_value = 0;
																	symbol_t *bp2;
																	for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																	{
																		symbol_t *bpv2;
																		bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																		if (bpv2 == NULL)
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
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
								if ((syntax_idcmp(ck, dk) == 0) && (target != a))
								{
									symbol_t *cgs;
									cgs = syntax_only_with(target, SYMBOL_GENERICS);
									if (cgs != NULL)
									{
										int32_t generic_without_value = 0;
										symbol_t *cg;
										for (cg = cgs->begin;cg != cgs->end;cg = cg->next)
										{
											if (symbol_check_type(cg, SYMBOL_GENERIC))
											{
												symbol_t *cgv;
												cgv = syntax_only_with(cg, SYMBOL_VALUE);
												if (cgv == NULL)
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
											for (b1 = target->begin;b1 != target->end;b1 = b1->next)
											{
												if (symbol_check_type(b1, SYMBOL_FUNCTION))
												{
													symbol_t *bk1;
													bk1 = syntax_extract_with(b1, SYMBOL_KEY);
													if (bk1 != NULL)
													{
														if (syntax_idstrcmp(bk1, "constructor") == 0)
														{
															symbol_t *bps1;
															bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
															if (bps1 != NULL)
															{
																symbol_t *bps2;
																bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
																if (bps2 != NULL)
																{
																	int32_t result;
																	result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																		if (bpv1 == NULL)
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
																if (bps2 != NULL)
																{
																	int32_t parameter_without_value = 0;
																	symbol_t *bp2;
																	for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																	{
																		symbol_t *bpv2;
																		bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																		if (bpv2 == NULL)
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
										for (b1 = target->begin;b1 != target->end;b1 = b1->next)
										{
											if (symbol_check_type(b1, SYMBOL_FUNCTION))
											{
												symbol_t *bk1;
												bk1 = syntax_extract_with(b1, SYMBOL_KEY);
												if (bk1 != NULL)
												{
													if (syntax_idstrcmp(bk1, "constructor") == 0)
													{
														symbol_t *bps1;
														bps1 = syntax_only_with(b1, SYMBOL_PARAMETERS);
														if (bps1 != NULL)
														{
															symbol_t *bps2;
															bps2 = syntax_only_with(d, SYMBOL_PARAMETERS);
															if (bps2 != NULL)
															{
																int32_t result;
																result = syntax_equal_psps(program, scope, bps1, bps2, flag);
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
																	if (bpv1 == NULL)
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
															if (bps2 != NULL)
															{
																int32_t parameter_without_value = 0;
																symbol_t *bp2;
																for (bp2 = bps2->begin;bp2 != bps2->end;bps2 = bps2->next)
																{
																	symbol_t *bpv2;
																	bpv2 = syntax_only_with(bp2, SYMBOL_VALUE);
																	if (bpv2 == NULL)
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
							}
						}
					}
				}
			}
		}
	}

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
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_heritages(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PARAMETERS))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_parameters(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_GENERICS))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_generics(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_PROPERTY))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_property(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_class(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_enum(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			list_t *response1 = list_create();
			int32_t result;
			result = syntax_function(program, scope, a, response1, flag);
			if (result == -1)
			{
				return -1;
			}
			list_destroy(response1);
			continue;
		}
	}

	list_unlink(scope, il1);
	return 1;
}

static int32_t
syntax_field(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
					if (ak != NULL)
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
syntax_fields(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;(a != target->end); a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_FIELD))
		{
			int32_t result;
			result = syntax_field(program, scope, a, response, flag);
			if (result == -1)
			{
				return -1;
			}
			
			symbol_t *ak;
			ak = syntax_extract_with(a, SYMBOL_KEY);
			if (ak != NULL)
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
syntax_import(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
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
			result = syntax_fields(program, scope, a, response, flag);
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
syntax_module(program_t *program, list_t *scope, symbol_t *target, list_t *response, uint64_t flag)
{
	symbol_t *a;
	for(a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_type(a, SYMBOL_IMPORT))
		{
			int32_t result;
			result = syntax_import(program, scope, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_CLASS))
		{
			int32_t result;
			result = syntax_class(program, scope, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_ENUM))
		{
			int32_t result;
			result = syntax_enum(program, scope, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_VAR))
		{
			int32_t result;
			result = syntax_var(program, scope, a, response, flag);
			if(result == -1)
			{
				return -1;
			}
			continue;
		}
		if (symbol_check_type(a, SYMBOL_FUNCTION))
		{
			int32_t result;
			result = syntax_function(program, scope, a, response, flag);
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

			int32_t result;
			result = syntax_module(program, scope, a, response, SYNTAX_FLAG_NONE);
			if(result == -1)
			{
				return -1;
			}
		}
	}

	return 1;
}


