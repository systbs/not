/*
 * Creator: Yasser Sajjadi(Ys)
 * Date: 24/3/2024
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "syntax.h"
#include "error.h"
#include "semantic.h"
#include "mensuration.h"

typedef enum semantic_flag 
{
    SEMANTIC_FLAG_NONE    = 0,
    SEMANTIC_FLAG_SELECT_FOLLOW  = 1 << 0, // follow is shows the method reversible or not
    SEMANTIC_FLAG_SELECT_OBJECT  = 1 << 1, // its work as search for none-types
    SEMANTIC_FLAG_COMPILE        = 1 << 2
} semantic_flag_t;


static error_t *
semantic_error(program_t *program, node_t *node, const char *format, ...)
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
semantic_idcmp(node_t *n1, node_t *n2)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	node_basic_t *nb2 = (node_basic_t *)n2->value;

	//printf("%s %s\n", nb1->value, nb2->value);

	return (strcmp(nb1->value, nb2->value) == 0);
}

static int32_t
semantic_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}



static int32_t
semantic_postfix(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag);

static int32_t
semantic_resolve(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag);

static int32_t
semantic_expression(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag);


static int32_t
semantic_parameters(program_t *program, node_t *node, uint64_t flag);

static int32_t
semantic_generics(program_t *program, node_t *node, uint64_t flag);

static int32_t
semantic_body(program_t *program, node_t *node, uint64_t flag);


/*
 * @Parameters: n1 is a class, n2 is a class
 * @Note: check n1 is subset of n2
 * @Return: 1 if is a subset otherwise 0 
*/
static int32_t
semantic_subset(program_t *program, node_t *n1, node_t *n2)
{
    if (n1->id == n2->id)
    {
        return 1;
    }

    if (n2->kind == NODE_KIND_CLASS)
    {
        node_class_t *class2 = (node_class_t *)n2->value;
        if (class2->heritages != NULL)
        {
            node_t *node2 = class2->heritages;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_HERITAGE)
                {
                    node_heritage_t *heritage2 = (node_heritage_t *)item2->value;

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_postfix(program, NULL, heritage2->type, response2, SEMANTIC_FLAG_NONE);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, heritage2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (r2 == 1)
                    {
                        ilist_t *b2;
                        for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                        {
                            node_t *item3 = (node_t *)b2->value;
                            int32_t r3 = semantic_subset(program, n1, item3);
                            if (r3 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r3 == 1)
                            {
                                list_destroy(response2);
                                return 1;
                            }
                        }
                    }
                    list_destroy(response2);
                }
            }
        }
    }

    return 0;
}


static int32_t
semantic_eqaul_gsgs(program_t *program, node_t *ngs1, node_t *ngs2)
{
   if (ngs1 == NULL)
    {
        if (ngs2 == NULL)
        {
            return 1;
        }
        else
        {
            node_block_t *bgs2 = (node_block_t *)ngs2->value;
            ilist_t *a2;
            for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;
                    if (generic2->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
    }
    else
    {
        if (ngs2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    if (generic1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt1 = 0, cnt2 = 0;
            cnt1 = 0;

            node_block_t *bps1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    cnt1 += 1;
                    cnt2 = 0;
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    node_block_t *bgs2 = (node_block_t *)ngs2->value;
                    ilist_t *a2;
                    for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            cnt2 += 1;
                            if (cnt2 < cnt1)
                            {
                                continue;
                            }
                            node_generic_t *generic2 = (node_generic_t *)item2->value;
                            
                            if (generic1->type != NULL)
                            {
                                if (generic2->type != NULL)
                                {
                                    list_t *response1 = list_create();
                                    if (response1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    int32_t r1 = semantic_resolve(program, NULL, generic1->type, response1, SEMANTIC_FLAG_NONE);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 0)
                                    {
                                        semantic_error(program, generic1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r1 == 1)
                                    {
                                        ilist_t *b1;
                                        for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                        {
                                            node_t *item3 = (node_t *)b1->value;

                                            list_t *response2 = list_create();
                                            if (response2 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            int32_t r2 = semantic_resolve(program, NULL, generic2->type, response2, SEMANTIC_FLAG_NONE);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 0)
                                            {
                                                semantic_error(program, generic2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                ilist_t *b2;
                                                for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                {
                                                    node_t *item4 = (node_t *)b2->value;
                                                    int32_t r3 = semantic_subset(program, item3, item4);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        r3 = semantic_subset(program, item4, item3);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            goto region_by_name_check;
                                                        }
                                                        
                                                    }
                                                }
                                            }
                                            list_destroy(response2);
                                        }
                                    
                                        list_destroy(response1);
                                    }
                                    break;
                               }
                            }
                        }
                    }

                    if (cnt1 > cnt2)
                    {
                        if (generic1->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            cnt2 = 0;
            node_block_t *bgs2 = (node_block_t *)ngs2->value;
            ilist_t *a2;
            for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    cnt2 += 1;
                    if (cnt2 > cnt1)
                    {
                        node_generic_t *generic2 = (node_generic_t *)item2->value;
                        if (generic2->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            return 1;

            region_by_name_check:
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;

                    int32_t found = 0;
                    node_block_t *bgs2 = (node_block_t *)ngs2->value;
                    ilist_t *a2;
                    for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic2 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(generic1->key, generic2->key) == 1)
                            {
                                found = 1;
                                if (generic1->type != NULL)
                                {
                                    if (generic2->type != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_resolve(program, NULL, generic1->type, response1, SEMANTIC_FLAG_NONE);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            ilist_t *b1;
                                            for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                            {
                                                node_t *item3 = (node_t *)b1->value;

                                                list_t *response2 = list_create();
                                                if (response2 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_resolve(program, NULL, generic2->type, response2, SEMANTIC_FLAG_NONE);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, generic2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    ilist_t *b2;
                                                    for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                    {
                                                        node_t *item4 = (node_t *)b2->value;
                                                        
                                                        int32_t r3 = semantic_subset(program, item3, item4);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            r3 = semantic_subset(program, item4, item3);
                                                            if (r3 == -1)
                                                            {
                                                                return -1;
                                                            }
                                                            else
                                                            if (r3 == 0)
                                                            {
                                                                list_destroy(response2);
                                                                list_destroy(response1);
                                                                return 0;
                                                            }
                                                        }
                                                    }
                                                }
                                                list_destroy(response2);
                                                break;
                                            }
                                        }
                                        list_destroy(response1);
                                    }
                                }
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (generic1->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            bgs2 = (node_block_t *)ngs2->value;
            for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;

                    int32_t found = 0;
                    for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item1->value;

                            if (semantic_idcmp(generic1->key, generic2->key) == 1)
                            {
                                found = 1;
                                if (generic1->type != NULL)
                                {
                                    if (generic2->type != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_resolve(program, NULL, generic1->type, response1, SEMANTIC_FLAG_NONE);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            ilist_t *b1;
                                            for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                            {
                                                node_t *item3 = (node_t *)b1->value;

                                                list_t *response2 = list_create();
                                                if (response2 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_resolve(program, NULL, generic2->type, response2, SEMANTIC_FLAG_NONE);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, generic2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    ilist_t *b2;
                                                    for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                    {
                                                        node_t *item4 = (node_t *)b2->value;
                                                        
                                                        int32_t r3 = semantic_subset(program, item3, item4);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            r3 = semantic_subset(program, item4, item3);
                                                            if (r3 == -1)
                                                            {
                                                                return -1;
                                                            }
                                                            else
                                                            if (r3 == 0)
                                                            {
                                                                list_destroy(response2);
                                                                list_destroy(response1);
                                                                return 0;
                                                            }
                                                        }
                                                    }
                                                }
                                                list_destroy(response2);
                                                break;
                                            }
                                        }
                                        list_destroy(response1);
                                    }
                                }
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (generic2->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            return 1;
        }
    }
    return 0;
}

static int32_t
semantic_eqaul_psps(program_t *program, node_t *nps1, node_t *nps2)
{
    if (nps1 == NULL)
    {
        if (nps2 == NULL)
        {
            return 1;
        }
        else
        {
            node_block_t *bps2 = (node_block_t *)nps2->value;
            ilist_t *a2;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (parameter2->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
    }
    else
    {
        if (nps2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    if (parameter1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt1, cnt2;
            cnt1 = 0;

            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    cnt1 += 1;
                    cnt2 = 0;
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    node_block_t *bps2 = (node_block_t *)nps2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PARAMETER)
                        {
                            cnt2 += 1;
                            if (cnt2 < cnt1)
                            {
                                continue;
                            }
                            
                            node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            
                            int32_t r1 = semantic_resolve(program, NULL, parameter1->type, response1, SEMANTIC_FLAG_NONE);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 0)
                            {
                                semantic_error(program, parameter1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                ilist_t *b1;
                                for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                {
                                    node_t *item3 = (node_t *)b1->value;

                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    int32_t r2 = semantic_resolve(program, NULL, parameter2->type, response2, SEMANTIC_FLAG_NONE);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, parameter2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        ilist_t *b2;
                                        for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                        {
                                            node_t *item4 = (node_t *)b2->value;
                                            
                                            if (item3->id != item4->id)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                goto region_by_name_check;
                                            }
                                        }
                                    }
                                     list_destroy(response2);
                                }
                               
                                list_destroy(response1);
                            }
                            break;
                        }
                    }

                    if (cnt1 > cnt2)
                    {
                        if (parameter1->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            
            cnt2 = 0;
            node_block_t *bps2 = (node_block_t *)nps2->value;
            ilist_t *a2;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    cnt2 += 1;
                    if (cnt2 > cnt1)
                    {
                        node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                        if (parameter2->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            return 1;
            
            region_by_name_check:
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

                    int32_t found = 0;
                    node_block_t *bps2 = (node_block_t *)nps2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                            if (semantic_idcmp(parameter1->key, parameter2->key) == 1)
                            {
                                found = 1;
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_resolve(program, NULL, parameter1->type, response1, SEMANTIC_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, parameter1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    ilist_t *b1;
                                    for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                    {
                                        node_t *item3 = (node_t *)b1->value;

                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_resolve(program, NULL, parameter2->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, parameter2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            ilist_t *b2;
                                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                            {
                                                node_t *item4 = (node_t *)b2->value;
                                                
                                                if (item3->id != item4->id)
                                                {
                                                    list_destroy(response2);
                                                    list_destroy(response1);
                                                    return 0;
                                                }
                                            }
                                        }
                                        list_destroy(response2);
                                        break;
                                    }
                                }
                                list_destroy(response1);
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (parameter1->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }
            

            bps2 = (node_block_t *)nps2->value;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;

                    int32_t found = 0;
                    for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

                            if (semantic_idcmp(parameter1->key, parameter2->key) == 1)
                            {
                                found = 1;
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_resolve(program, NULL, parameter1->type, response1, SEMANTIC_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, parameter1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    ilist_t *b1;
                                    for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                    {
                                        node_t *item3 = (node_t *)b1->value;

                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_resolve(program, NULL, parameter2->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, parameter2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            ilist_t *b2;
                                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                            {
                                                node_t *item4 = (node_t *)b2->value;
                                                
                                                if (item3->id != item4->id)
                                                {
                                                    list_destroy(response2);
                                                    list_destroy(response1);
                                                    return 0;
                                                }
                                            }
                                        }
                                        list_destroy(response2);
                                        break;
                                    }
                                }
                                list_destroy(response1);
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (parameter2->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            return 1;
        }
    }

    return 0;
}

static int32_t
semantic_eqaul_psas(program_t *program, node_t *nps1, node_t *nas2)
{
   if (nps1 == NULL)
    {
        if (nas2 == NULL)
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
        if (nas2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    if (parameter1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt2 = 0;
			int32_t use_by_key = 0;

			list_t *repository1 = list_create();
			if (repository1 == NULL)
			{
				fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        		return -1;
			}
            
			node_block_t *bas2 = (node_block_t *)nas2->value;
            ilist_t *a2;
            for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_ARGUMENT)
                {
					uint64_t cnt1 = 0;
                    cnt2 += 1;
					node_argument_t *argument2 = (node_argument_t *)item2->value;

					node_block_t *bps1 = (node_block_t *)nps1->value;

					ilist_t *a1;
					for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
					{
						node_t *item1 = (node_t *)a1->value;
						if (item1->kind == NODE_KIND_PARAMETER)
						{
							cnt1 += 1;
							
							node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

							if (argument2->value == NULL)
							{
								if (cnt1 < cnt2)
								{
									continue;
								}

								if (use_by_key == 1)
								{
									semantic_error(program, item1, "fields format");
									return -1;
								}

								ilist_t *que1 = list_rpush(repository1, item1);
								if (que1 == NULL)
								{
									fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
									return -1;
								}

								if (parameter1->type != NULL)
								{
									list_t *response1 = list_create();
									if (response1 == NULL)
									{
										fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}
									
									int32_t r1 = semantic_resolve(program, NULL, parameter1->type, response1, SEMANTIC_FLAG_NONE);
									if (r1 == -1)
									{
										return -1;
									}
									else
									if (r1 == 0)
									{
										semantic_error(program, parameter1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
										return -1;
									}
									else
									if (r1 == 1)
									{
										ilist_t *b1;
										for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
										{
											node_t *item3 = (node_t *)b1->value;

											list_t *response2 = list_create();
											if (response2 == NULL)
											{
												fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
												return -1;
											}
											
											int32_t r2 = semantic_expression(program, NULL, argument2->key, response2, SEMANTIC_FLAG_NONE);
											if (r2 == -1)
											{
												return -1;
											}
											else
											if (r2 == 0)
											{
												semantic_error(program, argument2->key, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
												return -1;
											}
											else
											if (r2 == 1)
											{
												ilist_t *b2;
												for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
												{
													node_t *item4 = (node_t *)b2->value;
													if (item3->id != item4->id)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}
											}
											list_destroy(response2);
										}
										list_destroy(response1);
									}
								}
								break;
							}
							else
							{
								if (argument2->key->kind != NODE_KIND_ID)
								{
									semantic_error(program, item1, "not an key id");
									return -1;
								}
								use_by_key = 1;

								if (semantic_idcmp(argument2->key, parameter1->key) == 1)
								{
									ilist_t *que1 = list_rpush(repository1, item1);
									if (que1 == NULL)
									{
										fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}

									if (parameter1->type != NULL)
									{
										list_t *response1 = list_create();
										if (response1 == NULL)
										{
											fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
											return -1;
										}
										
										int32_t r1 = semantic_resolve(program, NULL, parameter1->type, response1, SEMANTIC_FLAG_NONE);
										if (r1 == -1)
										{
											return -1;
										}
										else
										if (r1 == 0)
										{
											semantic_error(program, parameter1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
											return -1;
										}
										else
										if (r1 == 1)
										{
											ilist_t *b1;
											for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
											{
												node_t *item3 = (node_t *)b1->value;

												list_t *response2 = list_create();
												if (response2 == NULL)
												{
													fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
													return -1;
												}
												
												int32_t r2 = semantic_expression(program, NULL, argument2->value, response2, SEMANTIC_FLAG_NONE);
												if (r2 == -1)
												{
													return -1;
												}
												else
												if (r2 == 0)
												{
													semantic_error(program, argument2->value, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
													return -1;
												}
												else
												if (r2 == 1)
												{
													ilist_t *b2;
													for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
													{
														node_t *item4 = (node_t *)b2->value;
														if (item3->id != item4->id)
														{
															list_destroy(response2);
															list_destroy(response1);
															return 0;
														}
													}
												}
												list_destroy(response2);
											}
										
											list_destroy(response1);
										}
									}
								}
								
								break;
							}
						}
					}
                }
            }

            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
					node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

					int32_t found = 0;
					ilist_t *b1;
					for (b1 = repository1->begin;b1 != repository1->end;b1 = b1->next)
					{
						node_t *item2 = (node_t *)b1->value;
						if (item2->kind == NODE_KIND_PARAMETER)
						{
							if (item1->id == item2->id)
							{
								found = 1;
							}
						}
					}

					if (found == 0)
					{
						if (parameter1->value == NULL)
						{
							list_destroy(repository1);
							return 0;
						}
					}
                }
            }

			list_destroy(repository1);
            return 1;
        }
    }
    return 0;
}

static int32_t
semantic_eqaul_gsfs(program_t *program, node_t *ngs1, node_t *nfs2)
{
    if (ngs1 == NULL)
    {
        if (nfs2 == NULL)
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
        if (nfs2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    if (generic1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt2 = 0;
			int32_t use_by_key = 0;

			list_t *repository1 = list_create();
			if (repository1 == NULL)
			{
				fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        		return -1;
			}
            
			node_block_t *bfs2 = (node_block_t *)nfs2->value;
            ilist_t *a2;
            for (a2 = bfs2->list->begin;a2 != bfs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_FIELD)
                {
					uint64_t cnt1 = 0;
                    cnt2 += 1;
					node_field_t *field2 = (node_field_t *)item2->value;

					node_block_t *bgs1 = (node_block_t *)ngs1->value;

					ilist_t *a1;
					for (a1 = bgs1->list->begin;a1 != bgs1->list->end;a1 = a1->next)
					{
						node_t *item1 = (node_t *)a1->value;
						if (item1->kind == NODE_KIND_GENERIC)
						{
							cnt1 += 1;
							
							node_generic_t *generic1 = (node_generic_t *)item1->value;

							if (field2->value == NULL)
							{
								if (cnt1 < cnt2)
								{
									continue;
								}

								if (use_by_key == 1)
								{
									semantic_error(program, item1, "fields format");
									return -1;
								}

								ilist_t *que1 = list_rpush(repository1, item1);
								if (que1 == NULL)
								{
									fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
									return -1;
								}

								if (generic1->type != NULL)
								{
									list_t *response1 = list_create();
									if (response1 == NULL)
									{
										fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}
									
									int32_t r1 = semantic_resolve(program, NULL, generic1->type, response1, SEMANTIC_FLAG_NONE);
									if (r1 == -1)
									{
										return -1;
									}
									else
									if (r1 == 0)
									{
										semantic_error(program, generic1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
										return -1;
									}
									else
									if (r1 == 1)
									{
										ilist_t *b1;
										for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
										{
											node_t *item3 = (node_t *)b1->value;

											list_t *response2 = list_create();
											if (response2 == NULL)
											{
												fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
												return -1;
											}
											
											int32_t r2 = semantic_resolve(program, NULL, field2->key, response2, SEMANTIC_FLAG_NONE);
											if (r2 == -1)
											{
												return -1;
											}
											else
											if (r2 == 0)
											{
												semantic_error(program, field2->key, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
												return -1;
											}
											else
											if (r2 == 1)
											{
												ilist_t *b2;
												for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
												{
													node_t *item4 = (node_t *)b2->value;
													int32_t r3 = semantic_subset(program, item3, item4);
													if (r3 == -1)
													{
														return -1;
													}
													else
													if (r3 == 0)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}
											}
											list_destroy(response2);
										}
									
										list_destroy(response1);
									}
								}
								break;
							}
							else
							{
								if (field2->key->kind != NODE_KIND_ID)
								{
									semantic_error(program, item1, "not an key id");
									return -1;
								}
								use_by_key = 1;

								if (semantic_idcmp(field2->key, generic1->key) == 1)
								{
									ilist_t *que1 = list_rpush(repository1, item1);
									if (que1 == NULL)
									{
										fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}

									if (generic1->type != NULL)
									{
										list_t *response1 = list_create();
										if (response1 == NULL)
										{
											fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
											return -1;
										}
										
										int32_t r1 = semantic_resolve(program, NULL, generic1->type, response1, SEMANTIC_FLAG_NONE);
										if (r1 == -1)
										{
											return -1;
										}
										else
										if (r1 == 0)
										{
											semantic_error(program, generic1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
											return -1;
										}
										else
										if (r1 == 1)
										{
											ilist_t *b1;
											for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
											{
												node_t *item3 = (node_t *)b1->value;

												list_t *response2 = list_create();
												if (response2 == NULL)
												{
													fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
													return -1;
												}
												
												int32_t r2 = semantic_resolve(program, NULL, field2->value, response2, SEMANTIC_FLAG_NONE);
												if (r2 == -1)
												{
													return -1;
												}
												else
												if (r2 == 0)
												{
													semantic_error(program, field2->value, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
													return -1;
												}
												else
												if (r2 == 1)
												{
													ilist_t *b2;
													for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
													{
														node_t *item4 = (node_t *)b2->value;
														int32_t r3 = semantic_subset(program, item3, item4);
														if (r3 == -1)
														{
															return -1;
														}
														else
														if (r3 == 0)
														{
															list_destroy(response2);
															list_destroy(response1);
															return 0;
														}
													}
												}
												list_destroy(response2);
											}
										
											list_destroy(response1);
										}
									}
								}

								
								break;
							}
						}
					}
                }
            }

            node_block_t *bgs1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bgs1->list->begin;a1 != bgs1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
					node_generic_t *generic1 = (node_generic_t *)item1->value;

					int32_t found = 0;
					ilist_t *b1;
					for (b1 = repository1->begin;b1 != repository1->end;b1 = b1->next)
					{
						node_t *item2 = (node_t *)b1->value;
						if (item2->kind == NODE_KIND_GENERIC)
						{
							if (item1->id == item2->id)
							{
								found = 1;
							}
						}
					}

					if (found == 0)
					{
						if (generic1->value == NULL)
						{
							list_destroy(repository1);
							return 0;
						}
					}
                }
            }

			list_destroy(repository1);
            return 1;
        }
    }
    return 1;
}



static node_t *
semantic_make_field(program_t *program, node_t *arg)
{
	node_t *node = node_create(arg->parent, arg->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	return node_make_field(node, arg, NULL);
}

static node_t *
semantic_make_fields(program_t *program, node_t *parent, uint64_t n, ...)
{
	node_t *node = node_create(parent, parent->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	list_t *fields = list_create();
	if (fields == NULL)
	{
		return NULL;
	}

    va_list arg;
    va_start(arg, n);
 
    for (uint64_t i = 0; i < n; i++)
    {
        node_t *node2 = semantic_make_field(program, (node_t *)va_arg(arg, node_t *));
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(fields, node2))
		{
			return NULL;
		}
    }
    
    va_end(arg);

	return node_make_fields(node, fields);
}

static node_t *
semantic_make_argument(program_t *program, node_t *arg)
{
	node_t *node = node_create(arg->parent, arg->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	return node_make_argument(node, arg, NULL);
}

static node_t *
semantic_make_arguments(program_t *program, node_t *parent, uint64_t n, ...)
{
	node_t *node = node_create(parent, parent->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	list_t *arguments = list_create();
	if (arguments == NULL)
	{
		return NULL;
	}

    va_list arg;
    va_start(arg, n);
 
    for (uint64_t i = 0; i < n; i++)
    {
        node_t *node2 = semantic_make_argument(program, (node_t *)va_arg(arg, node_t *));
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(arguments, node2))
		{
			return NULL;
		}
    }
    
    va_end(arg);

	return node_make_arguments(node, arguments);
}



static int32_t
semantic_select(program_t *program, node_t *root, node_t *scope, node_t *name, list_t *response, uint64_t flag)
{
    if (root->kind == NODE_KIND_CATCH)
    {
        node_catch_t *catch1 = (node_catch_t *)root->value;
        node_t *node1 = catch1->parameters;
        node_block_t *block1 = (node_block_t *)node1->value;
        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                if (parameter1->key != NULL)
                {
                    if (semantic_idcmp(parameter1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    else
    if (root->kind == NODE_KIND_FORIN)
    {
        node_forin_t *for1 = (node_forin_t *)root->value;
        node_t *node1 = for1->initializer;
        node_block_t *block1 = (node_block_t *)node1->value;
        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (var1->key != NULL)
                {
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (semantic_idcmp(var1->key, name) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            return 1;
                        }
                    }
                    else
                    {
                        node_t *node3 = var1->key;
                        node_block_t *block3 = (node_block_t *)node3->value;
                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity1 = (node_entity_t *)item3->value;
                                if (semantic_idcmp(entity1->key, name) == 1)
                                {
                                    ilist_t *r1 = list_rpush(response, item3);
                                    if (r1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    if (root->kind == NODE_KIND_FOR)
    {
        node_for_t *for1 = (node_for_t *)root->value;
        node_t *node1 = for1->initializer;
        node_block_t *block1 = (node_block_t *)node1->value;
        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (var1->key != NULL)
                {
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (semantic_idcmp(var1->key, name) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            return 1;
                        }
                    }
                    else
                    {
                        node_t *node3 = var1->key;
                        node_block_t *block3 = (node_block_t *)node3->value;
                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity1 = (node_entity_t *)item3->value;
                                if (semantic_idcmp(entity1->key, name) == 1)
                                {
                                    ilist_t *r1 = list_rpush(response, item3);
                                    if (r1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    if (root->kind == NODE_KIND_BODY)
    {
        node_block_t *block1 = (node_block_t *)root->value;
        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_FOR)
            {
                node_for_t *for1 = (node_for_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (semantic_idcmp(for1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *for1 = (node_forin_t *)item1->value;
                if (for1->key != NULL)
                {
                    if (semantic_idcmp(for1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (semantic_idcmp(var1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
                else
                {
                    node_t *node2 = var1->key;
                    node_block_t *block2 = (node_block_t *)node2->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_ENTITY)
                        {
                            node_entity_t *entity2 = (node_entity_t *)item2->value;
                            if (semantic_idcmp(entity2->key, name) == 1)
                            {
                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    if (root->kind == NODE_KIND_FUN)
    {
        node_fun_t *fun1 = (node_fun_t *)root->value;

        if (fun1->generics != NULL)
        {
            node_t *node2 = fun1->generics;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic3 = (node_generic_t *)item2->value;
                    if (semantic_idcmp(generic3->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item2);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            node_t *node2 = fun1->parameters;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter3 = (node_parameter_t *)item2->value;
                    if (semantic_idcmp(parameter3->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item2);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    else
    if (root->kind == NODE_KIND_PACKAGE)
    {
        node_package_t *package1 = (node_package_t *)root->value;
        if (package1->generics != NULL)
        {
            node_t *node1 = package1->generics;
            node_block_t *block1 = (node_block_t *)node1->value;
            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    if (semantic_idcmp(generic1->key, name) == 1)
                    { 
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    else
    if (root->kind == NODE_KIND_CLASS)
    {
        node_class_t *class1 = (node_class_t *)root->value;

        if (class1->generics != NULL)
        {
            node_t *node2 = class1->generics;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic3 = (node_generic_t *)item2->value;
                    if (semantic_idcmp(generic3->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item2);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

        if (class1->heritages != NULL)
        {
            node_t *node2 = class1->heritages;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item2->kind == NODE_KIND_HERITAGE)
                {
                    node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                    if (semantic_idcmp(heritage3->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item2);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

        node_t *node1 = class1->block;
        node_block_t *block1 = (node_block_t *)node1->value;
        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                if (semantic_idcmp(class2->key, name) == 1)
                {
                    ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (semantic_idcmp(enum1->key, name) == 1)
                {
                    ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    return 1;
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                if (semantic_idcmp(fun1->key, name) == 1)
                {
                    ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (semantic_idcmp(property1->key, name) == 1)
                {
                    ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    return 1;
                }
                continue;
            }
        }

        if (class1->heritages != NULL)
        {
            node_t *node2 = class1->heritages;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item2->kind == NODE_KIND_HERITAGE)
                {
                    node_heritage_t *heritage3 = (node_heritage_t *)item2->value;

                    if (name->id == heritage3->type->id)
                    {
                        continue;
                    }

                    if (heritage3->value_update == NULL)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        
                        int32_t r1 = semantic_resolve(program, NULL, heritage3->type, response2, SEMANTIC_FLAG_NONE);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        {
                            uint64_t cnt_response2 = 0;

                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                cnt_response2 += 1;

                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    heritage3->value_update = item3;
                                    int32_t r2 = semantic_select(program, item3, NULL, name, response, flag | SEMANTIC_FLAG_SELECT_FOLLOW);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        return 1;
                                    }
                                }
                                else
                                if (item3->kind == NODE_KIND_GENERIC)
                                {
                                    node_generic_t *generic1 = (node_generic_t *)item3->value;
                                    if (generic1->value_update != NULL)
                                    {
                                        node_t *node3 = (node_t *)generic1->value_update;
                                        if (node3->kind == NODE_KIND_CLASS)
                                        {
                                            heritage3->value_update = node3;
                                            int32_t r2 = semantic_select(program, node3, NULL, name, response, flag | SEMANTIC_FLAG_SELECT_FOLLOW);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                return 1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, item3, "Wrong type, for (%lld:%lld)\n\nInternal:%s-%u",
                                        heritage3->type->position.line, heritage3->type->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                break;
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, heritage3->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }

                        list_destroy(response2);
                    }
                    else
                    {
                        node_t *item3 = heritage3->value_update;
                        if (item3->kind == NODE_KIND_CLASS)
                        {
                            int32_t r2 = semantic_select(program, item3, NULL, name, response, flag | SEMANTIC_FLAG_SELECT_FOLLOW);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                return 1;
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
    }
    else
    if (root->kind == NODE_KIND_MODULE)
    {
        node_module_t *module1 = (node_module_t *)root->value;
        ilist_t *a1;
        for (a1 = module1->items->begin;a1 != module1->items->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;

            if (item1->kind == NODE_KIND_USING)
            {
                node_using_t *usage1 = (node_using_t *)item1->value;
                node_t *node2 = usage1->packages;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_PACKAGE)
                    {
                        node_package_t *package2 = (node_package_t *)item2->value;
                        if (semantic_idcmp(package2->key, name) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item2);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            return 1;
                        }
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                if (semantic_idcmp(class2->key, name) == 1)
                {
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    return 1;
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (semantic_idcmp(enum1->key, name) == 1)
                {
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    return 1;
                }
                continue;
            }
        }
    }
    
    if (root->parent != NULL && ((flag & SEMANTIC_FLAG_SELECT_FOLLOW) != SEMANTIC_FLAG_SELECT_FOLLOW))
    {
        if ((root->kind == NODE_KIND_CLASS) && (scope == NULL))
        {
            return semantic_select(program, root->parent, root, name, response, flag);
        }
        else
        {
            return semantic_select(program, root->parent, scope, name, response, flag);
        }
    }

    return 0;
}

static int32_t
semantic_id(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    // node_basic_t *basic1 = (node_basic_t *)node->value;
    // printf("semanic %s\n", basic1->value);
	if (scope == NULL)
	{
		int32_t r1 = semantic_select(program, node->parent, NULL, node, response, flag);
		if (r1 == -1)
		{
			return -1;
		}
	}
	else
	{
		int32_t r1 = semantic_select(program, scope, scope, node, response, flag);
		if (r1 == -1)
		{
			return -1;
		}
	}
    

    if (list_count(response) > 0)
    {
        return 1;
    }

    return 0;
}

static int32_t
semantic_number(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
semantic_char(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
semantic_string(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
semantic_null(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
semantic_array(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
semantic_dictionary(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

static int32_t
semantic_primary(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_ID)
    {
        return semantic_id(program, scope, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_NUMBER)
    {
        return semantic_number(program, scope, node, response);
    }
    else
    if (node->kind == NODE_KIND_CHAR)
    {
        return semantic_char(program, scope, node, response);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return semantic_string(program, scope, node, response);
    }
    else
    if (node->kind == NODE_KIND_NULL)
    {
        return semantic_null(program, scope, node, response);
    }
    else
    if (node->kind == NODE_KIND_ARRAY)
    {
        return semantic_array(program, scope, node, response);
    }
    else
    if (node->kind == NODE_KIND_DICTIONARY)
    {
        return semantic_dictionary(program, scope, node, response);
    }
    else
    {
        semantic_error(program, node, "unselectable");
        return -1;
    }
}

static int32_t
semantic_field(program_t *program, node_t *node, uint64_t flag)
{
    node_field_t *field1 = (node_field_t *)node->value;
    
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_FIELDS)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_FIELD)
                    {
                        if (field1->value != NULL)
                        {
                            if (field1->key->kind != NODE_KIND_ID)
                            {
                                semantic_error(program, field1->key, "Not a 'Key'");
                                return -1;
                            }

                            node_field_t *field3 = (node_field_t *)item2->value;
                            if (field3->value != NULL)
                            {
                                if (field3->key->kind != NODE_KIND_ID)
                                {
                                    semantic_error(program, field3->key, "Not a 'Key'");
                                    return -1;
                                }
                                if (semantic_idcmp(field1->key, field3->key) == 1)
                                {
                                    semantic_error(program, field1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                        field3->key->position.line, field3->key->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }

    return 1;
}

static int32_t
semantic_fields(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *fields = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = fields->list->begin;a1 != fields->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_field(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_pseudonym(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    if (carrier->data != NULL)
    {
        int32_t r1 = semantic_fields(program, carrier->data, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, scope, carrier->base, response1, flag);
    if (r1 == -1)
    {
        return -1;
    }
    else
    {
        uint64_t cnt_response1 = 0;

        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            cnt_response1 += 1;
            
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *ngs1 = class1->generics;
                node_t *nfs2 = carrier->data;
                int32_t r1 = semantic_eqaul_gsfs(program, ngs1, nfs2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
					ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_PACKAGE)
            {
                node_package_t *package2 = (node_package_t *)item1->value;
                node_t *ngs1 = package2->generics;
                node_t *nfs2 = carrier->data;
                int32_t r2 = semantic_eqaul_gsfs(program, ngs1, nfs2);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
					ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, carrier->base, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }
    }

    list_destroy(response1);

    return 1;
}

static int32_t
semantic_attribute(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_binary_t *basic = (node_binary_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, scope, basic->left, response1, flag);
    if (r1 == -1)
    {
        return -1;
    }
    else
    {
        uint64_t cnt_response1 = 0;

        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            cnt_response1 += 1;

            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                node_t *node1 = enum1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)b1->value;
                    if (item2->kind == NODE_KIND_MEMBER)
                    {
                        node_member_t *pair2 = (node_member_t *)item2->value;
                        if (semantic_idcmp(pair2->key, basic->right) == 1)
                        {
                            ilist_t *r2 = list_rpush(response, item2);
                            if (r2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                    }
                }
                list_destroy(response1);
                return 0;
            }
            else
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                ilist_t *a2;
                for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class2 = (node_class_t *)item2->value;
                        if (semantic_idcmp(basic->right, class2->key) == 1)
                        {
                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                            {
                                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                {
                                    continue;
                                }

                                if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    continue;
                                }

                                item2->flag |= NODE_FLAG_DERIVE;
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;
                            }

                            ilist_t *r2 = list_rpush(response, item2);
                            if (r2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            continue;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item2->value;
                        if (semantic_idcmp(basic->right, enum2->key) == 1)
                        {
                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                            {
                                if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                {
                                    continue;
                                }

                                if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    continue;
                                }

                                item2->flag |= NODE_FLAG_DERIVE;
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;
                            }

                            ilist_t *r2 = list_rpush(response, item2);
                            if (r2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            continue;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;
                        if (semantic_idcmp(basic->right, fun2->key) == 1)
                        {
                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                            {
                                if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                {
                                    continue;
                                }

                                if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    continue;
                                }

                                item2->flag |= NODE_FLAG_DERIVE;
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;
                            }

                            ilist_t *r2 = list_rpush(response, item2);
                            if (r2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            continue;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;
                        if (semantic_idcmp(basic->right, property2->key) == 1)
                        {
                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                            {
                                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                {
                                    continue;
                                }

                                if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    continue;
                                }

                                item2->flag |= NODE_FLAG_DERIVE;
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;
                            }

                            ilist_t *r2 = list_rpush(response, item2);
                            if (r2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            continue;
                        }
                    }
                }

                if (class1->heritages != NULL)
                {
                    node_t *node2 = class1->heritages;

                    list_t *repository1 = list_create();
                    if (repository1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    while (node2 != NULL)
                    {
                        node_block_t *block2 = (node_block_t *)node2->value;

                        ilist_t *a3;
                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                        {
                            node_t *item2 = (node_t *)a3->value;
                            
                            if (item2->kind == NODE_KIND_HERITAGE)
                            {
                                node_heritage_t *heritage1 = (node_heritage_t *)item2->value;
                             
                                if (heritage1->value_update == NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                
                                    int32_t r2 = semantic_resolve(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    {
                                        uint64_t cnt_response2 = 0;

                                        ilist_t *a4;
                                        for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                        {
                                            cnt_response2 += 1;

                                            node_t *item3 = (node_t *)a4->value;

                                            heritage1->value_update = item3;

                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)item3->value;
                                                node_t *node3 = class2->block;
                                                node_block_t *block3 = (node_block_t *)node3->value;

                                                ilist_t *a5;
                                                for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                {
                                                    node_t *item4 = (node_t *)a5->value;
                                                    if (item4->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class3 = (node_class_t *)item4->value;
                                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item4->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item4->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r3 = list_rpush(response, item4);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                    else
                                                    if (item4->kind == NODE_KIND_ENUM)
                                                    {
                                                        node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                        if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item4->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item4->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r3 = list_rpush(response, item4);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                    else
                                                    if (item4->kind == NODE_KIND_FUN)
                                                    {
                                                        node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item4->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item4->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r3 = list_rpush(response, item4);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                    else
                                                    if (item4->kind == NODE_KIND_PROPERTY)
                                                    {
                                                        node_property_t *property1 = (node_property_t *)item4->value;
                                                        if (semantic_idcmp(property1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item4->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item4->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r3 = list_rpush(response, item4);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                }

                                                ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                            else
                                            if (item3->kind == NODE_KIND_GENERIC)
                                            {
                                                node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                if (generic1->value_update != NULL)
                                                {
                                                    node_t *item4 = (node_t *)generic1->value_update;
                                                    if (item4->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class2 = (node_class_t *)item4->value;
                                                        node_t *node3 = class2->block;
                                                        node_block_t *block3 = (node_block_t *)node3->value;

                                                        ilist_t *a5;
                                                        for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                        {
                                                            node_t *item5 = (node_t *)a5->value;
                                                            if (item5->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class3 = (node_class_t *)item5->value;
                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item5->kind == NODE_KIND_ENUM)
                                                            {
                                                                node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item5->kind == NODE_KIND_FUN)
                                                            {
                                                                node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item5->kind == NODE_KIND_PROPERTY)
                                                            {
                                                                node_property_t *property1 = (node_property_t *)item5->value;
                                                                if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                        }

                                                        ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                        if (r3 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                            heritage1->type->position.line, heritage1->type->position.column);
                                                        return -1;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                semantic_error(program, item3, "wrong type, for (%lld:%lld)",
                                                    heritage1->type->position.line, heritage1->type->position.column);
                                                return -1;
                                            }
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    node_t *node3 = heritage1->value_update;

                                    if (node3->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class2 = (node_class_t *)node3->value;
                                        node_t *node4 = class2->block;
                                        node_block_t *block3 = (node_block_t *)node4->value;

                                        ilist_t *a4;
                                        for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                        {
                                            node_t *item3 = (node_t *)a4->value;
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class3 = (node_class_t *)item3->value;
                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                {
                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                    {
                                                        if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                        {
                                                            continue;
                                                        }
                                                    }

                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            continue;
                                                        }

                                                        if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            continue;
                                                        }

                                                        item3->flag |= NODE_FLAG_DERIVE;
                                                    }
                                                    else
                                                    {
                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r2 = list_rpush(response, item3);
                                                    if (r2 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    continue;
                                                }
                                            }
                                            else
                                            if (item3->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                {
                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                    {
                                                        if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                        {
                                                            continue;
                                                        }
                                                    }

                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            continue;
                                                        }

                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            continue;
                                                        }

                                                        item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                    }
                                                    else
                                                    {
                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r2 = list_rpush(response, item3);
                                                    if (r2 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    continue;
                                                }
                                            }
                                            else
                                            if (item3->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                {
                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                    {
                                                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                        {
                                                            continue;
                                                        }
                                                    }

                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            continue;
                                                        }

                                                        if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            continue;
                                                        }

                                                        item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                    }
                                                    else
                                                    {
                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r2 = list_rpush(response, item3);
                                                    if (r2 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    continue;
                                                }
                                            }
                                            else
                                            if (item3->kind == NODE_KIND_ENUM)
                                            {
                                                node_property_t *property1 = (node_property_t *)item3->value;
                                                if (semantic_idcmp(property1->key, basic->right) == 1)
                                                {
                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                    {
                                                        if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                        {
                                                            continue;
                                                        }
                                                    }

                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            continue;
                                                        }

                                                        if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            continue;
                                                        }

                                                        item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                    }
                                                    else
                                                    {
                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r2 = list_rpush(response, item3);
                                                    if (r2 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    continue;
                                                }
                                            }
                                        }

                                        ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                        if (r3 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                            heritage1->type->position.line, heritage1->type->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    
                        ilist_t *r2 = list_rpop(repository1);
                        if (r2 != NULL)
                        {
                            node2 = (node_t *)r2->value;
                            continue;
                        }
                        else
                        {
                            node2 = NULL;
                            break;
                        }
                    }

                    list_destroy(repository1);
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (var1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *value_update = var1->value_update;
                if (value_update->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)value_update->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *a2;
                    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item2->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum2 = (node_enum_t *)item2->value;
                            if (semantic_idcmp(basic->right, enum2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item2->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item2->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                    }

                    if (class1->heritages != NULL)
                    {
                        node_t *node2 = class1->heritages;

                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (node2 != NULL)
                        {
                            node_block_t *block2 = (node_block_t *)node2->value;

                            ilist_t *a3;
                            for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                            {
                                node_t *item2 = (node_t *)a3->value;
                                
                                if (item2->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage1 = (node_heritage_t *)item2->value;
                                
                                    if (heritage1->value_update == NULL)
                                    {
                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    
                                        int32_t r2 = semantic_resolve(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *a4;
                                            for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item3 = (node_t *)a4->value;

                                                heritage1->value_update = item3;

                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class2 = (node_class_t *)item3->value;
                                                    node_t *node3 = class2->block;
                                                    node_block_t *block3 = (node_block_t *)node3->value;

                                                    ilist_t *a5;
                                                    for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                    {
                                                        node_t *item4 = (node_t *)a5->value;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class3 = (node_class_t *)item4->value;
                                                            if (semantic_idcmp(class3->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                            if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property1 = (node_property_t *)item4->value;
                                                            if (semantic_idcmp(property1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                    }

                                                    ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                    if (r3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_GENERIC)
                                                {
                                                    node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                    if (generic1->value_update != NULL)
                                                    {
                                                        node_t *item4 = (node_t *)generic1->value_update;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class2 = (node_class_t *)item4->value;
                                                            node_t *node3 = class2->block;
                                                            node_block_t *block3 = (node_block_t *)node3->value;

                                                            ilist_t *a5;
                                                            for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                            {
                                                                node_t *item5 = (node_t *)a5->value;
                                                                if (item5->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class3 = (node_class_t *)item5->value;
                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_ENUM)
                                                                {
                                                                    node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property1 = (node_property_t *)item5->value;
                                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                            }

                                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                heritage1->type->position.line, heritage1->type->position.column);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item3, "wrong type, for (%lld:%lld)",
                                                        heritage1->type->position.line, heritage1->type->position.column);
                                                    return -1;
                                                }
                                            }

                                            if (cnt_response2 == 0)
                                            {
                                                semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                    else
                                    {
                                        node_t *node3 = heritage1->value_update;

                                        if (node3->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class2 = (node_class_t *)node3->value;
                                            node_t *node4 = class2->block;
                                            node_block_t *block3 = (node_block_t *)node4->value;

                                            ilist_t *a4;
                                            for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                            {
                                                node_t *item3 = (node_t *)a4->value;
                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class3 = (node_class_t *)item3->value;
                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_FUN)
                                                {
                                                    node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_property_t *property1 = (node_property_t *)item3->value;
                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                            }

                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                            if (r3 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                                heritage1->type->position.line, heritage1->type->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        
                            ilist_t *r2 = list_rpop(repository1);
                            if (r2 != NULL)
                            {
                                node2 = (node_t *)r2->value;
                                continue;
                            }
                            else
                            {
                                node2 = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
                else
                {
                    semantic_error(program, value_update, "not accessable, in (%lld:%lld)",
                        node->position.line, node->position.column);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_ENTITY)
            {
                node_entity_t *entity1 = (node_entity_t *)item1->value;
                if (entity1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *value_update = entity1->value_update;
                if (value_update->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)value_update->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *a2;
                    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item2->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum2 = (node_enum_t *)item2->value;
                            if (semantic_idcmp(basic->right, enum2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item2->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item2->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                    }

                    if (class1->heritages != NULL)
                    {
                        node_t *node2 = class1->heritages;

                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (node2 != NULL)
                        {
                            node_block_t *block2 = (node_block_t *)node2->value;

                            ilist_t *a3;
                            for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                            {
                                node_t *item2 = (node_t *)a3->value;
                                
                                if (item2->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage1 = (node_heritage_t *)item2->value;
                                
                                    if (heritage1->value_update == NULL)
                                    {
                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    
                                        int32_t r2 = semantic_resolve(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *a4;
                                            for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item3 = (node_t *)a4->value;

                                                heritage1->value_update = item3;

                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class2 = (node_class_t *)item3->value;
                                                    node_t *node3 = class2->block;
                                                    node_block_t *block3 = (node_block_t *)node3->value;

                                                    ilist_t *a5;
                                                    for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                    {
                                                        node_t *item4 = (node_t *)a5->value;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class3 = (node_class_t *)item4->value;
                                                            if (semantic_idcmp(class3->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                            if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property1 = (node_property_t *)item4->value;
                                                            if (semantic_idcmp(property1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                    }

                                                    ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                    if (r3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_GENERIC)
                                                {
                                                    node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                    if (generic1->value_update != NULL)
                                                    {
                                                        node_t *item4 = (node_t *)generic1->value_update;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class2 = (node_class_t *)item4->value;
                                                            node_t *node3 = class2->block;
                                                            node_block_t *block3 = (node_block_t *)node3->value;

                                                            ilist_t *a5;
                                                            for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                            {
                                                                node_t *item5 = (node_t *)a5->value;
                                                                if (item5->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class3 = (node_class_t *)item5->value;
                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_ENUM)
                                                                {
                                                                    node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property1 = (node_property_t *)item5->value;
                                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                            }

                                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                heritage1->type->position.line, heritage1->type->position.column);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item3, "wrong type, for (%lld:%lld)",
                                                        heritage1->type->position.line, heritage1->type->position.column);
                                                    return -1;
                                                }
                                            }

                                            if (cnt_response2 == 0)
                                            {
                                                semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                    else
                                    {
                                        node_t *node3 = heritage1->value_update;

                                        if (node3->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class2 = (node_class_t *)node3->value;
                                            node_t *node4 = class2->block;
                                            node_block_t *block3 = (node_block_t *)node4->value;

                                            ilist_t *a4;
                                            for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                            {
                                                node_t *item3 = (node_t *)a4->value;
                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class3 = (node_class_t *)item3->value;
                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_FUN)
                                                {
                                                    node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_property_t *property1 = (node_property_t *)item3->value;
                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                            }

                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                            if (r3 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                                heritage1->type->position.line, heritage1->type->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        
                            ilist_t *r2 = list_rpop(repository1);
                            if (r2 != NULL)
                            {
                                node2 = (node_t *)r2->value;
                                continue;
                            }
                            else
                            {
                                node2 = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
                else
                {
                    semantic_error(program, value_update, "not accessable, in (%lld:%lld)",
                        node->position.line, node->position.column);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (property1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *value_update = property1->value_update;
                if (value_update->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)value_update->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *a2;
                    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item2->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum2 = (node_enum_t *)item2->value;
                            if (semantic_idcmp(basic->right, enum2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item2->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item2->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                    }

                    if (class1->heritages != NULL)
                    {
                        node_t *node2 = class1->heritages;

                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (node2 != NULL)
                        {
                            node_block_t *block2 = (node_block_t *)node2->value;

                            ilist_t *a3;
                            for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                            {
                                node_t *item2 = (node_t *)a3->value;
                                
                                if (item2->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage1 = (node_heritage_t *)item2->value;
                                
                                    if (heritage1->value_update == NULL)
                                    {
                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    
                                        int32_t r2 = semantic_resolve(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *a4;
                                            for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item3 = (node_t *)a4->value;

                                                heritage1->value_update = item3;

                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class2 = (node_class_t *)item3->value;
                                                    node_t *node3 = class2->block;
                                                    node_block_t *block3 = (node_block_t *)node3->value;

                                                    ilist_t *a5;
                                                    for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                    {
                                                        node_t *item4 = (node_t *)a5->value;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class3 = (node_class_t *)item4->value;
                                                            if (semantic_idcmp(class3->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                            if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property1 = (node_property_t *)item4->value;
                                                            if (semantic_idcmp(property1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                    }

                                                    ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                    if (r3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_GENERIC)
                                                {
                                                    node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                    if (generic1->value_update != NULL)
                                                    {
                                                        node_t *item4 = (node_t *)generic1->value_update;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class2 = (node_class_t *)item4->value;
                                                            node_t *node3 = class2->block;
                                                            node_block_t *block3 = (node_block_t *)node3->value;

                                                            ilist_t *a5;
                                                            for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                            {
                                                                node_t *item5 = (node_t *)a5->value;
                                                                if (item5->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class3 = (node_class_t *)item5->value;
                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_ENUM)
                                                                {
                                                                    node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property1 = (node_property_t *)item5->value;
                                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                            }

                                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                heritage1->type->position.line, heritage1->type->position.column);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item3, "wrong type, for (%lld:%lld)",
                                                        heritage1->type->position.line, heritage1->type->position.column);
                                                    return -1;
                                                }
                                            }

                                            if (cnt_response2 == 0)
                                            {
                                                semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                    else
                                    {
                                        node_t *node3 = heritage1->value_update;

                                        if (node3->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class2 = (node_class_t *)node3->value;
                                            node_t *node4 = class2->block;
                                            node_block_t *block3 = (node_block_t *)node4->value;

                                            ilist_t *a4;
                                            for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                            {
                                                node_t *item3 = (node_t *)a4->value;
                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class3 = (node_class_t *)item3->value;
                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_FUN)
                                                {
                                                    node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_property_t *property1 = (node_property_t *)item3->value;
                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                            }

                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                            if (r3 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                                heritage1->type->position.line, heritage1->type->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        
                            ilist_t *r2 = list_rpop(repository1);
                            if (r2 != NULL)
                            {
                                node2 = (node_t *)r2->value;
                                continue;
                            }
                            else
                            {
                                node2 = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
                else
                {
                    semantic_error(program, value_update, "not accessable, in (%lld:%lld)",
                        node->position.line, node->position.column);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                if (parameter1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *value_update = parameter1->value_update;
                if (value_update->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)value_update->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *a2;
                    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item2->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum2 = (node_enum_t *)item2->value;
                            if (semantic_idcmp(basic->right, enum2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item2->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item2->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        continue;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        continue;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                }

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                continue;
                            }
                        }
                    }

                    if (class1->heritages != NULL)
                    {
                        node_t *node2 = class1->heritages;

                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (node2 != NULL)
                        {
                            node_block_t *block2 = (node_block_t *)node2->value;

                            ilist_t *a3;
                            for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                            {
                                node_t *item2 = (node_t *)a3->value;
                                
                                if (item2->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage1 = (node_heritage_t *)item2->value;
                                
                                    if (heritage1->value_update == NULL)
                                    {
                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    
                                        int32_t r2 = semantic_resolve(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *a4;
                                            for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item3 = (node_t *)a4->value;

                                                heritage1->value_update = item3;

                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class2 = (node_class_t *)item3->value;
                                                    node_t *node3 = class2->block;
                                                    node_block_t *block3 = (node_block_t *)node3->value;

                                                    ilist_t *a5;
                                                    for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                    {
                                                        node_t *item4 = (node_t *)a5->value;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class3 = (node_class_t *)item4->value;
                                                            if (semantic_idcmp(class3->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                            if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                        else
                                                        if (item4->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property1 = (node_property_t *)item4->value;
                                                            if (semantic_idcmp(property1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        continue;
                                                                    }

                                                                    item4->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item4->flag &= ~NODE_FLAG_DERIVE;
                                                                }

                                                                ilist_t *r3 = list_rpush(response, item4);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                continue;
                                                            }
                                                        }
                                                    }

                                                    ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                    if (r3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_GENERIC)
                                                {
                                                    node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                    if (generic1->value_update != NULL)
                                                    {
                                                        node_t *item4 = (node_t *)generic1->value_update;
                                                        if (item4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class2 = (node_class_t *)item4->value;
                                                            node_t *node3 = class2->block;
                                                            node_block_t *block3 = (node_block_t *)node3->value;

                                                            ilist_t *a5;
                                                            for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                            {
                                                                node_t *item5 = (node_t *)a5->value;
                                                                if (item5->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class3 = (node_class_t *)item5->value;
                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_ENUM)
                                                                {
                                                                    node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item5->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property1 = (node_property_t *)item5->value;
                                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                continue;
                                                                            }

                                                                            item5->flag |= NODE_FLAG_DERIVE;
                                                                        }
                                                                        else
                                                                        {
                                                                            item5->flag &= ~NODE_FLAG_DERIVE;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item5);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                            }

                                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                heritage1->type->position.line, heritage1->type->position.column);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item3, "wrong type, for (%lld:%lld)",
                                                        heritage1->type->position.line, heritage1->type->position.column);
                                                    return -1;
                                                }
                                            }

                                            if (cnt_response2 == 0)
                                            {
                                                semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                    else
                                    {
                                        node_t *node3 = heritage1->value_update;

                                        if (node3->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class2 = (node_class_t *)node3->value;
                                            node_t *node4 = class2->block;
                                            node_block_t *block3 = (node_block_t *)node4->value;

                                            ilist_t *a4;
                                            for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                            {
                                                node_t *item3 = (node_t *)a4->value;
                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class3 = (node_class_t *)item3->value;
                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                    if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_FUN)
                                                {
                                                    node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                                else
                                                if (item3->kind == NODE_KIND_ENUM)
                                                {
                                                    node_property_t *property1 = (node_property_t *)item3->value;
                                                    if (semantic_idcmp(property1->key, basic->right) == 1)
                                                    {
                                                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                            {
                                                                continue;
                                                            }
                                                        }

                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                        {
                                                            if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                            {
                                                                continue;
                                                            }

                                                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                            {
                                                                continue;
                                                            }

                                                            item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                        }
                                                        else
                                                        {
                                                            item3->flag &= ~NODE_FLAG_DERIVE;
                                                        }

                                                        ilist_t *r2 = list_rpush(response, item3);
                                                        if (r2 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        continue;
                                                    }
                                                }
                                            }

                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                            if (r3 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                                heritage1->type->position.line, heritage1->type->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        
                            ilist_t *r2 = list_rpop(repository1);
                            if (r2 != NULL)
                            {
                                node2 = (node_t *)r2->value;
                                continue;
                            }
                            else
                            {
                                node2 = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
                else
                {
                    semantic_error(program, value_update, "not accessable, in (%lld:%lld)",
                        node->position.line, node->position.column);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                if (heritage1->value_update == NULL)
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                
                    int32_t r2 = semantic_resolve(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        uint64_t cnt_response2 = 0;

                        ilist_t *a2;
                        for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                        {
                            cnt_response2 += 1;

                            node_t *item2 = (node_t *)a2->value;

                            if (item2->kind == NODE_KIND_CLASS)
                            {
                                heritage1->value_update = item2;

                                node_class_t *class1 = (node_class_t *)item2->value;
                                node_t *node1 = class1->block;
                                node_block_t *block1 = (node_block_t *)node1->value;
                                ilist_t *a3;
                                for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                {
                                    node_t *item3 = (node_t *)a3->value;
                                    if (item3->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class2 = (node_class_t *)item3->value;
                                        if (semantic_idcmp(basic->right, class2->key) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    continue;
                                                }

                                                if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                {
                                                    continue;
                                                }

                                                item3->flag |= NODE_FLAG_DERIVE;
                                            }
                                            else
                                            {
                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                            }

                                            ilist_t *r2 = list_rpush(response, item3);
                                            if (r2 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            continue;
                                        }
                                    }
                                    else
                                    if (item3->kind == NODE_KIND_ENUM)
                                    {
                                        node_enum_t *enum2 = (node_enum_t *)item3->value;
                                        if (semantic_idcmp(basic->right, enum2->key) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    continue;
                                                }

                                                if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                {
                                                    continue;
                                                }

                                                item3->flag |= NODE_FLAG_DERIVE;
                                            }
                                            else
                                            {
                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                            }

                                            ilist_t *r2 = list_rpush(response, item3);
                                            if (r2 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            continue;
                                        }
                                    }
                                    else
                                    if (item3->kind == NODE_KIND_FUN)
                                    {
                                        node_fun_t *fun2 = (node_fun_t *)item3->value;
                                        if (semantic_idcmp(basic->right, fun2->key) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    continue;
                                                }

                                                if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                {
                                                    continue;
                                                }

                                                item3->flag |= NODE_FLAG_DERIVE;
                                            }
                                            else
                                            {
                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                            }

                                            ilist_t *r2 = list_rpush(response, item3);
                                            if (r2 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            continue;
                                        }
                                    }
                                    else
                                    if (item3->kind == NODE_KIND_PROPERTY)
                                    {
                                        node_property_t *property2 = (node_property_t *)item3->value;
                                        if (semantic_idcmp(basic->right, property2->key) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    continue;
                                                }

                                                if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                {
                                                    continue;
                                                }

                                                item3->flag |= NODE_FLAG_DERIVE;
                                            }
                                            else
                                            {
                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                            }

                                            ilist_t *r2 = list_rpush(response, item3);
                                            if (r2 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            continue;
                                        }
                                    }
                                }

                                if (class1->heritages != NULL)
                                {
                                    node_t *node2 = class1->heritages;

                                    list_t *repository1 = list_create();
                                    if (repository1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    while (node2 != NULL)
                                    {
                                        node_block_t *block2 = (node_block_t *)node2->value;

                                        ilist_t *a4;
                                        for (a4 = block2->list->begin;a4 != block2->list->end;a4 = a4->next)
                                        {
                                            node_t *item3 = (node_t *)a4->value;
                                            
                                            if (item3->kind == NODE_KIND_HERITAGE)
                                            {
                                                node_heritage_t *heritage2 = (node_heritage_t *)item3->value;
                                            
                                                if (heritage2->value_update == NULL)
                                                {
                                                    list_t *response3 = list_create();
                                                    if (response3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                
                                                    int32_t r3 = semantic_resolve(program, NULL, heritage2->type, response3, SEMANTIC_FLAG_NONE);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    {
                                                        uint64_t cnt_response3 = 0;

                                                        ilist_t *a5;
                                                        for (a5 = response3->begin;a5 != response3->end;a5 = a5->next)
                                                        {
                                                            cnt_response3 += 1;

                                                            node_t *item4 = (node_t *)a5->value;

                                                            heritage2->value_update = item4;

                                                            if (item4->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class2 = (node_class_t *)item3->value;
                                                                node_t *node3 = class2->block;
                                                                node_block_t *block3 = (node_block_t *)node3->value;

                                                                ilist_t *a6;
                                                                for (a6 = block3->list->begin;a6 != block3->list->end;a6 = a6->next)
                                                                {
                                                                    node_t *item5 = (node_t *)a6->value;
                                                                    if (item5->kind == NODE_KIND_CLASS)
                                                                    {
                                                                        node_class_t *class3 = (node_class_t *)item5->value;
                                                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                    else
                                                                    if (item5->kind == NODE_KIND_ENUM)
                                                                    {
                                                                        node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                        if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                    else
                                                                    if (item5->kind == NODE_KIND_FUN)
                                                                    {
                                                                        node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                    else
                                                                    if (item5->kind == NODE_KIND_PROPERTY)
                                                                    {
                                                                        node_property_t *property1 = (node_property_t *)item5->value;
                                                                        if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                }

                                                                ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_GENERIC)
                                                            {
                                                                node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                                if (generic1->value_update != NULL)
                                                                {
                                                                    node_t *item4 = (node_t *)generic1->value_update;
                                                                    if (item4->kind == NODE_KIND_CLASS)
                                                                    {
                                                                        node_class_t *class2 = (node_class_t *)item4->value;
                                                                        node_t *node3 = class2->block;
                                                                        node_block_t *block3 = (node_block_t *)node3->value;

                                                                        ilist_t *a5;
                                                                        for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                                        {
                                                                            node_t *item5 = (node_t *)a5->value;
                                                                            if (item5->kind == NODE_KIND_CLASS)
                                                                            {
                                                                                node_class_t *class3 = (node_class_t *)item5->value;
                                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                {
                                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                                    {
                                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                                    }

                                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                                    if (r3 == NULL)
                                                                                    {
                                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                    continue;
                                                                                }
                                                                            }
                                                                            else
                                                                            if (item5->kind == NODE_KIND_ENUM)
                                                                            {
                                                                                node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                                {
                                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                                    {
                                                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                                    }

                                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                                    if (r3 == NULL)
                                                                                    {
                                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                    continue;
                                                                                }
                                                                            }
                                                                            else
                                                                            if (item5->kind == NODE_KIND_FUN)
                                                                            {
                                                                                node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                {
                                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                                    {
                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                                    }

                                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                                    if (r3 == NULL)
                                                                                    {
                                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                    continue;
                                                                                }
                                                                            }
                                                                            else
                                                                            if (item5->kind == NODE_KIND_PROPERTY)
                                                                            {
                                                                                node_property_t *property1 = (node_property_t *)item5->value;
                                                                                if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                                {
                                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                                    {
                                                                                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                        {
                                                                                            continue;
                                                                                        }

                                                                                        item5->flag |= NODE_FLAG_DERIVE;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        item5->flag &= ~NODE_FLAG_DERIVE;
                                                                                    }

                                                                                    ilist_t *r3 = list_rpush(response, item5);
                                                                                    if (r3 == NULL)
                                                                                    {
                                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                    continue;
                                                                                }
                                                                            }
                                                                        }

                                                                        ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                            heritage1->type->position.line, heritage1->type->position.column);
                                                                        return -1;
                                                                    }
                                                                }
                                                            }
                                                            else
                                                            {
                                                                semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                    heritage2->type->position.line, heritage2->type->position.column);
                                                                return -1;
                                                            }
                                                        }

                                                        if (cnt_response3 == 0)
                                                        {
                                                            semantic_error(program, heritage2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }

                                                    list_destroy(response2);
                                                }
                                                else
                                                {
                                                    node_t *node3 = heritage2->value_update;

                                                    if (node3->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class2 = (node_class_t *)node3->value;
                                                        node_t *node4 = class2->block;
                                                        node_block_t *block3 = (node_block_t *)node4->value;

                                                        ilist_t *a4;
                                                        for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                                        {
                                                            node_t *item3 = (node_t *)a4->value;
                                                            if (item3->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class3 = (node_class_t *)item3->value;
                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                                    {
                                                                        if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            continue;
                                                                        }
                                                                    }

                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item3->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r2 = list_rpush(response, item3);
                                                                    if (r2 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item3->kind == NODE_KIND_ENUM)
                                                            {
                                                                node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                                    {
                                                                        if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            continue;
                                                                        }
                                                                    }

                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                                    }
                                                                    else
                                                                    {
                                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r2 = list_rpush(response, item3);
                                                                    if (r2 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item3->kind == NODE_KIND_FUN)
                                                            {
                                                                node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                                    {
                                                                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            continue;
                                                                        }
                                                                    }

                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                                    }
                                                                    else
                                                                    {
                                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r2 = list_rpush(response, item3);
                                                                    if (r2 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item3->kind == NODE_KIND_ENUM)
                                                            {
                                                                node_property_t *property1 = (node_property_t *)item3->value;
                                                                if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                                    {
                                                                        if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            continue;
                                                                        }
                                                                    }

                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                                    }
                                                                    else
                                                                    {
                                                                        item3->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r2 = list_rpush(response, item3);
                                                                    if (r2 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                        }

                                                        ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                        if (r3 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                                            heritage2->type->position.line, heritage2->type->position.column);
                                                        return -1;
                                                    }
                                                }
                                            }
                                        }
                                    
                                        ilist_t *r2 = list_rpop(repository1);
                                        if (r2 != NULL)
                                        {
                                            node2 = (node_t *)r2->value;
                                            continue;
                                        }
                                        else
                                        {
                                            node2 = NULL;
                                            break;
                                        }
                                    }

                                    list_destroy(repository1);
                                }
                            }
                            else
                            {
                                semantic_error(program, item2, "wrong type, for (%lld:%lld)",
                                    heritage1->type->position.line, heritage1->type->position.column);
                                return -1;
                            }
                            break;
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
                else
                {
                    node_t *value_update = heritage1->value_update;

                    if (value_update->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class1 = (node_class_t *)value_update->value;
                        node_t *node1 = class1->block;
                        node_block_t *block1 = (node_block_t *)node1->value;
                        ilist_t *a2;
                        for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                        {
                            node_t *item2 = (node_t *)a2->value;
                            if (item2->kind == NODE_KIND_CLASS)
                            {
                                node_class_t *class2 = (node_class_t *)item2->value;
                                if (semantic_idcmp(basic->right, class2->key) == 1)
                                {
                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                    {
                                        if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                        {
                                            continue;
                                        }

                                        if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                        {
                                            continue;
                                        }

                                        item2->flag |= NODE_FLAG_DERIVE;
                                    }
                                    else
                                    {
                                        item2->flag &= ~NODE_FLAG_DERIVE;
                                    }

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    continue;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_ENUM)
                            {
                                node_enum_t *enum2 = (node_enum_t *)item2->value;
                                if (semantic_idcmp(basic->right, enum2->key) == 1)
                                {
                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                    {
                                        if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                        {
                                            continue;
                                        }

                                        if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                        {
                                            continue;
                                        }

                                        item2->flag |= NODE_FLAG_DERIVE;
                                    }
                                    else
                                    {
                                        item2->flag &= ~NODE_FLAG_DERIVE;
                                    }

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    continue;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_FUN)
                            {
                                node_fun_t *fun2 = (node_fun_t *)item2->value;
                                if (semantic_idcmp(basic->right, fun2->key) == 1)
                                {
                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                    {
                                        if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                        {
                                            continue;
                                        }

                                        if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                        {
                                            continue;
                                        }

                                        item2->flag |= NODE_FLAG_DERIVE;
                                    }
                                    else
                                    {
                                        item2->flag &= ~NODE_FLAG_DERIVE;
                                    }

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    continue;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item2->value;
                                if (semantic_idcmp(basic->right, property2->key) == 1)
                                {
                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                    {
                                        if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                        {
                                            continue;
                                        }

                                        if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                        {
                                            continue;
                                        }

                                        item2->flag |= NODE_FLAG_DERIVE;
                                    }
                                    else
                                    {
                                        item2->flag &= ~NODE_FLAG_DERIVE;
                                    }

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    continue;
                                }
                            }
                        }

                        if (class1->heritages != NULL)
                        {
                            node_t *node2 = class1->heritages;

                            list_t *repository1 = list_create();
                            if (repository1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            while (node2 != NULL)
                            {
                                node_block_t *block2 = (node_block_t *)node2->value;

                                ilist_t *a3;
                                for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                {
                                    node_t *item2 = (node_t *)a3->value;
                                    
                                    if (item2->kind == NODE_KIND_HERITAGE)
                                    {
                                        node_heritage_t *heritage2 = (node_heritage_t *)item2->value;
                                    
                                        if (heritage2->value_update == NULL)
                                        {
                                            list_t *response2 = list_create();
                                            if (response2 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        
                                            int32_t r2 = semantic_resolve(program, NULL, heritage2->type, response2, SEMANTIC_FLAG_NONE);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            {
                                                uint64_t cnt_response2 = 0;

                                                ilist_t *a4;
                                                for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                                {
                                                    cnt_response2 += 1;

                                                    node_t *item3 = (node_t *)a4->value;

                                                    heritage2->value_update = item3;

                                                    if (item3->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class2 = (node_class_t *)item3->value;
                                                        node_t *node3 = class2->block;
                                                        node_block_t *block3 = (node_block_t *)node3->value;

                                                        ilist_t *a5;
                                                        for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                        {
                                                            node_t *item4 = (node_t *)a5->value;
                                                            if (item4->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class3 = (node_class_t *)item4->value;
                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item4->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item4->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item4);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_ENUM)
                                                            {
                                                                node_enum_t *enum1 = (node_enum_t *)item4->value;
                                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item4->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item4->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item4);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_FUN)
                                                            {
                                                                node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item4->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item4->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item4);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_PROPERTY)
                                                            {
                                                                node_property_t *property1 = (node_property_t *)item4->value;
                                                                if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                {
                                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                    {
                                                                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                        {
                                                                            continue;
                                                                        }

                                                                        item4->flag |= NODE_FLAG_DERIVE;
                                                                    }
                                                                    else
                                                                    {
                                                                        item4->flag &= ~NODE_FLAG_DERIVE;
                                                                    }

                                                                    ilist_t *r3 = list_rpush(response, item4);
                                                                    if (r3 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                        }

                                                        ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                        if (r3 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                    else
                                                    if (item3->kind == NODE_KIND_GENERIC)
                                                    {
                                                        node_generic_t *generic1 = (node_generic_t *)item3->value;
                                                        if (generic1->value_update != NULL)
                                                        {
                                                            node_t *item4 = (node_t *)generic1->value_update;
                                                            if (item4->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class2 = (node_class_t *)item4->value;
                                                                node_t *node3 = class2->block;
                                                                node_block_t *block3 = (node_block_t *)node3->value;

                                                                ilist_t *a5;
                                                                for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                                {
                                                                    node_t *item5 = (node_t *)a5->value;
                                                                    if (item5->kind == NODE_KIND_CLASS)
                                                                    {
                                                                        node_class_t *class3 = (node_class_t *)item5->value;
                                                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                    else
                                                                    if (item5->kind == NODE_KIND_ENUM)
                                                                    {
                                                                        node_enum_t *enum1 = (node_enum_t *)item5->value;
                                                                        if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                    else
                                                                    if (item5->kind == NODE_KIND_FUN)
                                                                    {
                                                                        node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                    else
                                                                    if (item5->kind == NODE_KIND_PROPERTY)
                                                                    {
                                                                        node_property_t *property1 = (node_property_t *)item5->value;
                                                                        if (semantic_idcmp(property1->key, basic->right) == 1)
                                                                        {
                                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                            {
                                                                                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                                {
                                                                                    continue;
                                                                                }

                                                                                item5->flag |= NODE_FLAG_DERIVE;
                                                                            }
                                                                            else
                                                                            {
                                                                                item5->flag &= ~NODE_FLAG_DERIVE;
                                                                            }

                                                                            ilist_t *r3 = list_rpush(response, item5);
                                                                            if (r3 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            continue;
                                                                        }
                                                                    }
                                                                }

                                                                ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                                if (r3 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                semantic_error(program, item4, "wrong type, for (%lld:%lld)",
                                                                    heritage2->type->position.line, heritage2->type->position.column);
                                                                return -1;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        semantic_error(program, item3, "wrong type, for (%lld:%lld)",
                                                            heritage2->type->position.line, heritage2->type->position.column);
                                                        return -1;
                                                    }
                                                }

                                                if (cnt_response2 == 0)
                                                {
                                                    semantic_error(program, heritage2->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }

                                            list_destroy(response2);
                                        }
                                        else
                                        {
                                            node_t *node3 = heritage2->value_update;

                                            if (node3->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)node3->value;
                                                node_t *node4 = class2->block;
                                                node_block_t *block3 = (node_block_t *)node4->value;

                                                ilist_t *a4;
                                                for (a4 = block3->list->begin;a4 != block3->list->end;a4 = a4->next)
                                                {
                                                    node_t *item3 = (node_t *)a4->value;
                                                    if (item3->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class3 = (node_class_t *)item3->value;
                                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                            {
                                                                if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                {
                                                                    continue;
                                                                }
                                                            }

                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item3->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r2 = list_rpush(response, item3);
                                                            if (r2 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                    else
                                                    if (item3->kind == NODE_KIND_ENUM)
                                                    {
                                                        node_enum_t *enum1 = (node_enum_t *)item3->value;
                                                        if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                            {
                                                                if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                {
                                                                    continue;
                                                                }
                                                            }

                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                            }
                                                            else
                                                            {
                                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r2 = list_rpush(response, item3);
                                                            if (r2 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                    else
                                                    if (item3->kind == NODE_KIND_FUN)
                                                    {
                                                        node_fun_t *fun1 = (node_fun_t *)item3->value;
                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                            {
                                                                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                {
                                                                    continue;
                                                                }
                                                            }

                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                            }
                                                            else
                                                            {
                                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r2 = list_rpush(response, item3);
                                                            if (r2 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                    else
                                                    if (item3->kind == NODE_KIND_ENUM)
                                                    {
                                                        node_property_t *property1 = (node_property_t *)item3->value;
                                                        if (semantic_idcmp(property1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                            {
                                                                if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                {
                                                                    continue;
                                                                }
                                                            }

                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    continue;
                                                                }

                                                                if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    continue;
                                                                }

                                                                item3->flag |= NODE_FLAG_DERIVE;                                                   
                                                            }
                                                            else
                                                            {
                                                                item3->flag &= ~NODE_FLAG_DERIVE;
                                                            }

                                                            ilist_t *r2 = list_rpush(response, item3);
                                                            if (r2 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                            continue;
                                                        }
                                                    }
                                                }

                                                ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                            else
                                            {
                                                semantic_error(program, node3, "wrong type, for (%lld:%lld)",
                                                    heritage2->type->position.line, heritage2->type->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            
                                ilist_t *r2 = list_rpop(repository1);
                                if (r2 != NULL)
                                {
                                    node2 = (node_t *)r2->value;
                                    continue;
                                }
                                else
                                {
                                    node2 = NULL;
                                    break;
                                }
                            }

                            list_destroy(repository1);
                        }
                    }
                    else
                    {
                        semantic_error(program, value_update, "not accessable, in (%lld:%lld)",
                            node->position.line, node->position.column);
                        return -1;
                    }
                }
            }
            else
			{
				semantic_error(program, item1, "wrong access, in (%lld:%lld)",
					node->position.line, node->position.column);
				return -1;
			}
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, basic->left, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }
    }

    list_destroy(response1);

    if (list_count(response) > 0)
    {
        return 1;
    }

    return 0;
}

static int32_t
semantic_call(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    node_t *base1 = carrier->base;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, scope, base1, response1, flag | SEMANTIC_FLAG_SELECT_OBJECT);
    if (r1 == -1)
    {
        return -1;
    }
    else
    {
        uint64_t cnt_response1 = 0;

        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            cnt_response1 += 1;

            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                {
                    semantic_error(program, item1, "Wrong call, for (%lld:%lld)\n\nInternal %s-%u",
                        base1->position.line, base1->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)b1->value;
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun1 = (node_fun_t *)item2->value;
                        if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                        {
                            node_t *nps1 = fun1->parameters;
                            node_t *nds2 = carrier->data;
                            int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            {
                                ilist_t *il1 = list_rpush(response, item1);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                node_t *nps1 = fun1->parameters;
                node_t *nds2 = carrier->data;
                int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SEMANTIC_FLAG_NONE);
                    if (r3 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        uint64_t cnt_response2 = 0;

                        ilist_t *a3;
                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                        {
                            cnt_response2 += 1;

                            node_t *item3 = (node_t *)a3->value;

                            ilist_t *il1 = list_rpush(response, item3);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            list_destroy(response1);
                            return 1;
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, fun1->result, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_LAMBDA)
            {
                node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                node_t *nps1 = fun1->parameters;
                node_t *nds2 = carrier->data;
                int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SEMANTIC_FLAG_NONE);
                    if (r3 == -1)
                    {
                        return -1;
                    }            
                    else
                    {
                        uint64_t cnt_response2 = 0;

                        ilist_t *a3;
                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                        {
                            cnt_response2 += 1;

                            node_t *item3 = (node_t *)a3->value;

                            if (item3->kind == NODE_KIND_CLASS)
                            {
                                item3->flag |= NODE_FLAG_NEW;
                            }

                            ilist_t *il1 = list_rpush(response, item3);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, fun1->result, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (property1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *value_update = property1->value_update;
                if (value_update->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)value_update->value;

                    node_t *nps1 = fun1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SEMANTIC_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        {
                            uint64_t cnt_response2 = 0;

                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                cnt_response2 += 1;

                                node_t *item3 = (node_t *)a3->value;

                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    item3->flag |= NODE_FLAG_NEW;
                                }

                                ilist_t *il1 = list_rpush(response, item3);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, fun1->result, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (value_update->kind == NODE_KIND_LAMBDA)
                {
                    node_lambda_t *fun1 = (node_lambda_t *)value_update->value;

                    node_t *nps1 = fun1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SEMANTIC_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        {
                            uint64_t cnt_response2 = 0;

                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                cnt_response2 += 1;

                                node_t *item3 = (node_t *)a3->value;

                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    item3->flag |= NODE_FLAG_NEW;
                                }

                                ilist_t *il1 = list_rpush(response, item3);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, fun1->result, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                else
                {
                    semantic_error(program, item1, "uncallable\n\tinternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;

                if (heritage1->value_update != NULL)
                {
                    node_t *item2 = heritage1->value_update;
                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class2 = (node_class_t *)item2->value;
                        node_t *node2 = class2->block;
                        node_block_t *block2 = (node_block_t *)node2->value;

                        ilist_t *a3;
                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_FUN)
                            {
                                node_fun_t *fun1 = (node_fun_t *)item3->value;
                                if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                                {
                                    node_t *nps1 = fun1->parameters;
                                    node_t *nds2 = carrier->data;
                                    int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        ilist_t *il1 = list_rpush(response, item2);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    int32_t r2 = semantic_postfix(program, NULL, heritage1->type, response2, SEMANTIC_FLAG_NONE);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, heritage1->type, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (r2 == 1)
                    {
                        ilist_t *a2;
                        for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                        {
                            node_t *item2 = (node_t *)a2->value;
                            if (item2->kind == NODE_KIND_CLASS)
                            {
                                node_class_t *class2 = (node_class_t *)item2->value;
                                node_t *node2 = class2->block;
                                node_block_t *block2 = (node_block_t *)node2->value;

                                ilist_t *a3;
                                for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                {
                                    node_t *item3 = (node_t *)a3->value;
                                    if (item3->kind == NODE_KIND_FUN)
                                    {
                                        node_fun_t *fun1 = (node_fun_t *)item3->value;
                                        if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                                        {
                                            node_t *nps1 = fun1->parameters;
                                            node_t *nds2 = carrier->data;
                                            int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                ilist_t *il1 = list_rpush(response, item2);
                                                if (il1 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                list_destroy(response1);
                                                return 1;
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
                semantic_error(program, item1, "not callable");
                return -1;
            }
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, base1, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }
    }

    list_destroy(response1);

    return 0;
}

static int32_t
semantic_item(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, scope, carrier->base, response1, flag | SEMANTIC_FLAG_SELECT_OBJECT);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, node, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
        return -1;
    }
    else
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                {
                    semantic_error(program, item1, "unmade class");
                    return -1;
                }
                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                ilist_t *b1;
                for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                {
                    node_t *item2 = (node_t *)b1->value;
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun1 = (node_fun_t *)item2->value;
                        if (semantic_idstrcmp(fun1->key, "[]") == 1)
                        {
                            node_t *nps1 = fun1->parameters;
                            node_t *nds2 = carrier->data;
                            int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                node_t *item3 = fun1->result;
                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    ilist_t *il1 = list_rpush(response, item3);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    ilist_t *il1 = list_rpush(response, item3);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                semantic_error(program, item1, "not an enumerable");
                return -1;
            }
        }
    }

    list_destroy(response1);

    return 0;
}

static int32_t
semantic_postfix(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
	if (node->kind == NODE_KIND_CALL)
    {
        return semantic_call(program, scope, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_ITEM)
    {
        return semantic_item(program, scope, node, response, flag);
    }
    if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return semantic_attribute(program, scope, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return semantic_pseudonym(program, scope, node, response, flag);
    }
    else
    {
        return semantic_primary(program, scope, node, response, flag);
    }
}

static int32_t
semantic_resolve(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, NULL, node, response1, SEMANTIC_FLAG_NONE);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, node, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
        return -1;
    }
    else
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_PACKAGE)
            {
                node_package_t *package1 = (node_package_t *)item1->value;

                node_t *root1 = item1->parent;
                while (root1 != NULL)
                {
                    if (root1->kind == NODE_KIND_USING)
                    {
                        break;
                    }
                    else
                    {
                        root1 = root1->parent;
                    }
                }

                if (root1->kind != NODE_KIND_USING)
                {
                    semantic_error(program, root1, "import not found\n");
                    return -1;
                }

                node_using_t *usage1 = (node_using_t *)root1->value;

                node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

                node_t *module1 = program_load(program, basic2->value);
                if (module1 == NULL)
                {
                    return -1;
                }

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_postfix(program, module1, package1->address, response2, SEMANTIC_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 0)
                {
                    semantic_error(program, package1->address, "Reference not found\n\nInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    ilist_t *b1;
                    for (b1 = response2->begin;b1 != response2->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            ilist_t *il1 = list_rpush(response, item2);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                list_destroy(response2);
            }
            else
            {
                ilist_t *il1 = list_rpush(response, item1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
            }
        }
        list_destroy(response1);
        return 1;
    }
    list_destroy(response1);
    return 0;
}

static int32_t
semantic_expression(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
	return semantic_resolve(program, scope, node, response, flag);
}


static int32_t
semantic_if(program_t *program, node_t *node, uint64_t flag)
{
	return 1;
}

static int32_t
semantic_for(program_t *program, node_t *node, uint64_t flag)
{
    node_for_t *for1 = (node_for_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CATCH)
            {
                node_catch_t *catch2 = (node_catch_t *)current->value;
                node_t *node2 = catch2->parameters;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                        if (semantic_idcmp(for1->key, parameter2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                }
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_FOR)
            {
                node_for_t *for2 = (node_for_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_FORIN)
            {
                node_forin_t *for2 = (node_forin_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *for2 = (node_forin_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
            }
            else
            if (current->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun2 = (node_fun_t *)current->value;

                if (fun2->generics != NULL)
                {
                    node_t *node2 = fun2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(for1->key, generic3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    node_t *node3 = fun2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        
                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (semantic_idcmp(for1->key, parameter3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    parameter3->key->position.line, parameter3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }
    
	return 1;
}

static int32_t
semantic_forin(program_t *program, node_t *node, uint64_t flag)
{
    node_forin_t *for1 = (node_forin_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CATCH)
            {
                node_catch_t *catch2 = (node_catch_t *)current->value;
                node_t *node2 = catch2->parameters;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                        if (semantic_idcmp(for1->key, parameter2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                }
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_FOR)
            {
                node_for_t *for2 = (node_for_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_FORIN)
            {
                node_forin_t *for2 = (node_forin_t *)current->value;
                node_t *node2 = for2->initializer;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (current->kind == NODE_KIND_BODY)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FORIN)
                    {
                        node_forin_t *for2 = (node_forin_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_VAR)
                    {
                        node_var_t *var2 = (node_var_t *)item2->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(for1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
            }
            else
            if (current->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun2 = (node_fun_t *)current->value;

                if (fun2->generics != NULL)
                {
                    node_t *node2 = fun2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(for1->key, generic3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    node_t *node3 = fun2->parameters;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        
                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (semantic_idcmp(for1->key, parameter3->key) == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    parameter3->key->position.line, parameter3->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }
    return 1;
}

static int32_t
semantic_parameter(program_t *program, node_t *node, uint64_t flag)
{
    node_parameter_t *parameter1 = (node_parameter_t *)node->value;
    
    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_PARAMETERS)
        {
            node_block_t *block2 = (node_block_t *)current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter3 = (node_parameter_t *)item2->value;
                    if (semantic_idcmp(parameter1->key, parameter3->key) == 1)
                    {
                        semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                            parameter3->key->position.line, parameter3->key->position.column);
                        return -1;
                    }
                }
            }
            
            sub = current;
            current = current->parent;
            continue;
        }
        else
        if (current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)current->value;

            if (class2->generics != NULL)
            {
                node_t *node2 = class2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    
                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (semantic_idcmp(parameter1->key, generic3->key) == 1)
                        {
                            semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                                generic3->key->position.line, generic3->key->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
        if (current->kind == NODE_KIND_FUN)
        {
            node_fun_t *fun2 = (node_fun_t *)current->value;

            if (fun2->generics != NULL)
            {
                node_t *node2 = fun2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    
                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (semantic_idcmp(parameter1->key, generic3->key) == 1)
                        {
                            semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                                generic3->key->position.line, generic3->key->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
        {
            sub = current;
            current = current->parent;
            continue;
        }
    }

    return 1;
}

static int32_t
semantic_parameters(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_parameter(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_generic(program_t *program, node_t *node, uint64_t flag)
{
    node_generic_t *generic1 = (node_generic_t *)node->value;
    
    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_GENERICS)
        {
            node_block_t *block2 = (node_block_t *)current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic3 = (node_generic_t *)item2->value;
                    if (semantic_idcmp(generic1->key, generic3->key) == 1)
                    {
                        semantic_error(program, generic1->key, "already defined, previous in (%lld:%lld)",
                            generic3->key->position.line, generic3->key->position.column);
                        return -1;
                    }
                }
            }
            
            break;
        }
        else
        {
            sub = current;
            current = current->parent;
            continue;
        }
    }

    return 1;
}

static int32_t
semantic_generics(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *generics = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = generics->list->begin;a1 != generics->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_generic(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_catch(program_t *program, node_t *node, uint64_t flag)
{
    node_catch_t *catch1 = (node_catch_t *)node->value;

    if (catch1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, catch1->parameters, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_try(program_t *program, node_t *node, uint64_t flag)
{
	node_try_t *try1 = (node_try_t *)node->value;

    node_t *node2 = try1->catchs;
    node_block_t *block2 = (node_block_t *)node2->value;

    ilist_t *a1;
    for (a1 = block2->list->begin;a1 != block2->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_catch(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_var(program_t *program, node_t *node, uint64_t flag)
{
    node_var_t *var1 = (node_var_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_CATCH)
        {
            node_catch_t *catch2 = (node_catch_t *)current->value;
            node_t *node2 = catch2->parameters;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (semantic_idcmp(var1->key, parameter2->key) == 1)
                        {
                            semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        node_t *node4 = (node_t *)var1->key;
                        node_block_t *block4 = (node_block_t *)node4->value;
                        ilist_t *a4;
                        for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                        {
                            node_t *item4 = (node_t *)a4->value;
                            if (item4->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity4 = (node_entity_t *)item4->value;
                                if (semantic_idcmp(entity4->key, parameter2->key) == 1)
                                {
                                    semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                        parameter2->key->position.line, parameter2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
            }
            sub = current;
            current = current->parent;
            continue;
        }
        else
        if (current->kind == NODE_KIND_FOR)
        {
            node_for_t *for2 = (node_for_t *)current->value;
            node_t *node2 = for2->initializer;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node4 = var1->key;
                            node_block_t *block4 = (node_block_t *)node4->value;
                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                if (item4->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity4 = (node_entity_t *)item4->value;
                                    if (semantic_idcmp(entity4->key, var2->key) == 1)
                                    {
                                        semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                            var2->key->position.line, var2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        node_t *node3 = var2->key;
                        node_block_t *block3 = (node_block_t *)node3->value;
                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item3->value;
                                if (var1->key->kind == NODE_KIND_ID)
                                {
                                    if (semantic_idcmp(var1->key, entity2->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                                else
                                {
                                    node_t *node4 = (node_t *)var1->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_ENTITY)
                                        {
                                            node_entity_t *entity4 = (node_entity_t *)item4->value;
                                            if (semantic_idcmp(entity4->key, entity2->key) == 1)
                                            {
                                                semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                                    entity2->key->position.line, entity2->key->position.column);
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
            break;
        }
        else
        if (current->kind == NODE_KIND_FORIN)
        {
            node_forin_t *for2 = (node_forin_t *)current->value;
            node_t *node2 = for2->initializer;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node4 = (node_t *)var1->key;
                            node_block_t *block4 = (node_block_t *)node4->value;
                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                if (item4->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity4 = (node_entity_t *)item4->value;
                                    if (semantic_idcmp(entity4->key, var2->key) == 1)
                                    {
                                        semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                            var2->key->position.line, var2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        node_t *node3 = var2->key;
                        node_block_t *block3 = (node_block_t *)node3->value;
                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item3->value;
                                if (var1->key->kind == NODE_KIND_ID)
                                {
                                    if (semantic_idcmp(var1->key, entity2->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                                else
                                {
                                    node_t *node4 = (node_t *)var1->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_ENTITY)
                                        {
                                            node_entity_t *entity4 = (node_entity_t *)item4->value;
                                            if (semantic_idcmp(entity4->key, entity2->key) == 1)
                                            {
                                                semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
                                                    entity2->key->position.line, entity2->key->position.column);
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
            break;
        }
        else
        if (current->kind == NODE_KIND_BODY)
        {
            node_block_t *block2 = (node_block_t *)current->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for2 = (node_for_t *)item2->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, for2->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            for2->key->position.line, for2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FORIN)
                {
                    node_forin_t *for2 = (node_forin_t *)item2->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, for2->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            for2->key->position.line, for2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, var2->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            var2->key->position.line, var2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            node_t *node3 = (node_t *)var2->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(var1->key, entity1->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            entity1->key->position.line, entity1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *enteiry1 = (node_entity_t *)item3->value;
                                    node_t *node4 = (node_t *)var2->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_ENTITY)
                                        {
                                            node_entity_t *enteiry2 = (node_entity_t *)item4->value;
                                            if (semantic_idcmp(enteiry1->key, enteiry2->key) == 1)
                                            {
                                                semantic_error(program, enteiry1->key, "already defined, previous in (%lld:%lld)",
                                                    enteiry2->key->position.line, enteiry2->key->position.column);
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
            sub = current;
            current = current->parent;
            continue;
        }
        else
        if (current->kind == NODE_KIND_FUN)
        {
            node_fun_t *fun2 = (node_fun_t *)current->value;

            if (fun2->generics != NULL)
            {
                node_t *node2 = fun2->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item2->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, generic3->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    generic3->key->position.line, generic3->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node3 = (node_t *)var1->key;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;
                                    if (semantic_idcmp(entity1->key, generic3->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            generic3->key->position.line, generic3->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (fun2->parameters != NULL)
            {
                node_t *node3 = fun2->parameters;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    
                    if (item3->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (semantic_idcmp(var1->key, parameter3->key) == 1)
                            {
                                semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                    parameter3->key->position.line, parameter3->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            node_t *node4 = (node_t *)var1->key;
                            node_block_t *block4 = (node_block_t *)node4->value;
                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                if (item4->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item4->value;
                                    if (semantic_idcmp(entity1->key, parameter3->key) == 1)
                                    {
                                        semantic_error(program, entity1->key, "already defined, previous in (%lld:%lld)",
                                            parameter3->key->position.line, parameter3->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            break;
        }
        else
        {
            sub = current;
            current = current->parent;
            continue;
        }
    }

	return 1;
}

static int32_t
semantic_statement(program_t *program, node_t *node, uint64_t flag)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result;
        result = semantic_if(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result;
        result = semantic_for(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result;
        result = semantic_forin(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result;
        result = semantic_try(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result;
        result = semantic_var(program, node, flag);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    {
        //return semantic_assign(program, node, flag);
        return 1;
    }
    return 0;
}

static int32_t
semantic_body(program_t *program, node_t *node, uint64_t flag)
{
    node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result = semantic_statement(program, item, flag);
        if (result == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_fun(program_t *program, node_t *node, uint64_t flag)
{
	node_fun_t *fun1 = (node_fun_t *)node->value;

    {
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(fun1->key, generic1->key) == 1)
                            {
                                semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic1->key->position.line, generic1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)item2->value;

                            if (semantic_idcmp(fun1->key, heritage1->key) == 1)
                            {
                                semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage1->key->position.line, heritage1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                node_t *node3 = class2->block;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a2;
                for (a2 = block3->list->begin;a2 != block3->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(fun1->key, class3->key) == 1)
                        {
                            node_t *ngs1 = class3->generics;
                            node_t *ngs2 = fun1->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item2->value;

                        if (semantic_idcmp(fun1->key, enum2->key) == 1)
                        {
                            semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(fun1->key, fun2->key) == 1)
                        {
                            node_t *ngs1 = fun1->generics;
                            node_t *ngs2 = fun2->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *nps1 = fun1->parameters;
                                node_t *nps2 = fun2->parameters;
                                int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                        fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(fun1->key, property2->key) == 1)
                        {
                            semantic_error(program, fun1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                current = current->parent;
                continue;
            }
        }
    }
    
    if (fun1->annotation != NULL)
    {
        node_t *node1 = fun1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = fun1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            if (semantic_idstrcmp(annotation1->key, "Main") == 1)
            {
                if (program->main == NULL)
                {
                    if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                    {
                        semantic_error(program, fun1->key, "Non-Static, 'Main' must be a static function\n\tInternal:%s-%u", 
                            __FILE__, __LINE__);
                        return -1;
                    }
                    program->main = node;
                }
                else
                {
                    node_t *node2 = program->main;
                    node_fun_t *fun2 = (node_fun_t *)node2->value;

                    semantic_error(program, fun1->key, "'Main' is already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                        fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            node1 = annotation1->next;
        }
    }

    if (fun1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, fun1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, fun1->parameters, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        int32_t r1 = semantic_body(program, fun1->body, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_enum(program_t *program, node_t *node, uint64_t flag)
{
	node_enum_t *enum1 = (node_enum_t *)node->value;
    
    {
        node_t *sub1 = node;
        node_t *current1 = node->parent;
        while (current1 != NULL)
        {
            if (current1->kind == NODE_KIND_MODULE)
            {
                node_module_t *module2 = (node_module_t *)current1->value;

                ilist_t *a2;
                for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub1->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_USING)
                    {
                        node_using_t *usage1 = (node_using_t *)item2->value;

                        if (usage1->packages != NULL)
                        {
                            node_t *node3 = usage1->packages;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_PACKAGE)
                                {
                                    node_package_t *package3 = (node_package_t *)item3->value;
                                    if (semantic_idcmp(enum1->key, package3->key) == 1)
                                    {
                                        semantic_error(program, enum1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                            package3->key->position.line, package3->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

                            node_t *node1 = program_load(program, basic2->value);
                            if (node1 == NULL)
                            {
                                return -1;
                            }
                            node_module_t *module1 = (node_module_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = module1->items->begin; a2 != module1->items->end; a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class1 = (node_class_t *)item2->value;
                                    if ((class1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(class1->key, enum1->key) == 1)
                                        {
                                            semantic_error(program, enum1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                class1->key->position.path, class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                    node_enum_t *enum2 = (node_enum_t *)item2->value;
                                    if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(enum2->key, enum1->key) == 1)
                                        {
                                            semantic_error(program, enum1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                enum2->key->position.path, enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(enum1->key, class3->key) == 1)
                        {
                            semantic_error(program, enum1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item2->value;

                        if (semantic_idcmp(enum1->key, enum2->key) == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            if (current1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current1->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(enum1->key, generic3->key) == 1)
                            {
                                semantic_error(program, enum1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic3->key->position.line, generic3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }

                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage3 = (node_heritage_t *)item3->value;
                            if (semantic_idcmp(enum1->key, heritage3->key) == 1)
                            {
                                semantic_error(program, enum1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage3->key->position.line, heritage3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }

                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(enum1->key, class3->key) == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item2->value;

                        if (semantic_idcmp(enum1->key, enum2->key) == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(enum1->key, fun2->key) == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(enum1->key, property2->key) == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
        }
    }

    if (enum1->annotation != NULL)
    {
        node_t *node1 = enum1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = enum1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            node1 = annotation1->next;
        }
    }

    node_t *node1 = enum1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        ilist_t *a2;
        for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;
            if (item1->id == item2->id)
            {
                break;
            }
            
            node_member_t *member1 = (node_member_t *)item1->value;
            node_member_t *member2 = (node_member_t *)item2->value;
            if (semantic_idcmp(member1->key, member2->key) == 1)
            {
                semantic_error(program, item1, "already defined, previous in (%lld:%lld)",
                    item2->position.line, item2->position.column);
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
semantic_property(program_t *program, node_t *node, uint64_t flag)
{
    node_property_t *property1 = (node_property_t *)node->value;
    
    {
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(property1->key, generic3->key) == 1)
                            {
                                semantic_error(program, property1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic3->key->position.line, generic3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }

                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage3 = (node_heritage_t *)item3->value;
                            if (semantic_idcmp(property1->key, heritage3->key) == 1)
                            {
                                semantic_error(program, property1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage3->key->position.line, heritage3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }

                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    
                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(property1->key, class3->key) == 1)
                        {
                            semantic_error(program, property1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(property1->key, fun2->key) == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item2->value;

                        if (semantic_idcmp(property1->key, enum2->key) == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(property1->key, property2->key) == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                current = current->parent;
                continue;
            }
        }
    }

    if (property1->annotation != NULL)
    {
        node_t *node1 = property1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = property1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            node1 = annotation1->next;
        }
    }

    return 1;
}

static int32_t
semantic_heritage(program_t *program, node_t *node, uint64_t flag)
{
    node_heritage_t *heritage1 = (node_heritage_t *)node->value;
    
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_HERITAGES)
            {
                node_block_t *block2 = (node_block_t *)current->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->id == sub->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                        if (semantic_idcmp(heritage1->key, heritage3->key) == 1)
                        {
                            semantic_error(program, heritage1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                heritage3->key->position.line, heritage3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                
                sub = current;
                current = current->parent;
                continue;
            }
            else
            if (current->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block3 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;

                        if (item3->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                            if (semantic_idcmp(heritage1->key, generic3->key) == 1)
                            {
                                semantic_error(program, heritage1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic3->key->position.line, generic3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                break;
            }
            else
            {
                sub = current;
                current = current->parent;
                continue;
            }
        }
    }

    return 1;
}

static int32_t
semantic_heritages(program_t *program, node_t *node, uint64_t flag)
{
	node_block_t *heritages = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_heritage(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_class(program_t *program, node_t *node, uint64_t flag)
{
	node_class_t *class1 = (node_class_t *)node->value;

    {
        node_t *sub1 = node;
        node_t *current1 = node->parent;
        while (current1 != NULL)
        {
            if (current1->kind == NODE_KIND_MODULE)
            {
                node_module_t *module2 = (node_module_t *)current1->value;

                ilist_t *a1;
                for (a1 = module2->items->begin;a1 != module2->items->end;a1 = a1->next)
                {
                    node_t *item1 = (node_t *)a1->value;
                    if (item1->id == sub1->id)
                    {
                        break;
                    }
                    
                    if (item1->kind == NODE_KIND_USING)
                    {
                        node_using_t *usage1 = (node_using_t *)item1->value;

                        if (usage1->packages != NULL)
                        {
                            node_t *node1 = usage1->packages;
                            node_block_t *block1 = (node_block_t *)node1->value;
                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_PACKAGE)
                                {
                                    node_package_t *package1 = (node_package_t *)item2->value;
                                    if (semantic_idcmp(package1->key, class1->key) == 1)
                                    {
                                        semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                            package1->key->position.line, package1->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

                            node_t *node1 = program_load(program, basic2->value);
                            if (node1 == NULL)
                            {
                                return -1;
                            }
                            node_module_t *module1 = (node_module_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = module1->items->begin; a2 != module1->items->end; a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class2 = (node_class_t *)item2->value;
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(class2->key, class1->key) == 1)
                                        {
                                            int32_t r1 = semantic_eqaul_gsgs(program, class2->generics, class1->generics);
                                            if (r1 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r1 == 1)
                                            {
                                                semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                    class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                    node_enum_t *enum1 = (node_enum_t *)item2->value;
                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(enum1->key, class1->key) == 1)
                                        {
                                            semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                enum1->key->position.path, enum1->key->position.line, enum1->key->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item1->value;
                        if (semantic_idcmp(class1->key, class3->key) == 1)
                        {
                            node_t *ngs1 = class1->generics;
                            node_t *ngs2 = class3->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item1->value;

                        if (semantic_idcmp(class1->key, enum2->key) == 1)
                        {
                            semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            if (current1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)current1->value;

                if (class2->generics != NULL)
                {
                    node_t *node2 = class2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(class1->key, generic1->key) == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    generic1->key->position.line, generic1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                if (class2->heritages != NULL)
                {
                    node_t *node2 = class2->heritages;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_HERITAGE)
                        {
                            node_heritage_t *heritage1 = (node_heritage_t *)item2->value;

                            if (semantic_idcmp(class1->key, heritage1->key) == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    heritage1->key->position.line, heritage1->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
                
                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->id == node->id)
                    {
                        break;
                    }

                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item2->value;
                        if (semantic_idcmp(class1->key, class3->key) == 1)
                        {
                            node_t *ngs1 = class1->generics;
                            node_t *ngs2 = class3->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun2 = (node_fun_t *)item2->value;

                        if (semantic_idcmp(class1->key, fun2->key) == 1)
                        {
                            node_t *ngs1 = class1->generics;
                            node_t *ngs2 = fun2->generics;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                    fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item2->value;

                        if (semantic_idcmp(class1->key, enum2->key) == 1)
                        {
                            semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_PROPERTY)
                    {
                        node_property_t *property2 = (node_property_t *)item2->value;

                        if (semantic_idcmp(class1->key, property2->key) == 1)
                        {
                            semantic_error(program, class1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
        }
    }

    if (class1->annotation != NULL)
    {
        node_t *node1 = class1->annotation;
        while (node1 != NULL)
        {
            node_note_t *annotation1 = (node_note_t *)node1->value;
            node_t *node2 = class1->annotation;
            while (node2 != NULL)
            {
                if (node1->id == node2->id)
                {
                    break;
                }
                node_note_t *annotation2 = (node_note_t *)node2->value;
                if (semantic_idcmp(annotation1->key, annotation2->key) == 1)
                {
                    semantic_error(program, annotation1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u", 
                        annotation2->key->position.line, annotation2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
                node2 = annotation2->next;
            }

            node1 = annotation1->next;
        }
    }

    if (class1->generics != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            semantic_error(program, class1->key, "Generic type in static class\n\tInternal:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_generics(program, class1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            semantic_error(program, class1->key, "Generic type in inheritance\n\tInternal:%s-%u", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_heritages(program, class1->heritages, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    node_t *node1 = class1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        
        if (item1->kind == NODE_KIND_CLASS)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, class2->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t result = semantic_class(program, item1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_ENUM)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, enum1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }
            
            int32_t result = semantic_enum(program, item1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_FUN)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, fun1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t result = semantic_fun(program, item1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_PROPERTY)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    semantic_error(program, property1->key, "Static class(%lld:%lld), members must be static\n\tInternal:%s-%u", 
                        class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }
            }

            int32_t result = semantic_property(program, item1, flag);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
semantic_package(program_t *program, node_t *node, uint64_t flag)
{
    node_package_t *package1 = (node_package_t *)node->value;

    if (package1->address != NULL)
    {
        node_t *address1 = package1->address;
        while(address1 != NULL)
        {
            if (address1->kind == NODE_KIND_PSEUDONYM)
            {
                node_carrier_t *carrier1 = (node_carrier_t *)address1->value;
                address1 = carrier1->base;
            }
            else
            if (address1->kind == NODE_KIND_ATTRIBUTE)
            {
                node_binary_t *binary = (node_binary_t *)address1->value;
                address1 = binary->left;
            }
            else
            if (address1->kind == NODE_KIND_ID)
            {
                break;
            }
            else
            {
                semantic_error(program, package1->address, "Not a address\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }  
    }   

    {
        node_t *sub1 = node;
        node_t *current1 = node->parent;
        while (current1 != NULL)
        {
            if (current1->kind == NODE_KIND_PACKAGES)
            {
                node_block_t *block3 = (node_block_t *)current1->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->kind == NODE_KIND_PACKAGE)
                    {
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        node_package_t *package3 = (node_package_t *)item3->value;

                        if (semantic_idcmp(package1->key, package3->key) == 1)
                        {
                            semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                package3->key->position.line, package3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }

                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
            else
            if (current1->kind == NODE_KIND_MODULE)
            {
                node_module_t *module1 = (node_module_t *)current1->value;

                ilist_t *a1;
                for (a1 = module1->items->begin;a1 != module1->items->end;a1 = a1->next)
                {
                    node_t *item1 = (node_t *)a1->value;
                    
                    if (item1->id == sub1->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_USING)
                    {
                        node_using_t *usage1 = (node_using_t *)item1->value;

                        if (usage1->packages != NULL)
                        {
                            node_t *node3 = usage1->packages;
                            node_block_t *block3 = (node_block_t *)node3->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_PACKAGE)
                                {
                                    node_package_t *package3 = (node_package_t *)item3->value;
                                    if (semantic_idcmp(package1->key, package3->key) == 1)
                                    {
                                        semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                            package3->key->position.line, package3->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

                            node_t *node1 = program_load(program, basic2->value);
                            if (node1 == NULL)
                            {
                                return -1;
                            }
                            node_module_t *module1 = (node_module_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = module1->items->begin; a2 != module1->items->end; a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class1 = (node_class_t *)item2->value;
                                    if ((class1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(class1->key, package1->key) == 1)
                                        {
                                            int32_t r1 = semantic_eqaul_gsgs(program, class1->generics, package1->generics);
                                            if (r1 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r1 == 1)
                                            {
                                                semantic_error(program, package1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                    class1->key->position.path, class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                    node_enum_t *enum1 = (node_enum_t *)item2->value;
                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                                    {
                                        if (semantic_idcmp(enum1->key, package1->key) == 1)
                                        {
                                            semantic_error(program, package1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                enum1->key->position.path, enum1->key->position.line, enum1->key->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class3 = (node_class_t *)item1->value;

                        if (semantic_idcmp(package1->key, class3->key) == 1)
                        {
                            semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum2 = (node_enum_t *)item1->value;

                        if (semantic_idcmp(package1->key, enum2->key) == 1)
                        {
                            semantic_error(program, package1->key, "Already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            {
                sub1 = current1;
                current1 = current1->parent;
                continue;
            }
        }
    }

    if (package1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, package1->generics, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_packages(program_t *program, node_t *node, uint64_t flag)
{
    node_block_t *packages = (node_block_t *)node->value;
    
	ilist_t *a1;
    for (a1 = packages->list->begin;a1 != packages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_package(program, item1, flag);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_using(program_t *program, node_t *node, uint64_t flag)
{
	node_using_t *usage1 = (node_using_t *)node->value;

	if (usage1->packages != NULL)
    {
        int32_t r1 = semantic_packages(program, usage1->packages, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }
    else
    {
        node_basic_t *basic2 = (node_basic_t *)usage1->path->value;

        node_t *node1 = program_load(program, basic2->value);
        if (node1 == NULL)
        {
            return -1;
        }

        node_module_t *module1 = (node_module_t *)node1->value;

        ilist_t *a1;
        for (a1 = module1->items->begin; a1 != module1->items->end; a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                if ((class1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                {
                    node_t *sub1 = node;
                    node_t *current1 = node->parent;
                    while (current1 != NULL)
                    {
                        if (current1->kind == NODE_KIND_MODULE)
                        {
                            node_module_t *module2 = (node_module_t *)current1->value;

                            ilist_t *a2;
                            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                
                                if (item2->id == sub1->id)
                                {
                                    break;
                                }

                                if (item2->kind == NODE_KIND_USING)
                                {
                                    node_using_t *usage2 = (node_using_t *)item2->value;
                                    node_t *node2 = usage2->packages;
                                    node_block_t *block1 = (node_block_t *)node2->value;

                                    ilist_t *a3;
                                    for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_PACKAGE)
                                        {
                                            node_package_t *package1 = (node_package_t *)item3->value;
                                            if (semantic_idcmp(class1->key, package1->key) == 1)
                                            {
                                                int32_t r1 = semantic_eqaul_gsgs(program, class1->generics, package1->generics);
                                                if (r1 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r1 == 1)
                                                {
                                                    semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        package1->key->position.path, package1->key->position.line, package1->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class2 = (node_class_t *)item2->value;

                                    if (semantic_idcmp(class1->key, class2->key) == 1)
                                    {
                                        int32_t r1 = semantic_eqaul_gsgs(program, class1->generics, class2->generics);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                    node_enum_t *enum1 = (node_enum_t *)item2->value;

                                    if (semantic_idcmp(class1->key, enum1->key) == 1)
                                    {
                                        semantic_error(program, class1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                            enum1->key->position.path, enum1->key->position.line, enum1->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                            break;
                        }
                        else
                        {
                            sub1 = current1;
                            current1 = current1->parent;
                            continue;
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) == SYNTAX_MODIFIER_EXPORT)
                {
                    node_t *sub1 = node;
                    node_t *current1 = node->parent;
                    while (current1 != NULL)
                    {
                        if (current1->kind == NODE_KIND_MODULE)
                        {
                            node_module_t *module2 = (node_module_t *)current1->value;

                            ilist_t *a2;
                            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                
                                if (item2->id == sub1->id)
                                {
                                    break;
                                }

                                if (item2->kind == NODE_KIND_USING)
                                {
                                    node_using_t *usage2 = (node_using_t *)item2->value;
                                    node_t *node2 = usage2->packages;
                                    node_block_t *block1 = (node_block_t *)node2->value;

                                    ilist_t *a3;
                                    for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_PACKAGE)
                                        {
                                            node_package_t *package1 = (node_package_t *)item3->value;
                                            if (semantic_idcmp(enum1->key, package1->key) == 1)
                                            {
                                                semantic_error(program, enum1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                    package1->key->position.path, package1->key->position.line, package1->key->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_class_t *class2 = (node_class_t *)item2->value;

                                    if (semantic_idcmp(enum1->key, class2->key) == 1)
                                    {
                                        semantic_error(program, enum1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                            class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                                    if (semantic_idcmp(enum1->key, enum2->key) == 1)
                                    {
                                        semantic_error(program, enum1->key, "Already defined, previous in (%s-%lld:%lld)\n\tInternal:%s-%u",
                                            enum2->key->position.path, enum2->key->position.line, enum2->key->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                            break;
                        }
                        else
                        {
                            sub1 = current1;
                            current1 = current1->parent;
                            continue;
                        }
                    }
                }
            }
        }
    }

	return 1;
}

int32_t
semantic_module(program_t *program, node_t *node)
{
	node_module_t *module = (node_module_t *)node->value;

    ilist_t *a1;
    for (a1 = module->items->begin; a1 != module->items->end; a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        if (item1->kind == NODE_KIND_USING)
        {
            int32_t result = semantic_using(program, item1, SEMANTIC_FLAG_NONE);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_t *clone1 = node_clone(item1->parent, item1);
            if (clone1 == NULL)
            {
                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            clone1->flag |= NODE_FLAG_NEW;
            int32_t result = semantic_class(program, clone1, SEMANTIC_FLAG_NONE);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item1->kind == NODE_KIND_ENUM)
        {
            int32_t result = semantic_enum(program, item1, SEMANTIC_FLAG_NONE);
            if (result == -1)
            {
                return -1;
            }
        }
    }
    
	return 1;
}

int32_t
semantic_run(program_t *program, node_t *node)
{
    node_module_t *module1 = (node_module_t *)node->value;
    
    FILE *fout = fopen(program->out_file, "wb");
	if (fout == NULL)
	{
        fprintf(stderr, "could not create(%s)\n", program->out_file);
        return -1;
    }
    program->out = fout;

    int32_t found1 = 0, found2 = 0, found3 = 0;
    ilist_t *a1;
    for (a1 = module1->items->begin; a1 != module1->items->end; a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)item1->value;
            if (semantic_idstrcmp(class1->key, "System") == 1)
            {
                found1 = 1;

                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;
                
                ilist_t *a2;
                for (a2 = block1->list->begin; a2 != block1->list->end; a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun1 = (node_fun_t *)item2->value;
                        if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                        {
                            found2 = 1;
                            fputs("global	_start\n\n", program->out);
                            fputs("section .text\n\n", program->out);
                            fputs("_start:\n", program->out);
                            /* int32_t r1 = semantic_fun(program, item2);
                            if (r1 == -1)
                            {
                                return -1;
                            } */
                        }
                    }
                }

                if (found2 == 0)
                {
                    semantic_error(program, item1, "'Constructor' not found");
                    fclose(program->out);
                    return -1;
                }

                if (found3 == 0)
                {
                    semantic_error(program, item1, "'Constructor' missmaching");
                    fclose(program->out);
                    return -1;
                }
            }
        }
    }

    if (found1 == 0)
    {
        semantic_error(program, node, "'System' class not found");
        fclose(program->out);
        return -1;
    }

    fclose(program->out);
	return 1;
}